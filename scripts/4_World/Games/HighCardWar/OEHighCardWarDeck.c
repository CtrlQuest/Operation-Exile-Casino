class OEHighCardWarDeck
{
    protected ref array<ref OEPlayingCard> m_Cards;

    void OEHighCardWarDeck(int deckCount = 6)
    {
        Reset(deckCount);
    }

    void Reset(int deckCount = 6)
    {
        if (deckCount < 1) deckCount = 1;
        if (deckCount > 8) deckCount = 8;

        m_Cards = new array<ref OEPlayingCard>;
        for (int deck = 0; deck < deckCount; deck++)
        {
            for (int suit = 0; suit < 4; suit++)
            {
                for (int rank = 2; rank <= 14; rank++)
                    m_Cards.Insert(new OEPlayingCard(rank, suit));
            }
        }
    }

    OEPlayingCard Draw()
    {
        if (!m_Cards || m_Cards.Count() == 0) return null;
        int index = Math.RandomInt(0, m_Cards.Count());
        OEPlayingCard card = m_Cards.Get(index);
        m_Cards.Remove(index);
        return card;
    }
}
