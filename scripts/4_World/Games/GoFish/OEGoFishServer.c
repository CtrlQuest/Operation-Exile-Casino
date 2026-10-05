class OEGoFishServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OEGoFishSession> m_Sessions;

    void OEGoFishServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OEGoFishSession>;
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
        if (!station || station.game != OE_CASINO_GAME_GO_FISH || !m_Config.goFish.enabled)
        {
            SendError(player, stationId, "This Go Fish table is not available.");
            return;
        }

        OEGoFishSession session = GetActiveSession(sender);
        if (session)
        {
            SendState(player, session, OE_GOFISH_STATUS_ACTIVE, "Your turn. Ask for a rank you hold.", false);
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
        if (!station || station.game != OE_CASINO_GAME_GO_FISH || !m_Config.goFish.enabled)
        {
            SendError(player, stationId, "This Go Fish table is not available.");
            return;
        }

        if (bet < m_Config.goFish.minBet || bet > m_Config.goFish.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        OEGoFishSession active = GetActiveSession(sender);
        if (active)
        {
            SendState(player, active, OE_GOFISH_STATUS_ACTIVE, "You already have an active Go Fish game.", false);
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        OEGoFishSession session = new OEGoFishSession(sender.GetId(), stationId, bet);
        m_Sessions.Set(sender.GetId(), session);

        int startingCards = m_Config.goFish.startingCards;
        for (int i = 0; i < startingCards; i++)
        {
            OEPlayingCard playerCard = session.deck.Draw();
            OEPlayingCard dealerCard = session.deck.Draw();
            if (playerCard) session.playerHand.Insert(playerCard);
            if (dealerCard) session.dealerHand.Insert(dealerCard);
        }

        ExtractBooks(session, session.playerHand, session.playerBookRanks, true);
        ExtractBooks(session, session.dealerHand, session.dealerBookRanks, false);
        RefillEmptyHands(session);

        if (CheckGameOver(player, session, true)) return;
        SendState(player, session, OE_GOFISH_STATUS_ACTIVE, "Your turn. Ask the dealer for a rank you hold.", true);
    }

    void HandleAsk(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<int> req;
        if (!ctx.Read(req)) return;
        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEGoFishSession session = GetActiveSession(sender);
        if (!session)
        {
            SendError(player, "", "No active Go Fish game.");
            return;
        }

        if (!ValidateStation(player, session.stationId))
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        int rank = req.param1;
        if (rank < 2 || rank > 14 || !HasRank(session.playerHand, rank))
        {
            SendState(player, session, OE_GOFISH_STATUS_ACTIVE, "You can only ask for a rank currently in your hand.", false);
            return;
        }

        string rankName = RankName(rank);
        AddDealerMemory(session, rank);
        int transferred = TransferRank(session.dealerHand, session.playerHand, rank);
        if (transferred > 0)
        {
            int booksBefore = session.playerBookRanks.Count();
            ExtractBooks(session, session.playerHand, session.playerBookRanks, true);
            RefillEmptyHands(session);
            if (CheckGameOver(player, session, true)) return;

            string text = "Dealer had " + transferred.ToString() + " " + rankName + " card";
            if (transferred != 1) text += "s";
            text += ". Ask again.";
            if (session.playerBookRanks.Count() > booksBefore)
                text = "BOOK! You completed " + rankName + "s. Ask again.";
            SendState(player, session, OE_GOFISH_STATUS_ACTIVE, text, true);
            return;
        }

        OEPlayingCard drawn = session.deck.Draw();
        if (drawn)
        {
            session.playerHand.Insert(drawn);
            int drawnRank = drawn.rank;
            int booksBeforeDraw = session.playerBookRanks.Count();
            ExtractBooks(session, session.playerHand, session.playerBookRanks, true);
            RefillEmptyHands(session);
            if (CheckGameOver(player, session, true)) return;

            if (drawnRank == rank)
            {
                string luckyText = "GO FISH - you drew " + rankName + ". Ask again.";
                if (session.playerBookRanks.Count() > booksBeforeDraw)
                    luckyText = "GO FISH - " + rankName + " completed a BOOK. Ask again.";
                SendState(player, session, OE_GOFISH_STATUS_ACTIVE, luckyText, true);
                return;
            }
        }

        DealerTurn(player, session, rankName);
    }

    protected void DealerTurn(PlayerBase player, OEGoFishSession session, string playerMissRank)
    {
        if (!player || !session || session.finished) return;
        RefillEmptyHands(session);
        if (CheckGameOver(player, session, true)) return;

        string summary = "GO FISH on " + playerMissRank + ". ";
        int guard = 0;

        while (guard < 40 && !session.finished)
        {
            guard++;
            if (session.dealerHand.Count() == 0)
            {
                RefillEmptyHands(session);
                if (session.dealerHand.Count() == 0) break;
            }

            int rank = ChooseDealerRank(session);
            if (rank < 2) break;
            string rankName = RankName(rank);
            int transferred = TransferRank(session.playerHand, session.dealerHand, rank);

            if (transferred > 0)
            {
                RemoveDealerMemory(session, rank);
                int booksBefore = session.dealerBookRanks.Count();
                ExtractBooks(session, session.dealerHand, session.dealerBookRanks, false);
                RefillEmptyHands(session);
                summary += "Dealer asked for " + rankName + " and took " + transferred.ToString() + ". ";
                if (session.dealerBookRanks.Count() > booksBefore)
                    summary += "Dealer made a BOOK. ";
                if (CheckGameOver(player, session, true)) return;
                continue;
            }

            OEPlayingCard drawn = session.deck.Draw();
            if (drawn)
            {
                session.dealerHand.Insert(drawn);
                int drawnRank = drawn.rank;
                ExtractBooks(session, session.dealerHand, session.dealerBookRanks, false);
                RefillEmptyHands(session);
                if (CheckGameOver(player, session, true)) return;

                if (drawnRank == rank)
                {
                    summary += "Dealer asked for " + rankName + ", went fishing and caught it. ";
                    continue;
                }
            }

            summary += "Dealer asked for " + rankName + " - GO FISH. ";
            break;
        }

        RefillEmptyHands(session);
        if (CheckGameOver(player, session, true)) return;
        summary += "Your turn.";
        SendState(player, session, OE_GOFISH_STATUS_ACTIVE, summary, true);
    }

    protected int ChooseDealerRank(OEGoFishSession session)
    {
        if (!session || !session.dealerHand || session.dealerHand.Count() == 0) return -1;

        array<int> preferred = new array<int>;
        foreach (int remembered : session.dealerMemoryRanks)
        {
            if (HasRank(session.dealerHand, remembered)) preferred.Insert(remembered);
        }
        if (preferred.Count() > 0)
            return preferred.Get(Math.RandomInt(0, preferred.Count()));

        array<int> ranks = new array<int>;
        foreach (OEPlayingCard card : session.dealerHand)
        {
            if (!card) continue;
            if (!ContainsInt(ranks, card.rank)) ranks.Insert(card.rank);
        }
        if (ranks.Count() == 0) return -1;
        return ranks.Get(Math.RandomInt(0, ranks.Count()));
    }

    protected void RefillEmptyHands(OEGoFishSession session)
    {
        if (!session || !session.deck) return;
        int refill = m_Config.goFish.refillCards;
        if (refill < 1) refill = 5;

        if (session.playerHand.Count() == 0 && session.deck.Count() > 0)
            DrawCards(session.deck, session.playerHand, refill);
        if (session.dealerHand.Count() == 0 && session.deck.Count() > 0)
            DrawCards(session.deck, session.dealerHand, refill);

        ExtractBooks(session, session.playerHand, session.playerBookRanks, true);
        ExtractBooks(session, session.dealerHand, session.dealerBookRanks, false);
    }

    protected void DrawCards(OECardDeck deck, array<ref OEPlayingCard> hand, int amount)
    {
        if (!deck || !hand) return;
        for (int i = 0; i < amount; i++)
        {
            OEPlayingCard card = deck.Draw();
            if (!card) return;
            hand.Insert(card);
        }
    }

    protected int TransferRank(array<ref OEPlayingCard> fromHand, array<ref OEPlayingCard> toHand, int rank)
    {
        if (!fromHand || !toHand) return 0;
        int moved = 0;
        for (int i = fromHand.Count() - 1; i >= 0; i--)
        {
            OEPlayingCard card = fromHand.Get(i);
            if (card && card.rank == rank)
            {
                toHand.Insert(card);
                fromHand.Remove(i);
                moved++;
            }
        }
        return moved;
    }

    protected void ExtractBooks(OEGoFishSession session, array<ref OEPlayingCard> hand, array<int> books, bool playerBook)
    {
        if (!session || !hand || !books) return;
        for (int rank = 2; rank <= 14; rank++)
        {
            if (CountRank(hand, rank) < 4) continue;
            for (int i = hand.Count() - 1; i >= 0; i--)
            {
                OEPlayingCard card = hand.Get(i);
                if (card && card.rank == rank) hand.Remove(i);
            }
            if (!ContainsInt(books, rank)) books.Insert(rank);
            if (playerBook) RemoveDealerMemory(session, rank);
        }
    }

    protected bool CheckGameOver(PlayerBase player, OEGoFishSession session, bool animate)
    {
        if (!player || !session) return false;
        if (session.finished) return true;
        int totalBooks = session.playerBookRanks.Count() + session.dealerBookRanks.Count();
        bool noCards = session.deck.Count() == 0 && session.playerHand.Count() == 0 && session.dealerHand.Count() == 0;
        if (totalBooks < 13 && !noCards) return false;

        session.finished = true;
        int playerBooks = session.playerBookRanks.Count();
        int dealerBooks = session.dealerBookRanks.Count();
        if (playerBooks > dealerBooks)
        {
            int payout = Math.Floor(session.bet * m_Config.goFish.winPayout);
            if (payout < session.bet) payout = session.bet;
            m_Currency.Credit(player, payout);
            SendState(player, session, OE_GOFISH_STATUS_PLAYER_WIN, "You beat the dealer " + playerBooks.ToString() + " books to " + dealerBooks.ToString() + ". Payout: " + payout.ToString() + " chips.", animate);
            LogResult(player, session, payout, "PLAYER_WIN");
            return true;
        }
        if (dealerBooks > playerBooks)
        {
            SendState(player, session, OE_GOFISH_STATUS_DEALER_WIN, "Dealer wins " + dealerBooks.ToString() + " books to " + playerBooks.ToString() + ". Your " + session.bet.ToString() + "-chip wager was lost.", animate);
            LogResult(player, session, 0, "DEALER_WIN");
            return true;
        }

        m_Currency.Credit(player, session.bet);
        SendState(player, session, OE_GOFISH_STATUS_PUSH, "Tie game: " + playerBooks.ToString() + " books each. Your " + session.bet.ToString() + "-chip wager was returned.", animate);
        LogResult(player, session, session.bet, "PUSH");
        return true;
    }

    protected bool HasRank(array<ref OEPlayingCard> hand, int rank)
    {
        return CountRank(hand, rank) > 0;
    }

    protected int CountRank(array<ref OEPlayingCard> hand, int rank)
    {
        if (!hand) return 0;
        int count = 0;
        foreach (OEPlayingCard card : hand)
        {
            if (card && card.rank == rank) count++;
        }
        return count;
    }

    protected bool ContainsInt(array<int> values, int value)
    {
        if (!values) return false;
        foreach (int current : values)
        {
            if (current == value) return true;
        }
        return false;
    }

    protected void AddDealerMemory(OEGoFishSession session, int rank)
    {
        if (!session || ContainsInt(session.dealerMemoryRanks, rank)) return;
        session.dealerMemoryRanks.Insert(rank);
    }

    protected void RemoveDealerMemory(OEGoFishSession session, int rank)
    {
        if (!session || !session.dealerMemoryRanks) return;
        for (int i = session.dealerMemoryRanks.Count() - 1; i >= 0; i--)
        {
            if (session.dealerMemoryRanks.Get(i) == rank)
            {
                session.dealerMemoryRanks.Remove(i);
                return;
            }
        }
    }

    protected string RankName(int rank)
    {
        if (rank == 11) return "JACK";
        if (rank == 12) return "QUEEN";
        if (rank == 13) return "KING";
        if (rank == 14) return "ACE";
        return rank.ToString();
    }

    protected string RankShort(int rank)
    {
        if (rank == 11) return "J";
        if (rank == 12) return "Q";
        if (rank == 13) return "K";
        if (rank == 14) return "A";
        return rank.ToString();
    }

    protected string BuildCardsString(array<ref OEPlayingCard> hand)
    {
        if (!hand) return "";
        string result = "";
        for (int rank = 2; rank <= 14; rank++)
        {
            for (int suit = 0; suit < 4; suit++)
            {
                foreach (OEPlayingCard card : hand)
                {
                    if (!card || card.rank != rank || card.suit != suit) continue;
                    if (result != "") result += ";";
                    result += card.rank.ToString() + "," + card.suit.ToString();
                }
            }
        }
        return result;
    }

    protected string BuildRankCounts(array<ref OEPlayingCard> hand)
    {
        string result = "";
        for (int rank = 2; rank <= 14; rank++)
        {
            if (result != "") result += ",";
            result += CountRank(hand, rank).ToString();
        }
        return result;
    }

    protected string BuildBookText(array<int> books)
    {
        if (!books || books.Count() == 0) return "NONE";
        string result = "";
        foreach (int rank : books)
        {
            if (result != "") result += "  ";
            result += RankShort(rank);
        }
        return result;
    }

    protected OEGoFishSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender) return null;
        OEGoFishSession session;
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
        if (!m_Config || !player) return null;
        OECasinoStation station = m_Config.GetStationById(stationId);
        if (!station) return null;
        if (vector.Distance(player.GetPosition(), station.position) > station.playDistance) return null;
        return station;
    }

    protected void SendIdleState(PlayerBase player, string stationId)
    {
        OEGoFishNetState state = new OEGoFishNetState();
        state.status = OE_GOFISH_STATUS_IDLE;
        state.stationId = stationId;
        state.balance = m_Currency.GetBalance(player);
        state.bet = 0;
        state.playerBooks = 0;
        state.dealerBooks = 0;
        state.pondCount = 52;
        state.playerHandCount = 0;
        state.dealerHandCount = 0;
        state.playerCards = "";
        state.playerRankCounts = "0,0,0,0,0,0,0,0,0,0,0,0,0";
        state.playerBookText = "NONE";
        state.dealerBookText = "NONE";
        state.message = "Set your wager and deal a new Go Fish game.";
        state.animateCards = false;
        SendNetState(player, state);
    }

    protected void SendState(PlayerBase player, OEGoFishSession session, int status, string text, bool animate)
    {
        OEGoFishNetState state = new OEGoFishNetState();
        state.status = status;
        state.stationId = session.stationId;
        state.balance = m_Currency.GetBalance(player);
        state.bet = session.bet;
        state.playerBooks = session.playerBookRanks.Count();
        state.dealerBooks = session.dealerBookRanks.Count();
        state.pondCount = session.deck.Count();
        state.playerHandCount = session.playerHand.Count();
        state.dealerHandCount = session.dealerHand.Count();
        state.playerCards = BuildCardsString(session.playerHand);
        state.playerRankCounts = BuildRankCounts(session.playerHand);
        state.playerBookText = BuildBookText(session.playerBookRanks);
        state.dealerBookText = BuildBookText(session.dealerBookRanks);
        state.message = text;
        state.animateCards = animate;
        SendNetState(player, state);
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OEGoFishNetState state = new OEGoFishNetState();
        state.status = OE_GOFISH_STATUS_ERROR;
        state.stationId = stationId;
        state.balance = m_Currency.GetBalance(player);
        state.playerRankCounts = "0,0,0,0,0,0,0,0,0,0,0,0,0";
        state.message = text;
        SendNetState(player, state);
    }

    protected void SendNetState(PlayerBase player, OEGoFishNetState state)
    {
        if (!player || !player.GetIdentity()) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_GOFISH_STATE, new Param1<ref OEGoFishNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OEGoFishSession session, int payout, string outcome)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        Print("[OperationExileCasino][GoFish] Player=" + player.GetIdentity().GetId() + " Station=" + session.stationId + " Wager=" + session.bet.ToString() + " Outcome=" + outcome + " Payout=" + payout.ToString());
    }
}
