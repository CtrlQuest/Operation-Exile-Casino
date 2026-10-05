class OEHighCardWarServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OEHighCardWarSession> m_Sessions;

    void OEHighCardWarServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OEHighCardWarSession>;
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
        if (!station || station.game != OE_CASINO_GAME_HIGH_CARD_WAR || !m_Config.highCardWar || m_Config.highCardWar.enabled != 1)
        {
            SendError(player, stationId, "This High Card / War table is not available.");
            return;
        }

        OEHighCardWarSession session = GetActiveSession(sender);
        if (session)
        {
            SendSessionState(player, session, OE_WAR_STATUS_TIE, "TIE! Choose SURRENDER or GO TO WAR.");
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

        if (!station || station.game != OE_CASINO_GAME_HIGH_CARD_WAR || !m_Config.highCardWar || m_Config.highCardWar.enabled != 1)
        {
            SendError(player, stationId, "This High Card / War table is not available.");
            return;
        }

        if (bet < m_Config.highCardWar.minBet || bet > m_Config.highCardWar.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        OEHighCardWarSession active = GetActiveSession(sender);
        if (active)
        {
            SendSessionState(player, active, OE_WAR_STATUS_TIE, "Finish your current tie before starting another hand.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        int deckCount = m_Config.highCardWar.deckCount;
        OEHighCardWarSession session = new OEHighCardWarSession(sender.GetId(), stationId, bet, deckCount);
        m_Sessions.Set(sender.GetId(), session);

        session.playerCard = session.deck.Draw();
        session.dealerCard = session.deck.Draw();
        if (!session.playerCard || !session.dealerCard)
        {
            m_Currency.Credit(player, bet);
            session.finished = true;
            SendError(player, stationId, "The card deck could not be dealt. Your wager was returned.");
            return;
        }

        ResolveInitialDeal(player, session);
    }

    void HandleAction(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEHighCardWarSession session = GetActiveSession(sender);
        if (!session || !session.waitingOnTieDecision)
        {
            SendError(player, "", "No active High Card / War tie.");
            return;
        }

        if (!ValidateStation(player, session.stationId))
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (req.param1 == OE_WAR_ACTION_SURRENDER)
        {
            Surrender(player, session);
            return;
        }

        if (req.param1 == OE_WAR_ACTION_GO_TO_WAR)
        {
            GoToWar(player, session);
            return;
        }

        SendSessionState(player, session, OE_WAR_STATUS_TIE, "Invalid action. Choose SURRENDER or GO TO WAR.");
    }

    protected void ResolveInitialDeal(PlayerBase player, OEHighCardWarSession session)
    {
        int playerRank = session.playerCard.rank;
        int dealerRank = session.dealerCard.rank;

        if (playerRank > dealerRank)
        {
            int payout = Math.Floor(session.originalBet * m_Config.highCardWar.normalWinPayout);
            if (!CreditOrRestore(player, session, payout, "High Card payout could not be created. Contact an admin.")) return;

            session.payout = payout;
            session.netResult = payout - session.totalStake;
            session.finished = true;
            SendSessionState(player, session, OE_WAR_STATUS_PLAYER_WIN, "Your " + session.playerCard.GetRankName() + " beats the dealer's " + session.dealerCard.GetRankName() + ". Paid " + payout.ToString() + " chips.");
            LogResult(player, session, "win");
            return;
        }

        if (dealerRank > playerRank)
        {
            session.payout = 0;
            session.netResult = 0 - session.totalStake;
            session.finished = true;
            SendSessionState(player, session, OE_WAR_STATUS_DEALER_WIN, "Dealer " + session.dealerCard.GetRankName() + " beats your " + session.playerCard.GetRankName() + ".");
            LogResult(player, session, "loss");
            return;
        }

        session.waitingOnTieDecision = true;
        session.netResult = 0 - session.originalBet;
        SendSessionState(player, session, OE_WAR_STATUS_TIE, "TIE on " + session.playerCard.GetRankName() + "! Surrender half the wager or match it and GO TO WAR.");
    }

    protected void Surrender(PlayerBase player, OEHighCardWarSession session)
    {
        int refund = Math.Floor(session.originalBet * m_Config.highCardWar.surrenderRefundMultiplier);
        if (refund < 0) refund = 0;

        if (refund > 0 && !m_Currency.Credit(player, refund))
        {
            SendSessionState(player, session, OE_WAR_STATUS_TIE, "Surrender refund could not be created. Try again or contact an admin.");
            return;
        }

        session.waitingOnTieDecision = false;
        session.finished = true;
        session.payout = refund;
        session.netResult = refund - session.totalStake;

        SendSessionState(player, session, OE_WAR_STATUS_SURRENDER, "Surrendered. " + refund.ToString() + " chips returned from the original wager.");
        LogResult(player, session, "surrender");
    }

    protected void GoToWar(PlayerBase player, OEHighCardWarSession session)
    {
        if (!m_Currency.TryDebit(player, session.originalBet))
        {
            SendSessionState(player, session, OE_WAR_STATUS_TIE, "You need another " + session.originalBet.ToString() + " chips to GO TO WAR.");
            return;
        }

        session.totalStake = session.originalBet * 2;
        session.waitingOnTieDecision = false;
        session.warRound = true;

        int burns = m_Config.highCardWar.warBurnCards;
        if (burns < 0) burns = 0;
        if (burns > 10) burns = 10;

        for (int i = 0; i < burns; i++)
        {
            OEPlayingCard burned = session.deck.Draw();
            if (!burned) break;
        }

        session.warPlayerCard = session.deck.Draw();
        session.warDealerCard = session.deck.Draw();
        if (!session.warPlayerCard || !session.warDealerCard)
        {
            // Return only the added war wager. The original tie remains live so the
            // player can choose again instead of losing money to a deck failure.
            m_Currency.Credit(player, session.originalBet);
            session.totalStake = session.originalBet;
            session.waitingOnTieDecision = true;
            session.warRound = false;
            session.warPlayerCard = null;
            session.warDealerCard = null;
            SendSessionState(player, session, OE_WAR_STATUS_TIE, "War cards could not be dealt. The added wager was returned.");
            return;
        }

        int playerRank = session.warPlayerCard.rank;
        int dealerRank = session.warDealerCard.rank;
        bool won = playerRank > dealerRank;
        bool secondTie = playerRank == dealerRank;
        if (secondTie && m_Config.highCardWar.secondTieWins == 1)
            won = true;

        if (won)
        {
            int payout = Math.Floor(session.originalBet * m_Config.highCardWar.warWinReturnMultiplier);
            if (!CreditWarPayout(player, session, payout)) return;

            session.payout = payout;
            session.netResult = payout - session.totalStake;
            session.finished = true;

            if (secondTie)
                SendSessionState(player, session, OE_WAR_STATUS_WAR_WIN, "WAR ties on " + session.warPlayerCard.GetRankName() + ". Second tie wins! Paid " + payout.ToString() + " chips.");
            else
                SendSessionState(player, session, OE_WAR_STATUS_WAR_WIN, "You win the WAR: " + session.warPlayerCard.GetRankName() + " beats " + session.warDealerCard.GetRankName() + ". Paid " + payout.ToString() + " chips.");

            LogResult(player, session, "war_win");
            return;
        }

        if (secondTie)
        {
            // Optional non-Vegas config fallback: second tie pushes both wagers.
            int pushReturn = session.totalStake;
            if (!m_Currency.Credit(player, pushReturn))
            {
                SendError(player, session.stationId, "War push could not be returned. Contact an admin.");
                return;
            }

            session.payout = pushReturn;
            session.netResult = 0;
            session.finished = true;
            SendSessionState(player, session, OE_WAR_STATUS_WAR_WIN, "WAR ties again. Both wagers returned.");
            LogResult(player, session, "war_push");
            return;
        }

        session.payout = 0;
        session.netResult = 0 - session.totalStake;
        session.finished = true;
        SendSessionState(player, session, OE_WAR_STATUS_WAR_LOSS, "Dealer wins the WAR: " + session.warDealerCard.GetRankName() + " beats " + session.warPlayerCard.GetRankName() + ". Both wagers are lost.");
        LogResult(player, session, "war_loss");
    }

    protected bool CreditOrRestore(PlayerBase player, OEHighCardWarSession session, int payout, string errorText)
    {
        if (m_Currency.Credit(player, payout)) return true;

        // Best effort restore of the original wager so a payout creation issue
        // cannot silently turn a winning hand into a loss.
        m_Currency.Credit(player, session.originalBet);
        session.finished = true;
        SendError(player, session.stationId, errorText);
        return false;
    }

    protected bool CreditWarPayout(PlayerBase player, OEHighCardWarSession session, int payout)
    {
        if (m_Currency.Credit(player, payout)) return true;

        // Best effort restore of both wagers if the payout cannot be created.
        m_Currency.Credit(player, session.totalStake);
        session.finished = true;
        SendError(player, session.stationId, "WAR payout could not be created. Both wagers were restored where possible; contact an admin.");
        return false;
    }

    protected OEHighCardWarSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender || !m_Sessions) return null;
        OEHighCardWarSession session;
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
        OEHighCardWarNetState state = new OEHighCardWarNetState();
        state.status = OE_WAR_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = "Set your wager, then DEAL CARDS.";
        SendState(player, state);
    }

    protected void SendSessionState(PlayerBase player, OEHighCardWarSession session, int status, string text)
    {
        OEHighCardWarNetState state = new OEHighCardWarNetState();
        state.status = status;
        state.bet = session.originalBet;
        state.totalStake = session.totalStake;
        state.balance = m_Currency.GetBalance(player);
        state.payout = session.payout;
        state.netResult = session.netResult;
        state.stationId = session.stationId;
        state.message = text;
        state.warRound = session.warRound;
        state.canSurrender = status == OE_WAR_STATUS_TIE && session.waitingOnTieDecision;
        state.canGoToWar = status == OE_WAR_STATUS_TIE && session.waitingOnTieDecision;

        if (session.playerCard)
        {
            state.playerRank = session.playerCard.rank;
            state.playerSuit = session.playerCard.suit;
        }
        if (session.dealerCard)
        {
            state.dealerRank = session.dealerCard.rank;
            state.dealerSuit = session.dealerCard.suit;
        }
        if (session.warPlayerCard)
        {
            state.warPlayerRank = session.warPlayerCard.rank;
            state.warPlayerSuit = session.warPlayerCard.suit;
        }
        if (session.warDealerCard)
        {
            state.warDealerRank = session.warDealerCard.rank;
            state.warDealerSuit = session.warDealerCard.suit;
        }

        SendState(player, state);
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OEHighCardWarNetState state = new OEHighCardWarNetState();
        state.status = OE_WAR_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendState(PlayerBase player, OEHighCardWarNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_WAR_STATE, new Param1<ref OEHighCardWarNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OEHighCardWarSession session, string outcome)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;

        string firstCards = "";
        if (session.playerCard && session.dealerCard)
            firstCards = session.playerCard.ToDisplayString() + " vs " + session.dealerCard.ToDisplayString();

        string warCards = "";
        if (session.warPlayerCard && session.warDealerCard)
            warCards = session.warPlayerCard.ToDisplayString() + " vs " + session.warDealerCard.ToDisplayString();

        FPrintln(file, "HIGH_CARD_WAR | " + player.GetIdentity().GetPlainId() + " | bet=" + session.originalBet + " | totalStake=" + session.totalStake + " | first=" + firstCards + " | war=" + warCards + " | outcome=" + outcome + " | payout=" + session.payout + " | net=" + session.netResult + " | station=" + session.stationId);
        CloseFile(file);
    }
}
