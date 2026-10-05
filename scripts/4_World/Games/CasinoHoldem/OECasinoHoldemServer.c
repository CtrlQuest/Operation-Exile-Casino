class OECasinoHoldemServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OECasinoHoldemSession> m_Sessions;

    void OECasinoHoldemServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OECasinoHoldemSession>;
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
        if (!station || station.game != OE_CASINO_GAME_CASINO_HOLDEM || !m_Config.casinoHoldem || m_Config.casinoHoldem.enabled != 1)
        {
            SendError(player, stationId, "This Texas Hold'em table is not available.");
            return;
        }

        OECasinoHoldemSession session = GetActiveSession(sender);
        if (session)
        {
            if (session.stationId != stationId)
            {
                SendRejected(player, stationId, "You already have an active Hold'em hand at another table.");
                return;
            }

            SendPlayerTurnState(player, session, "Hand restored. Continue the " + GetStreetName(session.street) + " betting round.");
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
        int openingBet = req.param2;
        OECasinoStation station = ValidateStation(player, stationId);

        if (!station || station.game != OE_CASINO_GAME_CASINO_HOLDEM || !m_Config.casinoHoldem || m_Config.casinoHoldem.enabled != 1)
        {
            SendError(player, stationId, "This Texas Hold'em table is not available.");
            return;
        }

        if (openingBet < m_Config.casinoHoldem.minBet || openingBet > m_Config.casinoHoldem.maxBet)
        {
            SendRejected(player, stationId, "BET must be between " + m_Config.casinoHoldem.minBet.ToString() + " and " + m_Config.casinoHoldem.maxBet.ToString() + " chips.");
            return;
        }

        OECasinoHoldemSession active = GetActiveSession(sender);
        if (active)
        {
            SendPlayerTurnState(player, active, "Finish your current Hold'em hand before dealing another one.");
            return;
        }

        if (!m_Currency.TryDebit(player, openingBet))
        {
            SendRejected(player, stationId, "Not enough casino currency. Your opening BET was not placed.");
            return;
        }

        OECasinoHoldemSession session = new OECasinoHoldemSession(sender.GetId(), stationId, openingBet);
        m_Sessions.Set(sender.GetId(), session);

        // Deal only the two private cards to each side. Community cards are
        // revealed in proper Hold'em streets after each betting round.
        session.playerCards.Insert(session.deck.Draw());
        session.dealerCards.Insert(session.deck.Draw());
        session.playerCards.Insert(session.deck.Draw());
        session.dealerCards.Insert(session.deck.Draw());

        if (!CardsValid(session.playerCards, 2) || !CardsValid(session.dealerCards, 2))
        {
            m_Currency.Credit(player, openingBet);
            session.finished = true;
            session.waitingForPlayer = false;
            SendError(player, stationId, "The Poker deck could not be dealt. Your opening BET was returned.");
            return;
        }

        SendPlayerTurnState(player, session, "PRE-FLOP. The dealer matched your opening bet. CHECK or BET " + session.startingBet.ToString() + ".");
    }

    void HandleAction(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param2<int, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OECasinoHoldemSession session = GetActiveSession(sender);
        if (!session || !session.waitingForPlayer)
        {
            SendRejected(player, "", "No active Hold'em decision.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_CASINO_HOLDEM)
        {
            SendError(player, session.stationId, "You moved too far away from the Poker table.");
            return;
        }

        int action = req.param1;
        int requestedRaise = req.param2;
        if (action == OE_HOLDEM_ACTION_FOLD) { PlayerFold(player, session); return; }
        if (action == OE_HOLDEM_ACTION_CHECK) { PlayerCheck(player, session); return; }
        if (action == OE_HOLDEM_ACTION_CALL) { PlayerCall(player, session); return; }
        if (action == OE_HOLDEM_ACTION_RAISE) { PlayerRaise(player, session, requestedRaise); return; }

        SendPlayerTurnState(player, session, "Invalid Poker action.");
    }

    protected void PlayerFold(PlayerBase player, OECasinoHoldemSession session)
    {
        // A player may fold whenever it is their turn, even when checking is
        // available. This mirrors normal poker table behaviour and lets a
        // player abandon a hand immediately after seeing their hole cards.
        session.waitingForPlayer = false;
        session.finished = true;
        session.payout = 0;
        session.netResult = 0 - session.playerContribution;
        session.dealerLastAction = "DEALER WINS BY FOLD";

        SendSessionState(player, session, OE_HOLDEM_STATUS_PLAYER_FOLD, "You folded. Your " + session.playerContribution.ToString() + " chips committed to the pot are lost.");
        LogResult(player, session, "player_fold");
    }

    protected void PlayerCheck(PlayerBase player, OECasinoHoldemSession session)
    {
        if (session.AmountToCall() > 0)
        {
            SendPlayerTurnState(player, session, "The dealer has bet. CALL, RAISE or FOLD.");
            return;
        }

        session.waitingForPlayer = false;
        DealerRespondToCheck(player, session);
    }

    protected void PlayerCall(PlayerBase player, OECasinoHoldemSession session)
    {
        int amount = session.AmountToCall();
        if (amount <= 0)
        {
            SendPlayerTurnState(player, session, "There is nothing to CALL. CHECK or BET instead.");
            return;
        }

        if (!m_Currency.TryDebit(player, amount))
        {
            SendPlayerTurnState(player, session, "CALL NOT PLACED. You need " + amount.ToString() + " chips to continue, or you can FOLD.");
            return;
        }

        session.playerStreetBet = session.playerStreetBet + amount;
        session.playerContribution = session.playerContribution + amount;
        session.RefreshPot();
        session.waitingForPlayer = false;
        session.dealerLastAction = "DEALER BET CALLED";

        AdvanceStreet(player, session, "You called " + amount.ToString() + ".");
    }

    protected void PlayerRaise(PlayerBase player, OECasinoHoldemSession session, int requestedRaise)
    {
        if (!CanRaise(session))
        {
            SendPlayerTurnState(player, session, "The raise limit for this betting round has been reached. CHECK/CALL or FOLD.");
            return;
        }

        int minRaise = GetMinimumRaiseAmount();
        int maxRaise = GetPlayerMaxRaiseAmount(player, session);
        if (requestedRaise < minRaise || requestedRaise > maxRaise)
        {
            SendPlayerTurnState(player, session, "RAISE NOT PLACED. Choose between " + minRaise.ToString() + " and " + maxRaise.ToString() + " chips.");
            return;
        }

        int toCall = session.AmountToCall();
        int raiseBy = requestedRaise;
        int totalPayment = toCall + raiseBy;

        if (!m_Currency.TryDebit(player, totalPayment))
        {
            string actionWord = "BET";
            if (toCall > 0) actionWord = "RAISE";
            SendPlayerTurnState(player, session, actionWord + " NOT PLACED. You need " + totalPayment.ToString() + " chips for this action.");
            return;
        }

        session.playerStreetBet = session.dealerStreetBet + raiseBy;
        session.playerContribution = session.playerContribution + totalPayment;
        session.raisesThisStreet = session.raisesThisStreet + 1;
        session.RefreshPot();
        session.waitingForPlayer = false;

        DealerRespondToPlayerBet(player, session);
    }

    protected void DealerRespondToCheck(PlayerBase player, OECasinoHoldemSession session)
    {
        int strength = GetDealerStrength(session);
        int aggression = GetDealerAggression();
        int bluff = GetDealerBluffChance();

        int betChance = bluff;
        if (strength >= 80) betChance = 76;
        else if (strength >= 65) betChance = 58;
        else if (strength >= 50) betChance = 38;
        else if (strength >= 35) betChance = 22;

        betChance = betChance + ((aggression - 50) / 2);
        if (betChance < 5) betChance = 5;
        if (betChance > 92) betChance = 92;

        bool canBet = CanRaise(session);
        int roll = Math.RandomInt(0, 100);
        if (canBet && roll < betChance)
        {
            int amount = GetDealerRaiseSize(player, session, strength, strength < 40);
            if (amount <= 0)
            {
                session.dealerLastAction = "DEALER CHECKS";
                AdvanceStreet(player, session, "You checked. Dealer checks behind.");
                return;
            }
            session.dealerStreetBet = session.dealerStreetBet + amount;
            session.dealerContribution = session.dealerContribution + amount;
            session.raisesThisStreet = session.raisesThisStreet + 1;
            session.RefreshPot();
            session.waitingForPlayer = true;

            session.dealerLastAction = "DEALER BETS " + amount.ToString();

            SendPlayerTurnState(player, session, "Dealer bets " + amount.ToString() + ". CALL " + session.AmountToCall().ToString() + ", RAISE or FOLD.");
            return;
        }

        session.dealerLastAction = "DEALER CHECKS";
        AdvanceStreet(player, session, "You checked. Dealer checks behind.");
    }

    protected void DealerRespondToPlayerBet(PlayerBase player, OECasinoHoldemSession session)
    {
        int toCall = session.playerStreetBet - session.dealerStreetBet;
        if (toCall <= 0)
        {
            AdvanceStreet(player, session, "Betting round complete.");
            return;
        }

        int strength = GetDealerStrength(session);
        int aggression = GetDealerAggression();
        int bluff = GetDealerBluffChance();
        bool canReRaise = CanRaise(session);

        // Weak hands can still bluff. This is deliberately server-side so the
        // client cannot know whether a raise represents strength or a bluff.
        if (strength < 40 && Math.RandomInt(0, 100) < bluff)
        {
            if (canReRaise)
            {
                DealerRaise(player, session, toCall, true);
                return;
            }

            DealerCallAndAdvance(player, session, toCall, true);
            return;
        }

        int foldChance = 0;
        int raiseChance = 0;

        if (strength >= 82)
        {
            foldChance = 0;
            raiseChance = 45;
        }
        else if (strength >= 68)
        {
            foldChance = 4;
            raiseChance = 32;
        }
        else if (strength >= 55)
        {
            foldChance = 12;
            raiseChance = 20;
        }
        else if (strength >= 42)
        {
            foldChance = 30;
            raiseChance = 10;
        }
        else
        {
            foldChance = 62;
            raiseChance = 5;
        }

        foldChance = foldChance - ((aggression - 50) / 3);
        raiseChance = raiseChance + ((aggression - 50) / 3);
        if (foldChance < 0) foldChance = 0;
        if (foldChance > 85) foldChance = 85;
        if (raiseChance < 0) raiseChance = 0;
        if (raiseChance > 70) raiseChance = 70;

        int decision = Math.RandomInt(0, 100);
        if (decision < foldChance)
        {
            DealerFold(player, session);
            return;
        }

        if (canReRaise && decision < (foldChance + raiseChance))
        {
            DealerRaise(player, session, toCall, false);
            return;
        }

        DealerCallAndAdvance(player, session, toCall, false);
    }

    protected void DealerRaise(PlayerBase player, OECasinoHoldemSession session, int toCall, bool bluffing)
    {
        int strength = GetDealerStrength(session);
        int raiseBy = GetDealerRaiseSize(player, session, strength, bluffing);
        if (raiseBy <= 0)
        {
            DealerCallAndAdvance(player, session, toCall, bluffing);
            return;
        }
        int payment = toCall + raiseBy;
        session.dealerStreetBet = session.playerStreetBet + raiseBy;
        session.dealerContribution = session.dealerContribution + payment;
        session.raisesThisStreet = session.raisesThisStreet + 1;
        session.RefreshPot();
        session.waitingForPlayer = true;

        session.dealerLastAction = "DEALER RAISES +" + raiseBy.ToString();

        SendPlayerTurnState(player, session, "Dealer raises. CALL " + session.AmountToCall().ToString() + ", RAISE again or FOLD.");
    }

    protected void DealerCallAndAdvance(PlayerBase player, OECasinoHoldemSession session, int toCall, bool bluffCall)
    {
        session.dealerStreetBet = session.dealerStreetBet + toCall;
        session.dealerContribution = session.dealerContribution + toCall;
        session.RefreshPot();
        session.dealerLastAction = "DEALER CALLS " + toCall.ToString();

        AdvanceStreet(player, session, "Dealer calls " + toCall.ToString() + ".");
    }

    protected void DealerFold(PlayerBase player, OECasinoHoldemSession session)
    {
        session.waitingForPlayer = false;
        session.finished = true;
        session.payout = session.pot;
        session.netResult = session.payout - session.playerContribution;
        session.dealerLastAction = "DEALER FOLDS";

        if (!CreditPayoutOrRestore(player, session, session.payout)) return;

        SendSessionState(player, session, OE_HOLDEM_STATUS_DEALER_FOLD, "Dealer folds to your bet. You take the " + session.pot.ToString() + " chip pot.");
        LogResult(player, session, "dealer_fold");
    }

    protected void AdvanceStreet(PlayerBase player, OECasinoHoldemSession session, string previousAction)
    {
        session.playerStreetBet = 0;
        session.dealerStreetBet = 0;
        session.raisesThisStreet = 0;

        if (session.street == OE_HOLDEM_STREET_PREFLOP)
        {
            if (!DealCommunity(player, session, 3)) return;
            session.street = OE_HOLDEM_STREET_FLOP;
            session.waitingForPlayer = true;
            UpdateVisiblePlayerHand(session);
            SendPlayerTurnState(player, session, previousAction + " FLOP dealt. Your action.");
            return;
        }

        if (session.street == OE_HOLDEM_STREET_FLOP)
        {
            if (!DealCommunity(player, session, 1)) return;
            session.street = OE_HOLDEM_STREET_TURN;
            session.waitingForPlayer = true;
            UpdateVisiblePlayerHand(session);
            SendPlayerTurnState(player, session, previousAction + " TURN dealt. Your action.");
            return;
        }

        if (session.street == OE_HOLDEM_STREET_TURN)
        {
            if (!DealCommunity(player, session, 1)) return;
            session.street = OE_HOLDEM_STREET_RIVER;
            session.waitingForPlayer = true;
            UpdateVisiblePlayerHand(session);
            SendPlayerTurnState(player, session, previousAction + " RIVER dealt. Final betting round.");
            return;
        }

        if (session.street == OE_HOLDEM_STREET_RIVER)
        {
            session.street = OE_HOLDEM_STREET_SHOWDOWN;
            ResolveShowdown(player, session);
            return;
        }
    }

    protected bool DealCommunity(PlayerBase player, OECasinoHoldemSession session, int count)
    {
        int beforeCount = session.communityCards.Count();
        for (int i = 0; i < count; i++)
        {
            OEPlayingCard card = session.deck.Draw();
            if (!card)
            {
                while (session.communityCards.Count() > beforeCount)
                    session.communityCards.Remove(session.communityCards.Count() - 1);

                RestorePlayerStake(player, session);
                session.finished = true;
                session.waitingForPlayer = false;
                SendError(player, session.stationId, "A community card could not be dealt. Your committed chips were restored where possible.");
                return false;
            }
            session.communityCards.Insert(card);
        }
        return true;
    }

    protected void ResolveShowdown(PlayerBase player, OECasinoHoldemSession session)
    {
        array<ref OEPlayingCard> playerSeven = BuildSeven(session.playerCards, session.communityCards);
        array<ref OEPlayingCard> dealerSeven = BuildSeven(session.dealerCards, session.communityCards);

        OEPokerHandResult playerResult = OEPokerHandEvaluator.EvaluateBest(playerSeven);
        OEPokerHandResult dealerResult = OEPokerHandEvaluator.EvaluateBest(dealerSeven);

        if (!playerResult || !dealerResult)
        {
            RestorePlayerStake(player, session);
            session.finished = true;
            session.waitingForPlayer = false;
            SendError(player, session.stationId, "Poker hand evaluation failed. Your committed chips were restored where possible.");
            return;
        }

        session.playerHandName = playerResult.name;
        session.dealerHandName = dealerResult.name;
        session.waitingForPlayer = false;
        session.finished = true;
        session.dealerLastAction = "SHOWDOWN";

        if (playerResult.score > dealerResult.score)
        {
            int payout = session.pot;
            if (!CreditPayoutOrRestore(player, session, payout)) return;
            session.payout = payout;
            session.netResult = payout - session.playerContribution;
            SendSessionState(player, session, OE_HOLDEM_STATUS_PLAYER_WIN, "SHOWDOWN. Your " + playerResult.name + " beats the dealer's " + dealerResult.name + ". You take the " + payout.ToString() + " chip pot.");
            LogResult(player, session, "showdown_win");
            return;
        }

        if (dealerResult.score > playerResult.score)
        {
            session.payout = 0;
            session.netResult = 0 - session.playerContribution;
            SendSessionState(player, session, OE_HOLDEM_STATUS_DEALER_WIN, "SHOWDOWN. Dealer " + dealerResult.name + " beats your " + playerResult.name + ".");
            LogResult(player, session, "showdown_loss");
            return;
        }

        int pushReturn = session.playerContribution;
        if (!CreditPayoutOrRestore(player, session, pushReturn)) return;
        session.payout = pushReturn;
        session.netResult = 0;
        SendSessionState(player, session, OE_HOLDEM_STATUS_PUSH, "SHOWDOWN PUSH. Both best five-card hands are " + playerResult.name + ". Your committed chips are returned.");
        LogResult(player, session, "showdown_push");
    }

    protected void UpdateVisiblePlayerHand(OECasinoHoldemSession session)
    {
        if (!session) return;
        if (session.communityCards.Count() < 3)
        {
            session.playerHandName = "";
            return;
        }

        array<ref OEPlayingCard> visible = BuildSeven(session.playerCards, session.communityCards);
        OEPokerHandResult result = OEPokerHandEvaluator.EvaluateBest(visible);
        if (result) session.playerHandName = result.name;
    }

    protected int GetDealerStrength(OECasinoHoldemSession session)
    {
        if (!session || !session.dealerCards || session.dealerCards.Count() < 2) return 20;

        OEPlayingCard first = session.dealerCards.Get(0);
        OEPlayingCard second = session.dealerCards.Get(1);
        if (!first || !second) return 20;

        // Pre-flop heuristic using only the dealer's own hidden cards.
        if (session.communityCards.Count() < 3)
        {
            int high = first.rank;
            int low = second.rank;
            if (low > high)
            {
                high = second.rank;
                low = first.rank;
            }

            int strength = 18 + high * 2;
            if (first.rank == second.rank) strength = 58 + high * 2;
            if (first.suit == second.suit) strength = strength + 7;

            int gap = high - low;
            if (gap == 1) strength = strength + 7;
            else if (gap == 2) strength = strength + 4;

            if (high >= 13 && low >= 10) strength = strength + 8;
            if (high == 14) strength = strength + 6;

            if (strength > 96) strength = 96;
            if (strength < 10) strength = 10;
            return strength;
        }

        array<ref OEPlayingCard> visible = BuildSeven(session.dealerCards, session.communityCards);
        OEPokerHandResult result = OEPokerHandEvaluator.EvaluateBest(visible);
        if (!result) return 25;

        int score = 18;
        if (result.category == OE_POKER_HAND_HIGH_CARD) score = 26;
        else if (result.category == OE_POKER_HAND_PAIR) score = 48;
        else if (result.category == OE_POKER_HAND_TWO_PAIR) score = 63;
        else if (result.category == OE_POKER_HAND_THREE_KIND) score = 72;
        else if (result.category == OE_POKER_HAND_STRAIGHT) score = 79;
        else if (result.category == OE_POKER_HAND_FLUSH) score = 83;
        else if (result.category == OE_POKER_HAND_FULL_HOUSE) score = 90;
        else if (result.category == OE_POKER_HAND_FOUR_KIND) score = 96;
        else if (result.category >= OE_POKER_HAND_STRAIGHT_FLUSH) score = 99;

        score = score + (result.primaryRank / 4);
        if (score > 99) score = 99;
        return score;
    }

    protected int GetDealerBluffChance()
    {
        int value = 12;
        if (m_Config && m_Config.casinoHoldem) value = m_Config.casinoHoldem.dealerBluffPercent;
        if (value < 0) value = 0;
        if (value > 50) value = 50;
        return value;
    }

    protected int GetDealerAggression()
    {
        int value = 55;
        if (m_Config && m_Config.casinoHoldem) value = m_Config.casinoHoldem.dealerAggressionPercent;
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        return value;
    }

    protected int GetMaxRaises()
    {
        int value = 2;
        if (m_Config && m_Config.casinoHoldem) value = m_Config.casinoHoldem.maxRaisesPerStreet;
        if (value < 1) value = 1;
        if (value > 5) value = 5;
        return value;
    }

    protected bool IsNoLimit()
    {
        if (!m_Config || !m_Config.casinoHoldem) return true;
        return m_Config.casinoHoldem.noLimit;
    }

    protected bool CanRaise(OECasinoHoldemSession session)
    {
        if (!session) return false;
        if (IsNoLimit()) return true;
        return session.raisesThisStreet < GetMaxRaises();
    }

    protected int GetMinimumRaiseAmount()
    {
        int value = 10;
        if (m_Config && m_Config.casinoHoldem) value = m_Config.casinoHoldem.minBet;
        if (value < 1) value = 1;
        return value;
    }

    protected int GetMaxRaiseMultiplier()
    {
        int value = 3;
        if (m_Config && m_Config.casinoHoldem) value = m_Config.casinoHoldem.dealerMaxRaiseMultiplier;
        if (value < 1) value = 1;
        if (value > 10) value = 10;
        return value;
    }

    protected int GetPlayerMaxRaiseAmount(PlayerBase player, OECasinoHoldemSession session)
    {
        if (!player || !session) return 0;
        int available = m_Currency.GetBalance(player);
        int toCall = session.AmountToCall();
        int maxRaise = available - toCall;
        if (maxRaise < 0) maxRaise = 0;

        // Restricted mode preserves the old configurable ceiling.
        if (!IsNoLimit())
        {
            int restrictedMax = GetMaxRaiseAmount(session);
            if (maxRaise > restrictedMax) maxRaise = restrictedMax;
        }
        return maxRaise;
    }

    protected int GetMaxRaiseAmount(OECasinoHoldemSession session)
    {
        if (!session || session.startingBet < 1) return GetMinimumRaiseAmount();
        int value = session.startingBet * GetMaxRaiseMultiplier();
        int minRaise = GetMinimumRaiseAmount();
        if (value < minRaise) value = minRaise;
        return value;
    }

    protected int GetDealerRaiseSize(PlayerBase player, OECasinoHoldemSession session, int strength, bool bluffing)
    {
        int unit = session.startingBet;
        if (unit < GetMinimumRaiseAmount()) unit = GetMinimumRaiseAmount();

        int multiplier = 1;
        int aggression = GetDealerAggression();
        int roll = Math.RandomInt(0, 100);

        if (bluffing)
        {
            // Most bluffs are smaller; an aggressive dealer can occasionally
            // represent more strength with a 2x opening-bet raise.
            if (aggression >= 65 && roll < 35) multiplier = 2;
        }
        else if (strength >= 82)
        {
            if (roll < 35) multiplier = 3;
            else if (roll < 80) multiplier = 2;
        }
        else if (strength >= 65)
        {
            if (roll < 35) multiplier = 2;
        }

        int amount = unit * multiplier;
        int maxRaise = GetMaxRaiseAmount(session);
        if (amount > maxRaise) amount = maxRaise;

        // Heads-up effective-stack protection: the dealer never makes a new
        // raise larger than the player could actually match. This avoids
        // creating an uncallable virtual-house bet when the player is nearly
        // or fully committed. The dealer can still CALL any legal player bet.
        int playerRemaining = m_Currency.GetBalance(player);
        if (amount > playerRemaining) amount = playerRemaining;
        if (amount < GetMinimumRaiseAmount()) return 0;
        return amount;
    }

    protected string GetStreetName(int street)
    {
        if (street == OE_HOLDEM_STREET_PREFLOP) return "PRE-FLOP";
        if (street == OE_HOLDEM_STREET_FLOP) return "FLOP";
        if (street == OE_HOLDEM_STREET_TURN) return "TURN";
        if (street == OE_HOLDEM_STREET_RIVER) return "RIVER";
        return "SHOWDOWN";
    }

    protected array<ref OEPlayingCard> BuildSeven(array<ref OEPlayingCard> holeCards, array<ref OEPlayingCard> community)
    {
        array<ref OEPlayingCard> cards = new array<ref OEPlayingCard>;
        if (holeCards)
        {
            foreach (OEPlayingCard hole : holeCards)
                if (hole) cards.Insert(hole);
        }
        if (community)
        {
            foreach (OEPlayingCard board : community)
                if (board) cards.Insert(board);
        }
        return cards;
    }

    protected bool CardsValid(array<ref OEPlayingCard> cards, int expectedCount)
    {
        if (!cards || cards.Count() != expectedCount) return false;
        foreach (OEPlayingCard card : cards)
            if (!card) return false;
        return true;
    }

    protected bool CreditPayoutOrRestore(PlayerBase player, OECasinoHoldemSession session, int payout)
    {
        if (m_Currency.Credit(player, payout)) return true;

        RestorePlayerStake(player, session);
        session.finished = true;
        session.waitingForPlayer = false;
        SendError(player, session.stationId, "Poker payout could not be created. Your committed chips were restored where possible; contact an admin.");
        return false;
    }

    protected void RestorePlayerStake(PlayerBase player, OECasinoHoldemSession session)
    {
        if (!player || !session) return;
        if (session.playerContribution > 0)
            m_Currency.Credit(player, session.playerContribution);
    }

    protected OECasinoHoldemSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender || !m_Sessions) return null;
        OECasinoHoldemSession session;
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
        OECasinoHoldemNetState state = new OECasinoHoldemNetState();
        state.status = OE_HOLDEM_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.raiseAmount = 0;
        state.minRaiseAmount = GetMinimumRaiseAmount();
        state.maxRaiseAmount = 0;
        state.communityHiddenCards = 5;
        state.dealerHiddenCards = 2;
        state.message = "Choose an opening BET. The dealer matches it, then PRE-FLOP betting begins.";
        SendState(player, state);
    }

    protected void SendPlayerTurnState(PlayerBase player, OECasinoHoldemSession session, string text)
    {
        if (!session) return;
        session.waitingForPlayer = true;
        SendSessionState(player, session, OE_HOLDEM_STATUS_PLAYER_TURN, text);
    }

    protected void SendSessionState(PlayerBase player, OECasinoHoldemSession session, int status, string text)
    {
        OECasinoHoldemNetState state = new OECasinoHoldemNetState();
        state.status = status;
        state.street = session.street;
        state.startingBet = session.startingBet;
        state.playerContribution = session.playerContribution;
        state.dealerContribution = session.dealerContribution;
        state.playerStreetBet = session.playerStreetBet;
        state.dealerStreetBet = session.dealerStreetBet;
        state.amountToCall = session.AmountToCall();
        state.raiseAmount = session.startingBet;
        state.minRaiseAmount = GetMinimumRaiseAmount();
        state.maxRaiseAmount = GetPlayerMaxRaiseAmount(player, session);
        state.pot = session.pot;
        state.balance = m_Currency.GetBalance(player);
        state.payout = session.payout;
        state.netResult = session.netResult;
        state.raisesThisStreet = session.raisesThisStreet;
        if (IsNoLimit()) state.maxRaisesPerStreet = 0;
        else state.maxRaisesPerStreet = GetMaxRaises();
        state.stationId = session.stationId;
        state.playerCards = BuildCardsString(session.playerCards);
        state.communityCards = BuildCardsString(session.communityCards);
        state.playerHandName = session.playerHandName;
        state.dealerHandName = session.dealerHandName;
        state.dealerLastAction = session.dealerLastAction;
        state.message = text;

        if (status == OE_HOLDEM_STATUS_PLAYER_TURN)
        {
            int toCall = session.AmountToCall();
            state.canCheck = toCall == 0;
            state.canCall = toCall > 0;
            state.canRaise = CanRaise(session) && state.maxRaiseAmount >= state.minRaiseAmount;
            state.canFold = true;
            state.dealerCards = "";
            state.dealerHiddenCards = 2;
            state.communityHiddenCards = 5 - session.communityCards.Count();
            if (state.communityHiddenCards < 0) state.communityHiddenCards = 0;
        }
        else if (status == OE_HOLDEM_STATUS_PLAYER_FOLD || status == OE_HOLDEM_STATUS_DEALER_FOLD)
        {
            state.dealerCards = "";
            state.dealerHiddenCards = 2;
            state.communityHiddenCards = 5 - session.communityCards.Count();
            if (state.communityHiddenCards < 0) state.communityHiddenCards = 0;
        }
        else
        {
            state.dealerCards = BuildCardsString(session.dealerCards);
            state.dealerHiddenCards = 0;
            state.communityHiddenCards = 0;
        }

        SendState(player, state);
    }

    protected void SendRejected(PlayerBase player, string stationId, string text)
    {
        OECasinoHoldemNetState state = new OECasinoHoldemNetState();
        state.status = OE_HOLDEM_STATUS_REJECTED;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.communityHiddenCards = 5;
        state.dealerHiddenCards = 2;
        state.message = text;
        SendState(player, state);
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OECasinoHoldemNetState state = new OECasinoHoldemNetState();
        state.status = OE_HOLDEM_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendState(PlayerBase player, OECasinoHoldemNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_HOLDEM_STATE, new Param1<ref OECasinoHoldemNetState>(state), true, player.GetIdentity());
    }

    protected string BuildCardsString(array<ref OEPlayingCard> cards)
    {
        if (!cards) return "";
        string result = "";
        foreach (OEPlayingCard card : cards)
        {
            if (!card) continue;
            if (result != "") result += ";";
            result += card.rank.ToString() + "," + card.suit.ToString();
        }
        return result;
    }

    protected void LogResult(PlayerBase player, OECasinoHoldemSession session, string outcome)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;

        FPrintln(file, "TEXAS_HOLDEM | " + player.GetIdentity().GetPlainId() + " | openingBet=" + session.startingBet + " | playerContribution=" + session.playerContribution + " | dealerContribution=" + session.dealerContribution + " | pot=" + session.pot + " | street=" + GetStreetName(session.street) + " | playerHand=" + session.playerHandName + " | dealerHand=" + session.dealerHandName + " | dealerAction=" + session.dealerLastAction + " | outcome=" + outcome + " | payout=" + session.payout + " | net=" + session.netResult + " | station=" + session.stationId);
        CloseFile(file);
    }
}
