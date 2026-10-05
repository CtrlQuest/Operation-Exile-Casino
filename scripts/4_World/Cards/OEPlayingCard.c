class OEPlayingCard
{
    int rank; // 2..14 (11 J, 12 Q, 13 K, 14 A)
    int suit;

    void OEPlayingCard(int cardRank = 2, int cardSuit = OE_CARD_SUIT_HEARTS)
    {
        rank = cardRank;
        suit = cardSuit;
    }

    bool IsRed()
    {
        return suit == OE_CARD_SUIT_HEARTS || suit == OE_CARD_SUIT_DIAMONDS;
    }

    string GetRankName()
    {
        if (rank == 11) return "JACK";
        if (rank == 12) return "QUEEN";
        if (rank == 13) return "KING";
        if (rank == 14) return "ACE";
        return rank.ToString();
    }

    string GetSuitName()
    {
        if (suit == OE_CARD_SUIT_HEARTS) return "HEARTS";
        if (suit == OE_CARD_SUIT_DIAMONDS) return "DIAMONDS";
        if (suit == OE_CARD_SUIT_CLUBS) return "CLUBS";
        return "SPADES";
    }

    string ToDisplayString()
    {
        return GetRankName() + " OF " + GetSuitName();
    }
}
