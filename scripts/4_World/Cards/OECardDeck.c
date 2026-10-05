class OECardDeck
{
    protected ref array<ref OEPlayingCard> m_Cards;

    void OECardDeck()
    {
        Reset();
    }

    void Reset()
    {
        m_Cards = new array<ref OEPlayingCard>;
        for (int suit = 0; suit < 4; suit++)
        {
            for (int rank = 2; rank <= 14; rank++)
            {
                m_Cards.Insert(new OEPlayingCard(rank, suit));
            }
        }
    }

    int Count()
    {
        if (!m_Cards) return 0;
        return m_Cards.Count();
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
