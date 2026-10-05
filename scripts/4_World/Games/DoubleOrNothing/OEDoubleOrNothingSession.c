class OEDoubleOrNothingSession
{
    string playerId;
    string stationId;
    int originalBet;
    int currentValue;
    int successfulDoubles;
    int payout;
    int netResult;
    bool finished;
    bool awaitingColour;
    int chosenColour;
    int cardRank;
    int cardSuit;

    void OEDoubleOrNothingSession(string uid, string station, int wager)
    {
        playerId = uid;
        stationId = station;
        originalBet = wager;
        currentValue = wager;
        successfulDoubles = 0;
        payout = 0;
        netResult = 0 - wager;
        finished = false;
        awaitingColour = true;
        chosenColour = OE_DON_CHOICE_NONE;
        cardRank = 0;
        cardSuit = -1;
    }

    void PrepareColourChoice()
    {
        awaitingColour = true;
        chosenColour = OE_DON_CHOICE_NONE;
        cardRank = 0;
        cardSuit = -1;
    }
}
