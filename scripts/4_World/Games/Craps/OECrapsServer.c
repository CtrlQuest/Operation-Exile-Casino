class OECrapsServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OECrapsSession> m_Sessions;

    void OECrapsServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OECrapsSession>;
    }

    void ApplyConfig(OECasinoConfig config)
    {
        if (!config) return;
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
    }

    void HandleSync(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_CRAPS || !m_Config.craps || m_Config.craps.enabled != 1)
        {
            SendError(player, stationId, "This Craps table is not available.");
            return;
        }

        OECrapsSession session = GetTableSession(sender);
        if (session)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, GetActiveMessage(session));
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleStart(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param3<string, int, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        int bet = req.param2;
        int lineBetType = req.param3;

        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_CRAPS || !m_Config.craps || m_Config.craps.enabled != 1)
        {
            SendError(player, stationId, "This Craps table is not available.");
            return;
        }

        if (bet < m_Config.craps.minBet || bet > m_Config.craps.maxBet)
        {
            SendRejected(player, stationId, "Line wager is outside the allowed range.");
            return;
        }

        if (!IsValidLineBet(lineBetType))
        {
            SendRejected(player, stationId, "Choose PASS LINE or DON'T PASS before rolling.");
            return;
        }

        OECrapsSession session = GetTableSession(sender);
        if (session && session.lineBetActive)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, "The current line bet is still active.");
            return;
        }

        if (session && session.stationId != stationId)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, "Finish or clear the Craps table at the original station.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            if (session) SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Not enough casino currency for the new line bet.");
            else SendRejected(player, stationId, "Not enough casino currency.");
            return;
        }

        if (!session)
        {
            session = new OECrapsSession(sender.GetId(), stationId, bet, lineBetType);
            m_Sessions.Set(sender.GetId(), session);
        }
        else
        {
            session.BeginLine(bet, lineBetType);
        }

        RollSession(player, session);
    }

    void HandleRoll(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OECrapsSession session = GetTableSession(sender);
        if (!session || !session.lineBetActive)
        {
            if (session) SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Choose a new PASS LINE or DON'T PASS wager before the next Come-Out roll.");
            else SendError(player, req.param1, "No active Craps line bet. Start a new round first.");
            return;
        }

        if (session.stationId != req.param1)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "This Craps table session belongs to another station.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_CRAPS)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "You moved too far away from the Craps table.");
            return;
        }

        RollSession(player, session);
    }

    void HandleBetAction(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<ref OECrapsBetActionRequest> reqParam;
        if (!ctx.Read(reqParam)) return;

        OECrapsBetActionRequest req = reqParam.param1;
        if (!req) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OECrapsSession session = GetTableSession(sender);
        if (!session)
        {
            SendRejected(player, req.stationId, "Start a Craps line bet before adding table bets.");
            return;
        }

        if (session.stationId != req.stationId)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "This Craps session belongs to another station.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_CRAPS)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "You moved too far away from the Craps table.");
            return;
        }

        if (req.action == OE_CRAPS_ACTION_UNDO)
        {
            UndoLastBet(player, session);
            return;
        }

        if (req.action == OE_CRAPS_ACTION_CLEAR)
        {
            ClearRemovableSideBets(player, session);
            return;
        }

        if (req.action != OE_CRAPS_ACTION_ADD)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Unknown Craps table action.");
            return;
        }

        AddSideBet(player, session, req.betType, req.target, req.amount);
    }

    protected void AddSideBet(PlayerBase player, OECrapsSession session, int betType, int target, int amount)
    {
        if (!session.lineBetActive || session.phase != OE_CRAPS_PHASE_POINT)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Side bets open after the Come-Out roll establishes a POINT.");
            return;
        }

        if (amount < m_Config.craps.sideMinBet || amount > m_Config.craps.maxSideBet)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Selected chip is outside the side-bet limits.");
            return;
        }

        if (!IsValidSideBet(betType, target))
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "That Craps side bet is not valid.");
            return;
        }

        if (session.GetSideBetTotal() + amount > m_Config.craps.maxSideBetTotal)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Total side bets exceed the table limit of " + m_Config.craps.maxSideBetTotal.ToString() + ".");
            return;
        }

        if (betType == OE_CRAPS_SIDE_PLACE && (target == 6 || target == 8))
        {
            if ((amount % 6) != 0)
            {
                SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "PLACE 6 / 8 needs a wager divisible by 6 for an exact 7:6 payout.  With these controls, try 30, 60, 90, 120, etc.");
                return;
            }
        }

        if (betType == OE_CRAPS_SIDE_ODDS)
        {
            int maxOdds = Math.Floor(session.originalBet * m_Config.craps.maxOddsMultiplier);
            if (session.oddsBet + amount > maxOdds)
            {
                SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Odds are capped at " + maxOdds.ToString() + " chips for this line wager.");
                return;
            }

            if (!IsExactOddsAmount(session, amount))
            {
                SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, GetOddsAmountError(session));
                return;
            }
        }

        if (!m_Currency.TryDebit(player, amount))
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_REJECTED, "Not enough casino currency for that side bet.");
            return;
        }

        if (betType == OE_CRAPS_SIDE_ODDS) session.oddsBet += amount;
        else if (betType == OE_CRAPS_SIDE_COME) session.comePending += amount;
        else if (betType == OE_CRAPS_SIDE_DONT_COME) session.dontComePending += amount;
        else if (betType == OE_CRAPS_SIDE_FIELD) session.fieldBet += amount;
        else if (betType == OE_CRAPS_SIDE_PLACE) session.AddPlaceBet(target, amount);

        session.lastActionType = betType;
        session.lastActionTarget = target;
        session.lastActionAmount = amount;

        string label = GetSideBetLabel(betType, target);
        SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, "Placed " + amount.ToString() + " on " + label + ".");
    }

    protected void UndoLastBet(PlayerBase player, OECrapsSession session)
    {
        int amount = session.lastActionAmount;
        if (amount <= 0)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Nothing new to undo before this roll.");
            return;
        }

        if (!RemoveSideAmount(session, session.lastActionType, session.lastActionTarget, amount))
        {
            session.ResetLastAction();
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "That bet can no longer be undone.");
            return;
        }

        if (!m_Currency.Credit(player, amount))
        {
            // Restore the table position if refund creation fails.
            RestoreSideAmount(session, session.lastActionType, session.lastActionTarget, amount);
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Undo refund could not be created. Contact an admin.");
            return;
        }

        session.ResetLastAction();
        if (!session.lineBetActive && !session.HasSideBets()) session.finished = true;
        SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, "Last side-bet chip returned.");
    }

    protected void ClearRemovableSideBets(PlayerBase player, OECrapsSession session)
    {
        int refund = session.oddsBet + session.comePending + session.dontComePending + session.fieldBet + session.GetPlaceTotal();
        if (refund <= 0)
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "No removable side bets are on the table. Established COME / DON'T COME points stay working.");
            return;
        }

        if (!m_Currency.Credit(player, refund))
        {
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Side-bet refund could not be created. Contact an admin.");
            return;
        }

        session.oddsBet = 0;
        session.comePending = 0;
        session.dontComePending = 0;
        session.fieldBet = 0;
        ClearPlaceBets(session);
        session.ResetLastAction();
        if (!session.lineBetActive && !session.HasSideBets()) session.finished = true;

        SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, "Cleared removable side bets and returned " + refund.ToString() + " chips.");
    }

    protected void RollSession(PlayerBase player, OECrapsSession session)
    {
        if (!player || !session || session.finished || !session.lineBetActive) return;

        bool placeBetsWorking = false;
        if (session.phase == OE_CRAPS_PHASE_POINT) placeBetsWorking = true;

        session.lastDie1 = Math.RandomInt(1, 7);
        session.lastDie2 = Math.RandomInt(1, 7);
        session.lastTotal = session.lastDie1 + session.lastDie2;
        session.rollNumber++;
        session.ResetLastAction();
        session.ResetLastSideResult();

        int sideCredit = ResolveSideBets(session, session.lastTotal, placeBetsWorking);

        if (session.phase == OE_CRAPS_PHASE_POINT)
            sideCredit += ResolveLineOdds(session, session.lastTotal);

        if (sideCredit > 0)
        {
            if (!m_Currency.Credit(player, sideCredit))
            {
                session.finished = true;
                session.lineBetActive = false;
                SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "A Craps side-bet payout could not be created. Contact an admin.");
                return;
            }
        }

        if (session.phase == OE_CRAPS_PHASE_COME_OUT)
        {
            ResolveComeOut(player, session);
            return;
        }

        ResolvePointRoll(player, session);
    }

    protected int ResolveSideBets(OECrapsSession session, int total, bool placeBetsWorking)
    {
        int credit = 0;
        int amount;

        // Existing COME points resolve before a newly placed COME travels.
        if (total == 7)
        {
            amount = session.GetComePointTotal();
            if (amount > 0)
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "COME points lose " + amount.ToString());
                ClearComePoints(session);
            }
        }
        else
        {
            amount = session.GetComePoint(total);
            if (amount > 0)
            {
                credit += amount * 2;
                session.lastSideReturn += amount * 2;
                session.lastSideNet += amount;
                AddSideSummary(session, "COME " + total.ToString() + " +" + amount.ToString());
                session.ClearComePoint(total);
            }
        }

        // Existing DON'T COME points work in reverse.
        if (total == 7)
        {
            amount = session.GetDontComePointTotal();
            if (amount > 0)
            {
                credit += amount * 2;
                session.lastSideReturn += amount * 2;
                session.lastSideNet += amount;
                AddSideSummary(session, "DON'T COME +" + amount.ToString());
                ClearDontComePoints(session);
            }
        }
        else
        {
            amount = session.GetDontComePoint(total);
            if (amount > 0)
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "DON'T COME " + total.ToString() + " loses");
                session.ClearDontComePoint(total);
            }
        }

        // Place bets are OFF on a Come-Out roll and working during Point play.
        if (placeBetsWorking)
        {
            if (total == 7)
            {
                amount = session.GetPlaceTotal();
                if (amount > 0)
                {
                    session.lastSideNet -= amount;
                    AddSideSummary(session, "PLACE bets lose " + amount.ToString());
                    ClearPlaceBets(session);
                }
            }
            else if (IsPointNumber(total))
            {
                amount = session.GetPlaceBet(total);
                if (amount > 0)
                {
                    int placeProfit = GetPlaceProfit(total, amount);
                    credit += placeProfit;
                    session.lastSideReturn += placeProfit;
                    session.lastSideNet += placeProfit;
                    AddSideSummary(session, "PLACE " + total.ToString() + " +" + placeProfit.ToString());
                    // Place stake stays up after a win.
                }
            }
        }

        // FIELD is a one-roll bet.
        amount = session.fieldBet;
        if (amount > 0)
        {
            session.fieldBet = 0;
            if (total == 2)
            {
                credit += amount * 3;
                session.lastSideReturn += amount * 3;
                session.lastSideNet += amount * 2;
                AddSideSummary(session, "FIELD 2 +" + (amount * 2).ToString());
            }
            else if (total == 12)
            {
                credit += amount * 4;
                session.lastSideReturn += amount * 4;
                session.lastSideNet += amount * 3;
                AddSideSummary(session, "FIELD 12 +" + (amount * 3).ToString());
            }
            else if (total == 3 || total == 4 || total == 9 || total == 10 || total == 11)
            {
                credit += amount * 2;
                session.lastSideReturn += amount * 2;
                session.lastSideNet += amount;
                AddSideSummary(session, "FIELD +" + amount.ToString());
            }
            else
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "FIELD loses " + amount.ToString());
            }
        }

        // A newly placed COME bet uses this roll as its own Come-Out.
        amount = session.comePending;
        if (amount > 0)
        {
            session.comePending = 0;
            if (total == 7 || total == 11)
            {
                credit += amount * 2;
                session.lastSideReturn += amount * 2;
                session.lastSideNet += amount;
                AddSideSummary(session, "COME +" + amount.ToString());
            }
            else if (total == 2 || total == 3 || total == 12)
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "COME loses " + amount.ToString());
            }
            else
            {
                session.AddComePoint(total, amount);
                AddSideSummary(session, "COME -> " + total.ToString());
            }
        }

        // DON'T COME mirrors the Don't Pass Come-Out rules.
        amount = session.dontComePending;
        if (amount > 0)
        {
            session.dontComePending = 0;
            if (total == 2 || total == 3)
            {
                credit += amount * 2;
                session.lastSideReturn += amount * 2;
                session.lastSideNet += amount;
                AddSideSummary(session, "DON'T COME +" + amount.ToString());
            }
            else if (total == 12)
            {
                credit += amount;
                session.lastSideReturn += amount;
                AddSideSummary(session, "DON'T COME push");
            }
            else if (total == 7 || total == 11)
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "DON'T COME loses " + amount.ToString());
            }
            else
            {
                session.AddDontComePoint(total, amount);
                AddSideSummary(session, "DON'T COME -> " + total.ToString());
            }
        }

        return credit;
    }

    protected int ResolveLineOdds(OECrapsSession session, int total)
    {
        int amount = session.oddsBet;
        if (amount <= 0) return 0;

        if (total != session.point && total != 7) return 0;

        session.oddsBet = 0;

        if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
        {
            if (total == 7)
            {
                session.lastSideNet -= amount;
                AddSideSummary(session, "ODDS lose " + amount.ToString());
                return 0;
            }

            int passProfit = GetPassOddsProfit(session.point, amount);
            session.lastSideReturn += amount + passProfit;
            session.lastSideNet += passProfit;
            AddSideSummary(session, "ODDS +" + passProfit.ToString());
            return amount + passProfit;
        }

        // DON'T PASS lays the odds: win when 7 arrives before the point.
        if (total == session.point)
        {
            session.lastSideNet -= amount;
            AddSideSummary(session, "LAY ODDS lose " + amount.ToString());
            return 0;
        }

        int dontProfit = GetDontPassOddsProfit(session.point, amount);
        session.lastSideReturn += amount + dontProfit;
        session.lastSideNet += dontProfit;
        AddSideSummary(session, "LAY ODDS +" + dontProfit.ToString());
        return amount + dontProfit;
    }

    protected void ResolveComeOut(PlayerBase player, OECrapsSession session)
    {
        int total = session.lastTotal;

        if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
        {
            if (total == 7 || total == 11)
            {
                FinishLineWin(player, session, "Natural " + total.ToString() + "! PASS LINE wins.", m_Config.craps.passLineReturnMultiplier, "pass_natural");
                return;
            }

            if (total == 2 || total == 3 || total == 12)
            {
                FinishLineLoss(player, session, "Come-Out " + total.ToString() + " is craps. PASS LINE loses.", "pass_craps");
                return;
            }

            EstablishPoint(player, session, total);
            return;
        }

        if (total == 7 || total == 11)
        {
            FinishLineLoss(player, session, "Natural " + total.ToString() + ". DON'T PASS loses.", "dont_natural_loss");
            return;
        }

        if (total == 2 || total == 3)
        {
            FinishLineWin(player, session, "Come-Out " + total.ToString() + ". DON'T PASS wins.", m_Config.craps.dontPassReturnMultiplier, "dont_craps_win");
            return;
        }

        if (total == 12)
        {
            FinishLinePush(player, session, "Come-Out 12. DON'T PASS pushes; your wager is returned.", "dont_12_push");
            return;
        }

        EstablishPoint(player, session, total);
    }

    protected void EstablishPoint(PlayerBase player, OECrapsSession session, int point)
    {
        session.point = point;
        session.phase = OE_CRAPS_PHASE_POINT;
        session.payout = 0;
        session.netResult = 0 - session.originalBet;

        string lineName = GetLineBetName(session.lineBetType);
        string message = "POINT " + point.ToString() + " established. " + lineName + " locked." + "  Table bets and Odds are now open.";
        SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, message);
    }

    protected void ResolvePointRoll(PlayerBase player, OECrapsSession session)
    {
        int total = session.lastTotal;
        int point = session.point;

        if (total == point)
        {
            if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
            {
                FinishLineWin(player, session, "POINT " + point.ToString() + " made before 7. PASS LINE wins.", m_Config.craps.passLineReturnMultiplier, "pass_point_win");
                return;
            }

            FinishLineLoss(player, session, "POINT " + point.ToString() + " made before 7. DON'T PASS loses.", "dont_point_loss");
            return;
        }

        if (total == 7)
        {
            if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
            {
                FinishLineLoss(player, session, "Seven-out before POINT " + point.ToString() + ". PASS LINE loses.", "pass_seven_out");
                return;
            }

            FinishLineWin(player, session, "Seven-out before POINT " + point.ToString() + ". DON'T PASS wins.", m_Config.craps.dontPassReturnMultiplier, "dont_seven_win");
            return;
        }

        string message = "Rolled " + total.ToString() + ". POINT remains " + point.ToString() + ".";
        SendSessionState(player, session, OE_CRAPS_STATUS_ACTIVE, message);
    }

    protected void FinishLineWin(PlayerBase player, OECrapsSession session, string text, float returnMultiplier, string outcome)
    {
        int lineStake = session.originalBet;
        int payout = Math.Floor(lineStake * returnMultiplier);
        if (payout < lineStake) payout = lineStake;

        if (!m_Currency.Credit(player, payout))
        {
            m_Currency.Credit(player, lineStake);
            session.finished = true;
            session.lineBetActive = false;
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Craps line payout could not be created. The line wager was restored where possible; contact an admin.");
            return;
        }

        session.payout = payout;
        session.netResult = payout - lineStake;
        SettleLineFinished(session);

        string message = text + " Paid " + payout.ToString() + " chips.";
        if (!session.finished) message += "  Side bets remain; choose a new line.";

        SendSessionState(player, session, OE_CRAPS_STATUS_WIN, message);
        LogResult(player, session, outcome, lineStake);
    }

    protected void FinishLineLoss(PlayerBase player, OECrapsSession session, string text, string outcome)
    {
        int lineStake = session.originalBet;
        session.payout = 0;
        session.netResult = 0 - lineStake;
        SettleLineFinished(session);

        string message = text;
        if (!session.finished) message += "  Side bets remain; choose a new line.";

        SendSessionState(player, session, OE_CRAPS_STATUS_LOSS, message);
        LogResult(player, session, outcome, lineStake);
    }

    protected void FinishLinePush(PlayerBase player, OECrapsSession session, string text, string outcome)
    {
        int lineStake = session.originalBet;
        if (!m_Currency.Credit(player, lineStake))
        {
            session.finished = true;
            session.lineBetActive = false;
            SendSessionState(player, session, OE_CRAPS_STATUS_ERROR, "Craps push could not return the line wager. Contact an admin.");
            return;
        }

        session.payout = lineStake;
        session.netResult = 0;
        SettleLineFinished(session);

        string message = text;
        if (!session.finished) message += "  Side bets remain; choose a new line.";

        SendSessionState(player, session, OE_CRAPS_STATUS_PUSH, message);
        LogResult(player, session, outcome, lineStake);
    }

    protected void SettleLineFinished(OECrapsSession session)
    {
        session.lineBetActive = false;
        session.phase = OE_CRAPS_PHASE_COME_OUT;
        session.point = 0;
        session.oddsBet = 0;
        session.ResetLastAction();

        if (session.HasSideBets()) session.finished = false;
        else session.finished = true;
    }

    protected bool IsValidLineBet(int lineBetType)
    {
        if (lineBetType == OE_CRAPS_BET_PASS_LINE) return true;
        if (lineBetType == OE_CRAPS_BET_DONT_PASS_LINE) return true;
        return false;
    }

    protected bool IsValidSideBet(int betType, int target)
    {
        if (betType == OE_CRAPS_SIDE_ODDS) return true;
        if (betType == OE_CRAPS_SIDE_COME) return true;
        if (betType == OE_CRAPS_SIDE_DONT_COME) return true;
        if (betType == OE_CRAPS_SIDE_FIELD) return true;
        if (betType == OE_CRAPS_SIDE_PLACE) return IsPointNumber(target);
        return false;
    }

    protected bool IsPointNumber(int number)
    {
        if (number == 4 || number == 5 || number == 6 || number == 8 || number == 9 || number == 10) return true;
        return false;
    }

    protected int GetPlaceProfit(int number, int amount)
    {
        if (number == 4 || number == 10) return (amount * 9) / 5;
        if (number == 5 || number == 9) return (amount * 7) / 5;
        if (number == 6 || number == 8) return (amount * 7) / 6;
        return 0;
    }

    protected bool IsExactOddsAmount(OECrapsSession session, int amount)
    {
        if (!session || amount <= 0) return false;

        int point = session.point;
        if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
        {
            if (point == 4 || point == 10) return true;
            if (point == 5 || point == 9) return (amount % 2) == 0;
            if (point == 6 || point == 8) return (amount % 5) == 0;
            return false;
        }

        if (session.lineBetType == OE_CRAPS_BET_DONT_PASS_LINE)
        {
            if (point == 4 || point == 10) return (amount % 2) == 0;
            if (point == 5 || point == 9) return (amount % 3) == 0;
            if (point == 6 || point == 8) return (amount % 6) == 0;
            return false;
        }

        return false;
    }

    protected string GetOddsAmountError(OECrapsSession session)
    {
        if (!session) return "That Odds wager cannot be paid exactly.";

        int point = session.point;
        if (session.lineBetType == OE_CRAPS_BET_PASS_LINE)
        {
            if (point == 5 || point == 9)
                return "PASS ODDS on 5 / 9 needs an even wager for an exact 3:2 payout.";
            if (point == 6 || point == 8)
                return "PASS ODDS on 6 / 8 needs a wager divisible by 5 for an exact 6:5 payout.";
        }
        else if (session.lineBetType == OE_CRAPS_BET_DONT_PASS_LINE)
        {
            if (point == 4 || point == 10)
                return "DON'T PASS ODDS on 4 / 10 needs an even wager for an exact 1:2 payout.";
            if (point == 5 || point == 9)
                return "DON'T PASS ODDS on 5 / 9 needs a wager divisible by 3 for an exact 2:3 payout.  Try 30, 60, 90, 120, etc.";
            if (point == 6 || point == 8)
                return "DON'T PASS ODDS on 6 / 8 needs a wager divisible by 6 for an exact 5:6 payout.  Try 30, 60, 90, 120, etc.";
        }

        return "That Odds wager cannot be paid exactly for the current POINT.";
    }

    protected int GetPassOddsProfit(int point, int amount)
    {
        if (point == 4 || point == 10) return amount * 2;
        if (point == 5 || point == 9) return (amount * 3) / 2;
        if (point == 6 || point == 8) return (amount * 6) / 5;
        return 0;
    }

    protected int GetDontPassOddsProfit(int point, int amount)
    {
        if (point == 4 || point == 10) return amount / 2;
        if (point == 5 || point == 9) return (amount * 2) / 3;
        if (point == 6 || point == 8) return (amount * 5) / 6;
        return 0;
    }

    protected bool RemoveSideAmount(OECrapsSession session, int betType, int target, int amount)
    {
        if (betType == OE_CRAPS_SIDE_ODDS && session.oddsBet >= amount) { session.oddsBet -= amount; return true; }
        if (betType == OE_CRAPS_SIDE_COME && session.comePending >= amount) { session.comePending -= amount; return true; }
        if (betType == OE_CRAPS_SIDE_DONT_COME && session.dontComePending >= amount) { session.dontComePending -= amount; return true; }
        if (betType == OE_CRAPS_SIDE_FIELD && session.fieldBet >= amount) { session.fieldBet -= amount; return true; }
        if (betType == OE_CRAPS_SIDE_PLACE && session.GetPlaceBet(target) >= amount) { session.RemovePlaceBet(target, amount); return true; }
        return false;
    }

    protected void RestoreSideAmount(OECrapsSession session, int betType, int target, int amount)
    {
        if (betType == OE_CRAPS_SIDE_ODDS) session.oddsBet += amount;
        else if (betType == OE_CRAPS_SIDE_COME) session.comePending += amount;
        else if (betType == OE_CRAPS_SIDE_DONT_COME) session.dontComePending += amount;
        else if (betType == OE_CRAPS_SIDE_FIELD) session.fieldBet += amount;
        else if (betType == OE_CRAPS_SIDE_PLACE) session.AddPlaceBet(target, amount);
    }

    protected void ClearPlaceBets(OECrapsSession session)
    {
        session.place4 = 0;
        session.place5 = 0;
        session.place6 = 0;
        session.place8 = 0;
        session.place9 = 0;
        session.place10 = 0;
    }

    protected void ClearComePoints(OECrapsSession session)
    {
        session.come4 = 0;
        session.come5 = 0;
        session.come6 = 0;
        session.come8 = 0;
        session.come9 = 0;
        session.come10 = 0;
    }

    protected void ClearDontComePoints(OECrapsSession session)
    {
        session.dontCome4 = 0;
        session.dontCome5 = 0;
        session.dontCome6 = 0;
        session.dontCome8 = 0;
        session.dontCome9 = 0;
        session.dontCome10 = 0;
    }

    protected void AddSideSummary(OECrapsSession session, string text)
    {
        if (!session || text == "") return;
        if (session.lastSideSummary != "") session.lastSideSummary += " | ";
        session.lastSideSummary += text;
    }

    protected string GetSideBetLabel(int betType, int target)
    {
        if (betType == OE_CRAPS_SIDE_ODDS) return "ODDS";
        if (betType == OE_CRAPS_SIDE_COME) return "COME";
        if (betType == OE_CRAPS_SIDE_DONT_COME) return "DON'T COME";
        if (betType == OE_CRAPS_SIDE_FIELD) return "FIELD";
        if (betType == OE_CRAPS_SIDE_PLACE) return "PLACE " + target.ToString();
        return "BET";
    }

    protected string GetLineBetName(int lineBetType)
    {
        if (lineBetType == OE_CRAPS_BET_DONT_PASS_LINE) return "DON'T PASS";
        return "PASS LINE";
    }

    protected string GetActiveMessage(OECrapsSession session)
    {
        if (!session) return "Active Craps table.";
        if (!session.lineBetActive)
            return "Side bets are still working. Choose PASS LINE or DON'T PASS and start the next Come-Out roll.";
        if (session.phase == OE_CRAPS_PHASE_POINT)
            return "POINT " + session.point.ToString() + " is active. Add table bets or roll again.";
        return "Come-Out roll in progress.";
    }

    protected OECrapsSession GetTableSession(PlayerIdentity sender)
    {
        if (!sender || !m_Sessions) return null;
        OECrapsSession session;
        if (!m_Sessions.Find(sender.GetId(), session)) return null;
        if (!session || session.finished) return null;
        return session;
    }

    protected PlayerBase ResolvePlayer(PlayerIdentity sender, Object target)
    {
        PlayerBase player = PlayerBase.Cast(target);
        if (!sender || !player || !player.GetIdentity()) return null;
        if (player.GetIdentity().GetId() != sender.GetId()) return null;
        return player;
    }

    protected OECasinoStation ValidateStation(PlayerBase player, string stationId)
    {
        if (!m_Config || !player) return null;
        OECasinoStation station = m_Config.GetStationById(stationId);
        if (!station) return null;
        if (vector.Distance(player.GetPosition(), station.position) > station.playDistance) return null;
        return station;
    }

    protected void SendIdleState(PlayerBase player, string stationId)
    {
        OECrapsNetState state = new OECrapsNetState();
        state.status = OE_CRAPS_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = "Choose PASS LINE or DON'T PASS, set your wager, then roll.";
        SendState(player, state);
    }

    protected void CopySideState(OECrapsSession session, OECrapsNetState state)
    {
        state.oddsBet = session.oddsBet;
        state.comePending = session.comePending;
        state.dontComePending = session.dontComePending;
        state.fieldBet = session.fieldBet;

        state.place4 = session.place4;
        state.place5 = session.place5;
        state.place6 = session.place6;
        state.place8 = session.place8;
        state.place9 = session.place9;
        state.place10 = session.place10;

        state.come4 = session.come4;
        state.come5 = session.come5;
        state.come6 = session.come6;
        state.come8 = session.come8;
        state.come9 = session.come9;
        state.come10 = session.come10;

        state.dontCome4 = session.dontCome4;
        state.dontCome5 = session.dontCome5;
        state.dontCome6 = session.dontCome6;
        state.dontCome8 = session.dontCome8;
        state.dontCome9 = session.dontCome9;
        state.dontCome10 = session.dontCome10;
    }

    protected void SendSessionState(PlayerBase player, OECrapsSession session, int status, string text)
    {
        OECrapsNetState state = new OECrapsNetState();
        state.status = status;
        state.balance = m_Currency.GetBalance(player);
        if (session.lineBetActive) state.bet = session.originalBet;
        else state.bet = 0;
        state.lineBetType = session.lineBetType;
        state.phase = session.phase;
        state.point = session.point;
        state.die1 = session.lastDie1;
        state.die2 = session.lastDie2;
        state.total = session.lastTotal;
        state.payout = session.payout;
        state.netResult = session.netResult;
        state.rollNumber = session.rollNumber;
        state.roundActive = session.lineBetActive;
        state.tableActive = !session.finished;
        state.sideBetTotal = session.GetSideBetTotal();
        state.lastSideReturn = session.lastSideReturn;
        state.lastSideNet = session.lastSideNet;
        state.sideSummary = session.lastSideSummary;
        state.stationId = session.stationId;
        state.message = text;
        CopySideState(session, state);
        SendState(player, state);
    }

    protected void SendRejected(PlayerBase player, string stationId, string text)
    {
        OECrapsNetState state = new OECrapsNetState();
        state.status = OE_CRAPS_STATUS_REJECTED;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OECrapsNetState state = new OECrapsNetState();
        state.status = OE_CRAPS_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendState(PlayerBase player, OECrapsNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CRAPS_STATE, new Param1<ref OECrapsNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OECrapsSession session, string outcome, int lineStake)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;

        FPrintln(file, "CRAPS | " + player.GetIdentity().GetPlainId() + " | lineBet=" + lineStake + " | line=" + GetLineBetName(session.lineBetType) + " | rolls=" + session.rollNumber + " | lastDice=" + session.lastDie1 + "+" + session.lastDie2 + " | total=" + session.lastTotal + " | outcome=" + outcome + " | linePayout=" + session.payout + " | lineNet=" + session.netResult + " | sideNet=" + session.lastSideNet + " | side=" + session.lastSideSummary + " | station=" + session.stationId);
        CloseFile(file);
    }
}
