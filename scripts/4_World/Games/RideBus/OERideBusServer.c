class OERideBusServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OERideBusSession> m_Sessions;

    void OERideBusServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OERideBusSession>;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.TickTimeouts, 500, true);
    }

    void ~OERideBusServer()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.TickTimeouts);
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
        if (!station || station.game != OE_CASINO_GAME_RIDE_BUS || !m_Config.rideBus.enabled)
        {
            SendError(player, stationId, "This Ride the Bus table is not available.");
            return;
        }

        OERideBusSession session = GetActiveSession(sender);
        if (session)
        {
            if (IsDecisionExpired(session))
            {
                TimeoutSession(player, session);
                return;
            }

            SendState(player, session, OE_RTB_STATUS_PLAYING, GetRoundPrompt(session.round));
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleStart(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param2<string, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        int bet = req.param2;
        OECasinoStation station = ValidateStation(player, stationId);

        if (!station || station.game != OE_CASINO_GAME_RIDE_BUS || !m_Config.rideBus.enabled)
        {
            SendError(player, stationId, "This Ride the Bus table is not available.");
            return;
        }

        if (bet < m_Config.rideBus.minBet || bet > m_Config.rideBus.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        string uid = sender.GetId();
        OERideBusSession oldSession;
        if (m_Sessions.Find(uid, oldSession) && oldSession && !oldSession.finished)
        {
            SendState(player, oldSession, OE_RTB_STATUS_PLAYING, "You already have an active game.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        OERideBusSession session = new OERideBusSession(uid, stationId, bet);
        PrepareHiddenCard(session);
        StartDecisionWindow(session);
        m_Sessions.Set(uid, session);
        SendState(player, session, OE_RTB_STATUS_PLAYING, "Round 1: RED or BLACK?");
    }

    void HandleChoice(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OERideBusSession session = GetActiveSession(sender);
        if (!session)
        {
            SendError(player, "", "No active Ride the Bus game.");
            return;
        }

        if (!ValidateStation(player, session.stationId))
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (IsDecisionExpired(session))
        {
            TimeoutSession(player, session);
            return;
        }

        ProcessChoice(player, session, req.param1, false);
    }

    protected void ProcessChoice(PlayerBase player, OERideBusSession session, int choice, bool timedOut)
    {
        if (!player || !session || session.finished) return;

        bool correct = false;
        OEPlayingCard card;
        string timeoutPrefix = "";
        if (timedOut)
            timeoutPrefix = "Time expired - auto-picked " + GetChoiceLabel(choice) + ". ";

        if (session.round == OE_RTB_ROUND_COLOUR)
        {
            if (choice != OE_RTB_CHOICE_RED && choice != OE_RTB_CHOICE_BLACK) return;

            card = TakeHiddenCard(session);
            if (!card) return;
            session.cards.Insert(card);
            correct = (choice == OE_RTB_CHOICE_RED && card.IsRed()) || (choice == OE_RTB_CHOICE_BLACK && !card.IsRed());

            if (correct)
            {
                session.cashoutValue = Math.Floor(session.bet * m_Config.rideBus.round1Multiplier);
                session.round = OE_RTB_ROUND_HIGH_LOW;
                PrepareHiddenCard(session);
                StartDecisionWindow(session);
                SendState(player, session, OE_RTB_STATUS_PLAYING, timeoutPrefix + "Correct. Round 2: HIGHER or LOWER?");
                return;
            }
        }
        else if (session.round == OE_RTB_ROUND_HIGH_LOW)
        {
            if (choice != OE_RTB_CHOICE_HIGHER && choice != OE_RTB_CHOICE_LOWER) return;

            OEPlayingCard first = session.cards.Get(0);
            card = TakeHiddenCard(session);
            if (!card) return;
            session.cards.Insert(card);

            if (choice == OE_RTB_CHOICE_HIGHER)
                correct = card.rank > first.rank || (m_Config.rideBus.equalCountsAsHigher && card.rank == first.rank);
            else
                correct = card.rank < first.rank;

            if (correct)
            {
                session.cashoutValue = Math.Floor(session.bet * m_Config.rideBus.round2Multiplier);
                session.round = OE_RTB_ROUND_IN_OUT;
                PrepareHiddenCard(session);
                StartDecisionWindow(session);
                SendState(player, session, OE_RTB_STATUS_PLAYING, timeoutPrefix + "Correct. Round 3: INSIDE or OUTSIDE?");
                return;
            }
        }
        else if (session.round == OE_RTB_ROUND_IN_OUT)
        {
            if (choice != OE_RTB_CHOICE_INSIDE && choice != OE_RTB_CHOICE_OUTSIDE) return;

            OEPlayingCard a = session.cards.Get(0);
            OEPlayingCard b = session.cards.Get(1);
            card = TakeHiddenCard(session);
            if (!card) return;
            session.cards.Insert(card);

            int low = Math.Min(a.rank, b.rank);
            int high = Math.Max(a.rank, b.rank);
            bool inside;
            if (m_Config.rideBus.insideIsInclusive)
                inside = card.rank >= low && card.rank <= high;
            else
                inside = card.rank > low && card.rank < high;

            correct = (choice == OE_RTB_CHOICE_INSIDE && inside) || (choice == OE_RTB_CHOICE_OUTSIDE && !inside);
            if (correct)
            {
                session.cashoutValue = Math.Floor(session.bet * m_Config.rideBus.round3Multiplier);
                session.round = OE_RTB_ROUND_SUIT;
                PrepareHiddenCard(session);
                StartDecisionWindow(session);
                SendState(player, session, OE_RTB_STATUS_PLAYING, timeoutPrefix + "Correct. Final round: pick the SUIT.");
                return;
            }
        }
        else if (session.round == OE_RTB_ROUND_SUIT)
        {
            int suit = ChoiceToSuit(choice);
            if (suit < 0) return;

            card = TakeHiddenCard(session);
            if (!card) return;
            session.cards.Insert(card);
            correct = card.suit == suit;

            if (correct)
            {
                session.cashoutValue = Math.Floor(session.bet * m_Config.rideBus.round4Multiplier);
                session.decisionDeadlineMs = 0;

                if (!m_Currency.Credit(player, session.cashoutValue))
                {
                    SendError(player, session.stationId, "Payout could not be created. Contact an admin.");
                    return;
                }

                session.finished = true;
                SendState(player, session, OE_RTB_STATUS_WON, timeoutPrefix + "Route complete! Paid " + session.cashoutValue.ToString() + " chips.");
                LogResult(player, session, session.cashoutValue);
                return;
            }
        }

        session.decisionDeadlineMs = 0;
        session.cashoutValue = 0;
        session.finished = true;
        if (timedOut)
            SendState(player, session, OE_RTB_STATUS_LOST, timeoutPrefix + "Wrong call. The wager is lost.");
        else
            SendState(player, session, OE_RTB_STATUS_LOST, "Wrong call. The wager is lost.");
        LogResult(player, session, 0);
    }

    void HandleCashOut(PlayerIdentity sender, Object target)
    {
        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OERideBusSession session = GetActiveSession(sender);
        if (!session || session.cashoutValue <= 0 || !m_Config.rideBus.allowCashOut)
        {
            string stationId = "";
            if (session) stationId = session.stationId;
            SendError(player, stationId, "Nothing is available to collect.");
            return;
        }

        if (!ValidateStation(player, session.stationId))
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        CashOutSession(player, session, "Forfeited and collected " + session.cashoutValue.ToString() + " chips.");
    }

    protected void TickTimeouts()
    {
        if (!m_Sessions) return;

        foreach (string uid, OERideBusSession session : m_Sessions)
        {
            if (!session || session.finished || session.decisionDeadlineMs <= 0) continue;
            if (!IsDecisionExpired(session)) continue;

            PlayerBase player = FindPlayerByUid(uid);
            if (!player) continue;

            TimeoutSession(player, session);
        }
    }

    protected void TimeoutSession(PlayerBase player, OERideBusSession session)
    {
        if (!player || !session || session.finished) return;

        string behavior = "lose";
        if (m_Config && m_Config.rideBus)
            behavior = m_Config.rideBus.timeoutBehavior;

        if (behavior == "first_choice")
        {
            int firstChoice = GetFirstChoiceForRound(session.round);
            if (firstChoice >= 0)
            {
                ProcessChoice(player, session, firstChoice, true);
                return;
            }
        }

        // Default timeout: reveal the already-drawn hidden card, then lose the wager.
        OEPlayingCard timedOutCard = TakeHiddenCard(session);
        if (timedOutCard)
            session.cards.Insert(timedOutCard);

        session.decisionDeadlineMs = 0;
        session.cashoutValue = 0;
        session.finished = true;
        SendState(player, session, OE_RTB_STATUS_LOST, "Time expired. The wager is lost.");
        LogResult(player, session, 0);
    }

    protected int GetFirstChoiceForRound(int round)
    {
        if (round == OE_RTB_ROUND_COLOUR) return OE_RTB_CHOICE_RED;
        if (round == OE_RTB_ROUND_HIGH_LOW) return OE_RTB_CHOICE_HIGHER;
        if (round == OE_RTB_ROUND_IN_OUT) return OE_RTB_CHOICE_INSIDE;
        if (round == OE_RTB_ROUND_SUIT) return OE_RTB_CHOICE_HEARTS;
        return -1;
    }

    protected string GetChoiceLabel(int choice)
    {
        if (choice == OE_RTB_CHOICE_RED) return "RED";
        if (choice == OE_RTB_CHOICE_BLACK) return "BLACK";
        if (choice == OE_RTB_CHOICE_HIGHER) return "HIGHER";
        if (choice == OE_RTB_CHOICE_LOWER) return "LOWER";
        if (choice == OE_RTB_CHOICE_INSIDE) return "INSIDE";
        if (choice == OE_RTB_CHOICE_OUTSIDE) return "OUTSIDE";
        if (choice == OE_RTB_CHOICE_HEARTS) return "HEARTS";
        if (choice == OE_RTB_CHOICE_CLUBS) return "CLUBS";
        if (choice == OE_RTB_CHOICE_DIAMONDS) return "DIAMONDS";
        if (choice == OE_RTB_CHOICE_SPADES) return "SPADES";
        return "FIRST OPTION";
    }

    protected void CashOutSession(PlayerBase player, OERideBusSession session, string message)
    {
        if (!player || !session || session.finished) return;

        int payout = session.cashoutValue;
        if (payout <= 0)
        {
            session.decisionDeadlineMs = 0;
            session.finished = true;
            SendState(player, session, OE_RTB_STATUS_CASHED, message);
            LogResult(player, session, 0);
            return;
        }

        if (!m_Currency.Credit(player, payout))
        {
            SendError(player, session.stationId, "Payout could not be created. Contact an admin.");
            return;
        }

        session.decisionDeadlineMs = 0;
        session.finished = true;
        SendState(player, session, OE_RTB_STATUS_CASHED, message);
        LogResult(player, session, payout);
    }

    protected void PrepareHiddenCard(OERideBusSession session)
    {
        if (!session || !session.deck) return;
        session.pendingCard = session.deck.Draw();
    }

    protected OEPlayingCard TakeHiddenCard(OERideBusSession session)
    {
        if (!session) return null;
        OEPlayingCard card = session.pendingCard;
        session.pendingCard = null;
        return card;
    }

    protected void StartDecisionWindow(OERideBusSession session)
    {
        if (!session || !m_Config || !m_Config.rideBus) return;
        session.decisionDeadlineMs = GetGame().GetTime() + (m_Config.rideBus.choiceSeconds * 1000);
    }

    protected bool IsDecisionExpired(OERideBusSession session)
    {
        if (!session || session.decisionDeadlineMs <= 0) return false;
        return GetGame().GetTime() >= session.decisionDeadlineMs;
    }

    protected int GetSecondsRemaining(OERideBusSession session)
    {
        if (!session || session.decisionDeadlineMs <= 0) return 0;
        int remainingMs = session.decisionDeadlineMs - GetGame().GetTime();
        if (remainingMs <= 0) return 0;
        return (remainingMs + 999) / 1000;
    }

    protected PlayerBase FindPlayerByUid(string uid)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player || !player.GetIdentity()) continue;
            if (player.GetIdentity().GetId() == uid) return player;
        }

        return null;
    }

    protected void SendIdleState(PlayerBase player, string stationId)
    {
        OERideBusNetState state = new OERideBusNetState();
        state.status = OE_RTB_STATUS_IDLE;
        state.round = OE_RTB_ROUND_COLOUR;
        state.bet = 0;
        state.cashoutValue = 0;
        state.balance = m_Currency.GetBalance(player);
        state.decisionSeconds = 0;
        state.stationId = stationId;
        state.message = "Set your wager, then PLACE BET & DEAL.";
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_STATE, new Param1<ref OERideBusNetState>(state), true, player.GetIdentity());
    }

    protected string GetRoundPrompt(int round)
    {
        if (round == OE_RTB_ROUND_COLOUR) return "Round 1: RED or BLACK?";
        if (round == OE_RTB_ROUND_HIGH_LOW) return "Round 2: HIGHER or LOWER?";
        if (round == OE_RTB_ROUND_IN_OUT) return "Round 3: INSIDE or OUTSIDE?";
        if (round == OE_RTB_ROUND_SUIT) return "Final round: pick the SUIT.";
        return "Ride the Bus game in progress.";
    }

    protected int ChoiceToSuit(int choice)
    {
        if (choice == OE_RTB_CHOICE_HEARTS) return OE_CARD_SUIT_HEARTS;
        if (choice == OE_RTB_CHOICE_DIAMONDS) return OE_CARD_SUIT_DIAMONDS;
        if (choice == OE_RTB_CHOICE_CLUBS) return OE_CARD_SUIT_CLUBS;
        if (choice == OE_RTB_CHOICE_SPADES) return OE_CARD_SUIT_SPADES;
        return -1;
    }

    protected OERideBusSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender) return null;
        OERideBusSession session;
        if (m_Sessions.Find(sender.GetId(), session) && session && !session.finished)
            return session;
        return null;
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
        OECasinoStation station = m_Config.GetStationById(stationId);
        if (!station || !player) return null;
        if (vector.Distance(player.GetPosition(), station.position) > station.playDistance) return null;
        return station;
    }

    protected void SendState(PlayerBase player, OERideBusSession session, int status, string text)
    {
        OERideBusNetState state = new OERideBusNetState();
        state.status = status;
        state.round = session.round;
        state.bet = session.bet;
        state.cashoutValue = session.cashoutValue;
        state.balance = m_Currency.GetBalance(player);
        state.decisionSeconds = GetSecondsRemaining(session);
        state.stationId = session.stationId;
        state.message = text;

        if (session.cards.Count() > 0)
        {
            state.card1 = session.cards.Get(0).ToDisplayString();
            state.card1Rank = session.cards.Get(0).rank;
            state.card1Suit = session.cards.Get(0).suit;
        }
        if (session.cards.Count() > 1)
        {
            state.card2 = session.cards.Get(1).ToDisplayString();
            state.card2Rank = session.cards.Get(1).rank;
            state.card2Suit = session.cards.Get(1).suit;
        }
        if (session.cards.Count() > 2)
        {
            state.card3 = session.cards.Get(2).ToDisplayString();
            state.card3Rank = session.cards.Get(2).rank;
            state.card3Suit = session.cards.Get(2).suit;
        }
        if (session.cards.Count() > 3)
        {
            state.card4 = session.cards.Get(3).ToDisplayString();
            state.card4Rank = session.cards.Get(3).rank;
            state.card4Suit = session.cards.Get(3).suit;
        }

        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_STATE, new Param1<ref OERideBusNetState>(state), true, player.GetIdentity());
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OERideBusNetState state = new OERideBusNetState();
        state.status = OE_RTB_STATUS_ERROR;
        state.stationId = stationId;
        state.balance = m_Currency.GetBalance(player);
        state.message = text;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_STATE, new Param1<ref OERideBusNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OERideBusSession session, int payout)
    {
        if (!m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;
        FPrintln(file, "RIDE_BUS | " + player.GetIdentity().GetPlainId() + " | bet=" + session.bet + " | payout=" + payout + " | station=" + session.stationId);
        CloseFile(file);
    }
}
