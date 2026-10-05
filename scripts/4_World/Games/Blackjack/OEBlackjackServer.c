class OEBlackjackServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OEBlackjackSession> m_Sessions;
    // Extra strong-reference registry for active split hands.
    // Normal Blackjack continues to use m_Sessions exactly as before.
    protected ref array<ref OEBlackjackSession> m_ActiveSplitSessions;

    void OEBlackjackServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OEBlackjackSession>;
        m_ActiveSplitSessions = new array<ref OEBlackjackSession>;
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
        if (!station || station.game != OE_CASINO_GAME_BLACKJACK || !m_Config.blackjack.enabled)
        {
            SendError(player, stationId, "This Blackjack table is not available.");
            return;
        }

        OEBlackjackSession session = GetActiveSession(sender);
        if (session)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
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

        if (!station || station.game != OE_CASINO_GAME_BLACKJACK || !m_Config.blackjack.enabled)
        {
            SendError(player, stationId, "This Blackjack table is not available.");
            return;
        }

        if (bet < m_Config.blackjack.minBet || bet > m_Config.blackjack.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        string uid = sender.GetId();
        OEBlackjackSession oldSession;
        if (m_Sessions.Find(uid, oldSession) && oldSession && !oldSession.finished)
        {
            SendState(player, oldSession, OE_BJ_STATUS_PLAYING, "You already have an active Blackjack hand.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        OEBlackjackSession session = new OEBlackjackSession(uid, stationId, bet);
        m_Sessions.Set(uid, session);

        // Standard alternating deal: player, dealer, player, dealer.
        session.playerCards.Insert(session.deck.Draw());
        session.dealerCards.Insert(session.deck.Draw());
        session.playerCards.Insert(session.deck.Draw());
        session.dealerCards.Insert(session.deck.Draw());

        ResolveInitialBlackjacks(player, session);
        if (!session.finished)
            SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
    }

    void HandleAction(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEBlackjackSession session = GetActiveSession(sender);
        if (!session)
        {
            SendError(player, "", "No active Blackjack hand.");
            return;
        }

        Print("[OperationExileCasino][Blackjack] Action Player=" + sender.GetId() + " Action=" + req.param1.ToString() + " Split=" + session.splitActive.ToString() + " ActiveHand=" + session.activeSplitHand.ToString() + " Finished=" + session.finished.ToString());

        if (!ValidateStation(player, session.stationId))
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (req.param1 == OE_BJ_ACTION_HIT)
            PlayerHit(player, session);
        else if (req.param1 == OE_BJ_ACTION_STAND)
            PlayerStand(player, session);
        else if (req.param1 == OE_BJ_ACTION_DOUBLE)
            PlayerDouble(player, session);
        else if (req.param1 == OE_BJ_ACTION_SPLIT)
            PlayerSplit(player, session);
    }

    protected void PlayerHit(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished) return;

        if (session.splitActive)
        {
            SplitPlayerHit(player, session);
            return;
        }

        OEPlayingCard card = session.deck.Draw();
        if (!card) return;
        session.playerCards.Insert(card);

        bool soft = false;
        int total = GetHandValue(session.playerCards, soft);
        if (total > 21)
        {
            FinishLoss(player, session, "Bust with " + total.ToString() + ". Dealer wins.");
            return;
        }

        if (total == 21)
        {
            DealerTurn(player, session);
            return;
        }

        SendState(player, session, OE_BJ_STATUS_PLAYING, "Choose HIT or STAND.");
    }

    protected void PlayerStand(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished) return;

        if (session.splitActive)
        {
            FinishCurrentSplitHand(player, session);
            return;
        }

        DealerTurn(player, session);
    }

    protected void PlayerDouble(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished) return;

        // Operation Exile house rule: no Double Down after a split.
        if (session.splitActive)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Double Down is not available after splitting.");
            return;
        }

        if (m_Config.blackjack.allowDoubleDown != 1) return;
        if (session.playerCards.Count() != 2)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Double Down is only available on your first two cards.");
            return;
        }

        if (!m_Currency.TryDebit(player, session.originalBet))
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Not enough casino currency to Double Down.");
            return;
        }

        session.bet += session.originalBet;
        OEPlayingCard card = session.deck.Draw();
        if (!card) return;
        session.playerCards.Insert(card);

        bool soft = false;
        int total = GetHandValue(session.playerCards, soft);
        if (total > 21)
        {
            FinishLoss(player, session, "Double Down bust with " + total.ToString() + ". Dealer wins.");
            return;
        }

        DealerTurn(player, session);
    }

    protected void PlayerSplit(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished) return;

        // One split only. Re-splitting is intentionally not part of Operation Exile Blackjack.
        if (session.splitActive)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Re-splitting is not available.");
            return;
        }

        if (!CanSplitInitialPair(session))
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Split is only available on an opening pair.");
            return;
        }

        // Match the original wager to create the second hand.
        if (!m_Currency.TryDebit(player, session.originalBet))
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, "Not enough casino currency to Split.");
            return;
        }

        session.splitActive = true;
        session.activeSplitHand = 1;

        // Snapshot the split wager once and use these dedicated values for the
        // entire split round.  Do not depend on originalBet/bet during final
        // settlement; those fields are also used by the legacy single-hand path.
        session.splitHandBet = session.originalBet;
        session.splitTotalStake = session.splitHandBet * 2;
        session.bet = session.splitTotalStake;
        RememberActiveSplitSession(session);
        Print("[OperationExileCasino][Blackjack] Split started Player=" + session.playerId + " Station=" + session.stationId + " Wager=" + session.bet.ToString());

        OEPlayingCard firstCard = session.playerCards.Get(0);
        OEPlayingCard secondCard = session.playerCards.Get(1);
        session.splitHand1.Insert(firstCard);
        session.splitHand2.Insert(secondCard);
        session.playerCards.Clear();

        // Each split hand receives one new card immediately.
        session.splitHand1.Insert(session.deck.Draw());
        session.splitHand2.Insert(session.deck.Draw());

        session.splitAces = false;
        if (firstCard && firstCard.rank == 14)
            session.splitAces = true;

        // Split Aces receive one card per Ace, then both hands automatically stand.
        if (session.splitAces)
        {
            session.splitHand1Finished = true;
            session.splitHand2Finished = true;
            DealerTurnSplit(player, session);
            return;
        }

        // A split-hand 21 is a normal 21, not a natural Blackjack, and auto-stands.
        bool hand1Soft = false;
        int hand1Total = GetHandValue(session.splitHand1, hand1Soft);
        if (hand1Total == 21)
        {
            session.splitHand1Finished = true;
            session.activeSplitHand = 2;
        }

        bool hand2Soft = false;
        int hand2Total = GetHandValue(session.splitHand2, hand2Soft);
        if (session.activeSplitHand == 2 && hand2Total == 21)
        {
            session.splitHand2Finished = true;
            DealerTurnSplit(player, session);
            return;
        }

        SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
    }

    protected void SplitPlayerHit(PlayerBase player, OEBlackjackSession session)
    {
        if (!session.splitActive) return;

        if (!session.deck)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
            return;
        }

        OEPlayingCard card = session.deck.Draw();
        if (!card)
        {
            SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
            return;
        }

        bool soft = false;
        int total = 0;
        if (session.activeSplitHand == 1)
        {
            session.splitHand1.Insert(card);
            total = GetHandValue(session.splitHand1, soft);
        }
        else
        {
            session.splitHand2.Insert(card);
            total = GetHandValue(session.splitHand2, soft);
        }

        if (total >= 21)
        {
            FinishCurrentSplitHand(player, session);
            return;
        }

        SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
    }

    protected void FinishCurrentSplitHand(PlayerBase player, OEBlackjackSession session)
    {
        if (!session || !session.splitActive) return;

        if (session.activeSplitHand == 1)
        {
            session.splitHand1Finished = true;
            session.activeSplitHand = 2;

            bool hand2Soft = false;
            int hand2Total = GetHandValue(session.splitHand2, hand2Soft);
            if (hand2Total >= 21)
            {
                session.splitHand2Finished = true;
                DealerTurnSplit(player, session);
                return;
            }

            SendState(player, session, OE_BJ_STATUS_PLAYING, GetPlayingPrompt(session));
            return;
        }

        session.splitHand2Finished = true;
        DealerTurnSplit(player, session);
    }

    protected void DealerTurn(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished) return;

        bool dealerSoft = false;
        int dealerTotal = GetHandValue(session.dealerCards, dealerSoft);

        while (dealerTotal < 17 || (dealerTotal == 17 && dealerSoft && m_Config.blackjack.dealerHitsSoft17))
        {
            OEPlayingCard card = session.deck.Draw();
            if (!card) break;
            session.dealerCards.Insert(card);
            dealerTotal = GetHandValue(session.dealerCards, dealerSoft);
        }

        bool playerSoft = false;
        int playerTotal = GetHandValue(session.playerCards, playerSoft);

        if (dealerTotal > 21)
        {
            FinishWin(player, session, "Dealer busts with " + dealerTotal.ToString() + ". You win!");
            return;
        }

        if (playerTotal > dealerTotal)
        {
            FinishWin(player, session, "You win " + playerTotal.ToString() + " to " + dealerTotal.ToString() + ".");
            return;
        }

        if (playerTotal < dealerTotal)
        {
            FinishLoss(player, session, "Dealer wins " + dealerTotal.ToString() + " to " + playerTotal.ToString() + ".");
            return;
        }

        FinishPush(player, session, "Push at " + playerTotal.ToString() + ". Your stake is returned.");
    }

    protected void DealerTurnSplit(PlayerBase player, OEBlackjackSession session)
    {
        if (!player || !session || session.finished || !session.splitActive) return;

        bool dealerSoft = false;
        int dealerTotal = GetHandValue(session.dealerCards, dealerSoft);

        while (dealerTotal < 17 || (dealerTotal == 17 && dealerSoft && m_Config.blackjack.dealerHitsSoft17))
        {
            OEPlayingCard card = session.deck.Draw();
            if (!card) break;
            session.dealerCards.Insert(card);
            dealerTotal = GetHandValue(session.dealerCards, dealerSoft);
        }

        // Use the wager snapshot captured at the instant the pair was split.
        // This keeps split settlement independent from the legacy single-hand
        // wager fields and fixes the previous 1-chip settlement corruption.
        int splitWager = session.splitHandBet;
        int totalStake = session.splitTotalStake;
        if (splitWager <= 0)
            splitWager = session.originalBet;
        if (totalStake <= 0)
            totalStake = splitWager * 2;

        OEBlackjackSplitSettlement hand1Settlement = SettleSplitHand(session.splitHand1, dealerTotal, splitWager, "HAND 1");
        OEBlackjackSplitSettlement hand2Settlement = SettleSplitHand(session.splitHand2, dealerTotal, splitWager, "HAND 2");

        session.splitHand1Result = hand1Settlement.shortResult;
        session.splitHand2Result = hand2Settlement.shortResult;
        int totalReturn = hand1Settlement.payout + hand2Settlement.payout;

        if (totalReturn > 0)
        {
            if (!m_Currency.Credit(player, totalReturn))
            {
                SendError(player, session.stationId, "Split payout could not be created. Contact an admin.");
                return;
            }
        }

        // Restore the authoritative split stake snapshot before publishing the
        // final state.  RESULT is always total return minus both split wagers.
        session.splitHandBet = splitWager;
        session.splitTotalStake = totalStake;
        session.bet = totalStake;
        session.originalBet = splitWager;
        session.splitNetResult = totalReturn - totalStake;
        session.finished = true;

        int status = OE_BJ_STATUS_PUSH;
        if (session.splitNetResult > 0)
            status = OE_BJ_STATUS_PLAYER_WIN;
        else if (session.splitNetResult < 0)
            status = OE_BJ_STATUS_DEALER_WIN;

        Print("[OperationExileCasino][Blackjack] Split settlement Player=" + session.playerId + " HandBet=" + splitWager.ToString() + " TotalStake=" + totalStake.ToString() + " Return=" + totalReturn.ToString() + " Net=" + session.splitNetResult.ToString());

        string message = hand1Settlement.detail + "  " + hand2Settlement.detail + "  Paid " + totalReturn.ToString() + " chips.";
        SendState(player, session, status, message);
        LogResult(player, session, totalReturn, "split");
    }

    protected OEBlackjackSplitSettlement SettleSplitHand(array<ref OEPlayingCard> cards, int dealerTotal, int wager, string handName)
    {
        OEBlackjackSplitSettlement result = new OEBlackjackSplitSettlement();
        bool soft = false;
        int total = GetHandValue(cards, soft);

        if (total > 21)
        {
            result.shortResult = "LOSS -" + wager.ToString();
            result.detail = handName + " busts with " + total.ToString() + ".";
            result.payout = 0;
            return result;
        }

        if (dealerTotal > 21 || total > dealerTotal)
        {
            result.payout = Math.Floor(wager * m_Config.blackjack.normalWinPayout);
            int profit = result.payout - wager;
            result.shortResult = "WIN +" + profit.ToString();
            if (dealerTotal > 21)
                result.detail = handName + " wins " + total.ToString() + " - dealer busts.";
            else
                result.detail = handName + " wins " + total.ToString() + " to " + dealerTotal.ToString() + ".";
            return result;
        }

        if (total < dealerTotal)
        {
            result.shortResult = "LOSS -" + wager.ToString();
            result.detail = handName + " loses " + total.ToString() + " to " + dealerTotal.ToString() + ".";
            result.payout = 0;
            return result;
        }

        result.shortResult = "PUSH 0";
        result.detail = handName + " pushes at " + total.ToString() + ".";
        result.payout = wager;
        return result;
    }

    protected void ResolveInitialBlackjacks(PlayerBase player, OEBlackjackSession session)
    {
        bool playerBJ = IsBlackjack(session.playerCards);
        bool dealerBJ = IsBlackjack(session.dealerCards);

        if (playerBJ && dealerBJ)
        {
            FinishPush(player, session, "Both have Blackjack. Push - your stake is returned.");
            return;
        }

        if (playerBJ)
        {
            int payout = Math.Floor(session.bet * m_Config.blackjack.blackjackPayout);
            if (!m_Currency.Credit(player, payout))
            {
                SendError(player, session.stationId, "Blackjack payout could not be created. Contact an admin.");
                return;
            }

            session.finished = true;
            SendState(player, session, OE_BJ_STATUS_BLACKJACK, "BLACKJACK! Paid " + payout.ToString() + " chips.");
            LogResult(player, session, payout, "blackjack");
            return;
        }

        if (dealerBJ)
        {
            FinishLoss(player, session, "Dealer has Blackjack. The wager is lost.");
        }
    }

    protected void FinishWin(PlayerBase player, OEBlackjackSession session, string message)
    {
        int payout = Math.Floor(session.bet * m_Config.blackjack.normalWinPayout);
        if (!m_Currency.Credit(player, payout))
        {
            SendError(player, session.stationId, "Payout could not be created. Contact an admin.");
            return;
        }

        session.finished = true;
        SendState(player, session, OE_BJ_STATUS_PLAYER_WIN, message + " Paid " + payout.ToString() + " chips.");
        LogResult(player, session, payout, "win");
    }

    protected void FinishLoss(PlayerBase player, OEBlackjackSession session, string message)
    {
        session.finished = true;
        SendState(player, session, OE_BJ_STATUS_DEALER_WIN, message);
        LogResult(player, session, 0, "loss");
    }

    protected void FinishPush(PlayerBase player, OEBlackjackSession session, string message)
    {
        if (!m_Currency.Credit(player, session.bet))
        {
            SendError(player, session.stationId, "Returned stake could not be created. Contact an admin.");
            return;
        }

        session.finished = true;
        SendState(player, session, OE_BJ_STATUS_PUSH, message);
        LogResult(player, session, session.bet, "push");
    }

    protected bool CanSplitInitialPair(OEBlackjackSession session)
    {
        if (!session || session.finished || session.splitActive) return false;
        if (!session.playerCards || session.playerCards.Count() != 2) return false;

        OEPlayingCard card1 = session.playerCards.Get(0);
        OEPlayingCard card2 = session.playerCards.Get(1);
        if (!card1 || !card2) return false;

        // House rule: an actual rank pair is required (K+K yes, K+Q no).
        return card1.rank == card2.rank;
    }

    protected string GetPlayingPrompt(OEBlackjackSession session)
    {
        if (!session) return "Choose HIT or STAND.";

        if (session.splitActive)
        {
            if (session.activeSplitHand == 1)
                return "HAND 1 - Choose HIT or STAND.";
            return "HAND 2 - Choose HIT or STAND.";
        }

        if (CanSplitInitialPair(session))
            return "Choose HIT, STAND, DOUBLE DOWN or SPLIT.";

        if (session.playerCards && session.playerCards.Count() == 2)
            return "Choose HIT, STAND or DOUBLE DOWN.";

        return "Choose HIT or STAND.";
    }

    protected bool IsBlackjack(array<ref OEPlayingCard> cards)
    {
        if (!cards || cards.Count() != 2) return false;
        bool soft = false;
        return GetHandValue(cards, soft) == 21;
    }

    protected int GetHandValue(array<ref OEPlayingCard> cards, out bool soft)
    {
        soft = false;
        if (!cards) return 0;

        int total = 0;
        int aces = 0;
        foreach (OEPlayingCard card : cards)
        {
            if (!card) continue;
            if (card.rank == 14)
            {
                total += 11;
                aces++;
            }
            else if (card.rank >= 11)
            {
                total += 10;
            }
            else
            {
                total += card.rank;
            }
        }

        while (total > 21 && aces > 0)
        {
            total -= 10;
            aces--;
        }

        if (aces > 0 && total <= 21)
            soft = true;

        return total;
    }

    protected int GetVisibleDealerValue(OEBlackjackSession session)
    {
        if (!session || !session.dealerCards || session.dealerCards.Count() == 0) return 0;
        OEPlayingCard card = session.dealerCards.Get(0);
        if (!card) return 0;
        if (card.rank == 14) return 11;
        if (card.rank >= 11) return 10;
        return card.rank;
    }

    protected string BuildCardsString(array<ref OEPlayingCard> cards, int visibleCount = -1)
    {
        if (!cards) return "";
        int count = cards.Count();
        if (visibleCount >= 0 && visibleCount < count) count = visibleCount;

        string result = "";
        for (int i = 0; i < count; i++)
        {
            OEPlayingCard card = cards.Get(i);
            if (!card) continue;
            if (result != "") result += ";";
            result += card.rank.ToString() + "," + card.suit.ToString();
        }
        return result;
    }

    protected OEBlackjackSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender) return null;

        string uid = sender.GetId();
        OEBlackjackSession session;

        // Primary path: unchanged from the stable pre-split Blackjack builds.
        if (m_Sessions.Find(uid, session) && session)
        {
            if (!session.finished)
                return session;

            // A completed split always has BOTH split hands finished before the
            // session itself is marked finished. If the VM reports a split
            // session as finished while a hand is still open, recover it rather
            // than dropping the player's live round.
            if (session.splitActive && (!session.splitHand1Finished || !session.splitHand2Finished))
            {
                Print("[OperationExileCasino][Blackjack] Recovering prematurely-finished split session for Player=" + uid);
                session.finished = false;
                return session;
            }
        }

        // Split-only fallback. This is intentionally separate from the normal
        // Blackjack session path so the stable single-hand game is untouched.
        if (m_ActiveSplitSessions)
        {
            foreach (OEBlackjackSession splitSession : m_ActiveSplitSessions)
            {
                if (!splitSession) continue;
                if (splitSession.playerId != uid) continue;

                if (!splitSession.finished)
                {
                    Print("[OperationExileCasino][Blackjack] Recovered active split session from fallback registry for Player=" + uid);
                    m_Sessions.Set(uid, splitSession);
                    return splitSession;
                }

                if (!splitSession.splitHand1Finished || !splitSession.splitHand2Finished)
                {
                    Print("[OperationExileCasino][Blackjack] Recovered inconsistent split session from fallback registry for Player=" + uid);
                    splitSession.finished = false;
                    m_Sessions.Set(uid, splitSession);
                    return splitSession;
                }
            }
        }

        Print("[OperationExileCasino][Blackjack] No active session found for Player=" + uid);
        return null;
    }

    protected void RememberActiveSplitSession(OEBlackjackSession session)
    {
        if (!session || !m_ActiveSplitSessions) return;

        foreach (OEBlackjackSession existing : m_ActiveSplitSessions)
        {
            if (existing == session)
                return;
        }

        m_ActiveSplitSessions.Insert(session);
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
        OEBlackjackNetState state = new OEBlackjackNetState();
        state.status = OE_BJ_STATUS_IDLE;
        state.bet = 0;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = "Set your wager, then PLACE BET & DEAL.";
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_STATE, new Param1<ref OEBlackjackNetState>(state), true, player.GetIdentity());
    }

    protected void SendState(PlayerBase player, OEBlackjackSession session, int status, string text)
    {
        OEBlackjackNetState state = new OEBlackjackNetState();
        state.status = status;
        if (session.splitActive && session.splitTotalStake > 0)
            state.bet = session.splitTotalStake;
        else
            state.bet = session.bet;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = session.stationId;
        state.message = text;

        bool dealerSoft = false;
        bool hideDealerHole = status == OE_BJ_STATUS_PLAYING;
        if (hideDealerHole)
        {
            state.dealerTotal = GetVisibleDealerValue(session);
            state.dealerCards = BuildCardsString(session.dealerCards, 1);
            state.dealerHiddenCards = Math.Max(0, session.dealerCards.Count() - 1);
        }
        else
        {
            state.dealerTotal = GetHandValue(session.dealerCards, dealerSoft);
            state.dealerCards = BuildCardsString(session.dealerCards);
            state.dealerHiddenCards = 0;
        }

        state.splitActive = session.splitActive;
        if (session.splitActive)
        {
            bool hand1Soft = false;
            bool hand2Soft = false;
            state.activeSplitHand = session.activeSplitHand;
            if (session.splitHandBet > 0)
                state.splitHandBet = session.splitHandBet;
            else
                state.splitHandBet = session.originalBet;
            state.splitHand1Total = GetHandValue(session.splitHand1, hand1Soft);
            state.splitHand2Total = GetHandValue(session.splitHand2, hand2Soft);
            state.splitHand1Cards = BuildCardsString(session.splitHand1);
            state.splitHand2Cards = BuildCardsString(session.splitHand2);
            state.splitHand1Result = session.splitHand1Result;
            state.splitHand2Result = session.splitHand2Result;
            state.splitNetResult = session.splitNetResult;
            state.playerTotal = 0;
            state.playerCards = "";

            bool activeDone = false;
            if (session.activeSplitHand == 1)
                activeDone = session.splitHand1Finished;
            else
                activeDone = session.splitHand2Finished;

            state.canHit = status == OE_BJ_STATUS_PLAYING && !activeDone && !session.splitAces;
            state.canStand = status == OE_BJ_STATUS_PLAYING && !activeDone && !session.splitAces;
            state.canDouble = false;
            state.canSplit = false;
        }
        else
        {
            bool playerSoft = false;
            state.playerTotal = GetHandValue(session.playerCards, playerSoft);
            state.playerCards = BuildCardsString(session.playerCards);
            state.canHit = status == OE_BJ_STATUS_PLAYING;
            state.canStand = status == OE_BJ_STATUS_PLAYING;
            state.canDouble = status == OE_BJ_STATUS_PLAYING && m_Config.blackjack.allowDoubleDown == 1 && session.playerCards.Count() == 2;
            state.canSplit = status == OE_BJ_STATUS_PLAYING && CanSplitInitialPair(session);
        }

        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_STATE, new Param1<ref OEBlackjackNetState>(state), true, player.GetIdentity());
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OEBlackjackNetState state = new OEBlackjackNetState();
        state.status = OE_BJ_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_STATE, new Param1<ref OEBlackjackNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OEBlackjackSession session, int payout, string outcome)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        Print("[OperationExileCasino][Blackjack] Player=" + player.GetIdentity().GetId() + " Station=" + session.stationId + " Wager=" + session.bet.ToString() + " Outcome=" + outcome + " Payout=" + payout.ToString());
    }
}
