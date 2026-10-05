class OERideBusSession
{
    string playerId;
    string stationId;
    int bet;
    int round;
    int cashoutValue;
    int decisionDeadlineMs;
    bool finished;

    ref OECardDeck deck;
    ref OEPlayingCard pendingCard;
    ref array<ref OEPlayingCard> cards;

    void OERideBusSession(string uid, string station, int wager)
    {
        playerId = uid;
        stationId = station;
        bet = wager;
        round = OE_RTB_ROUND_COLOUR;

        // The player may get off the bus before making the first call and
        // recover the original stake. Correct rounds increase this value.
        cashoutValue = wager;
        decisionDeadlineMs = 0;
        finished = false;
        deck = new OECardDeck();
        pendingCard = null;
        cards = new array<ref OEPlayingCard>;
    }
}
