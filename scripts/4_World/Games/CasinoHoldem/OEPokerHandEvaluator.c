class OEPokerHandResult
{
    int category;
    int score;
    int primaryRank;
    string name;

    void OEPokerHandResult()
    {
        category = OE_POKER_HAND_HIGH_CARD;
        score = 0;
        primaryRank = 0;
        name = "HIGH CARD";
    }
}

class OEPokerHandEvaluator
{
    static OEPokerHandResult EvaluateBest(array<ref OEPlayingCard> cards)
    {
        OEPokerHandResult best = new OEPokerHandResult();
        if (!cards || cards.Count() < 5) return best;

        int n = cards.Count();
        for (int a = 0; a < n - 4; a++)
        {
            for (int b = a + 1; b < n - 3; b++)
            {
                for (int c = b + 1; c < n - 2; c++)
                {
                    for (int d = c + 1; d < n - 1; d++)
                    {
                        for (int e = d + 1; e < n; e++)
                        {
                            array<ref OEPlayingCard> five = new array<ref OEPlayingCard>;
                            five.Insert(cards.Get(a));
                            five.Insert(cards.Get(b));
                            five.Insert(cards.Get(c));
                            five.Insert(cards.Get(d));
                            five.Insert(cards.Get(e));

                            OEPokerHandResult current = EvaluateFive(five);
                            if (current && current.score > best.score)
                                best = current;
                        }
                    }
                }
            }
        }

        return best;
    }

    static OEPokerHandResult EvaluateFive(array<ref OEPlayingCard> cards)
    {
        OEPokerHandResult result = new OEPokerHandResult();
        if (!cards || cards.Count() != 5) return result;

        array<int> rankCounts = new array<int>;
        for (int r = 0; r <= 14; r++)
        {
            rankCounts.Insert(0);
        }

        array<int> ranks = new array<int>;
        bool flush = true;
        int firstSuit = -1;

        foreach (OEPlayingCard card : cards)
        {
            if (!card) continue;
            int rank = card.rank;
            if (rank < 2 || rank > 14) continue;

            rankCounts.Set(rank, rankCounts.Get(rank) + 1);
            ranks.Insert(rank);

            if (firstSuit < 0) firstSuit = card.suit;
            else if (card.suit != firstSuit) flush = false;
        }

        SortDescending(ranks);
        int straightHigh = GetStraightHigh(rankCounts);

        int fourRank = 0;
        array<int> trips = new array<int>;
        array<int> pairs = new array<int>;
        array<int> singles = new array<int>;

        for (int rankScan = 14; rankScan >= 2; rankScan--)
        {
            int count = rankCounts.Get(rankScan);
            if (count == 4) fourRank = rankScan;
            else if (count == 3) trips.Insert(rankScan);
            else if (count == 2) pairs.Insert(rankScan);
            else if (count == 1) singles.Insert(rankScan);
        }

        if (flush && straightHigh > 0)
        {
            if (straightHigh == 14)
            {
                SetResult(result, OE_POKER_HAND_ROYAL_FLUSH, 14, 0, 0, 0, 0, "ROYAL FLUSH");
                return result;
            }

            SetResult(result, OE_POKER_HAND_STRAIGHT_FLUSH, straightHigh, 0, 0, 0, 0, "STRAIGHT FLUSH");
            return result;
        }

        if (fourRank > 0)
        {
            int fourKicker = SafeGet(singles, 0);
            SetResult(result, OE_POKER_HAND_FOUR_KIND, fourRank, fourKicker, 0, 0, 0, "FOUR OF A KIND");
            return result;
        }

        if (trips.Count() > 0 && (pairs.Count() > 0 || trips.Count() > 1))
        {
            int fullTrip = trips.Get(0);
            int fullPair = 0;
            if (trips.Count() > 1) fullPair = trips.Get(1);
            if (pairs.Count() > 0 && pairs.Get(0) > fullPair) fullPair = pairs.Get(0);
            SetResult(result, OE_POKER_HAND_FULL_HOUSE, fullTrip, fullPair, 0, 0, 0, "FULL HOUSE");
            return result;
        }

        if (flush)
        {
            SetResult(result, OE_POKER_HAND_FLUSH, SafeGet(ranks, 0), SafeGet(ranks, 1), SafeGet(ranks, 2), SafeGet(ranks, 3), SafeGet(ranks, 4), "FLUSH");
            return result;
        }

        if (straightHigh > 0)
        {
            SetResult(result, OE_POKER_HAND_STRAIGHT, straightHigh, 0, 0, 0, 0, "STRAIGHT");
            return result;
        }

        if (trips.Count() > 0)
        {
            SetResult(result, OE_POKER_HAND_THREE_KIND, trips.Get(0), SafeGet(singles, 0), SafeGet(singles, 1), 0, 0, "THREE OF A KIND");
            return result;
        }

        if (pairs.Count() >= 2)
        {
            SetResult(result, OE_POKER_HAND_TWO_PAIR, pairs.Get(0), pairs.Get(1), SafeGet(singles, 0), 0, 0, "TWO PAIR");
            return result;
        }

        if (pairs.Count() == 1)
        {
            SetResult(result, OE_POKER_HAND_PAIR, pairs.Get(0), SafeGet(singles, 0), SafeGet(singles, 1), SafeGet(singles, 2), 0, "PAIR");
            return result;
        }

        SetResult(result, OE_POKER_HAND_HIGH_CARD, SafeGet(ranks, 0), SafeGet(ranks, 1), SafeGet(ranks, 2), SafeGet(ranks, 3), SafeGet(ranks, 4), "HIGH CARD");
        return result;
    }

    static void SetResult(OEPokerHandResult result, int category, int a, int b, int c, int d, int e, string handName)
    {
        if (!result) return;
        result.category = category;
        result.primaryRank = a;
        result.score = Encode(category, a, b, c, d, e);
        result.name = handName;
    }

    static int Encode(int category, int a, int b, int c, int d, int e)
    {
        int value = category;
        value = value * 15 + a;
        value = value * 15 + b;
        value = value * 15 + c;
        value = value * 15 + d;
        value = value * 15 + e;
        return value;
    }

    static int GetStraightHigh(array<int> rankCounts)
    {
        if (!rankCounts || rankCounts.Count() < 15) return 0;

        for (int high = 14; high >= 6; high--)
        {
            if (rankCounts.Get(high) > 0 && rankCounts.Get(high - 1) > 0 && rankCounts.Get(high - 2) > 0 && rankCounts.Get(high - 3) > 0 && rankCounts.Get(high - 4) > 0)
                return high;
        }

        // Wheel straight: A-2-3-4-5.
        if (rankCounts.Get(14) > 0 && rankCounts.Get(5) > 0 && rankCounts.Get(4) > 0 && rankCounts.Get(3) > 0 && rankCounts.Get(2) > 0)
            return 5;

        return 0;
    }

    static void SortDescending(array<int> values)
    {
        if (!values) return;
        for (int i = 0; i < values.Count(); i++)
        {
            for (int j = i + 1; j < values.Count(); j++)
            {
                if (values.Get(j) > values.Get(i))
                {
                    int temp = values.Get(i);
                    values.Set(i, values.Get(j));
                    values.Set(j, temp);
                }
            }
        }
    }

    static int SafeGet(array<int> values, int index)
    {
        if (!values || index < 0 || index >= values.Count()) return 0;
        return values.Get(index);
    }
}
