class OEDoubleOrNothingNetState
{
    int status;
    int bet;
    int balance;
    int currentValue;
    int nextValue;
    int successfulDoubles;
    int maxDoubles;
    float winChancePercent;
    int payout;
    int netResult;
    bool canDouble;
    bool canCashOut;
    bool awaitingColour;
    int chosenColour;
    int cardRank;
    int cardSuit;
    string stationId;
    string message;

    void OEDoubleOrNothingNetState()
    {
        status = OE_DON_STATUS_IDLE;
        bet = 0;
        balance = 0;
        currentValue = 0;
        nextValue = 0;
        successfulDoubles = 0;
        maxDoubles = 0;
        winChancePercent = 50.0;
        payout = 0;
        netResult = 0;
        canDouble = false;
        canCashOut = false;
        awaitingColour = false;
        chosenColour = OE_DON_CHOICE_NONE;
        cardRank = 0;
        cardSuit = -1;
        stationId = "";
        message = "";
    }
}
