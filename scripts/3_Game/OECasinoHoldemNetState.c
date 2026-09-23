class OECasinoHoldemNetState
{
    int status;
    int street;
    int startingBet;
    int playerContribution;
    int dealerContribution;
    int playerStreetBet;
    int dealerStreetBet;
    int amountToCall;
    int raiseAmount;
    int minRaiseAmount;
    int maxRaiseAmount;
    int pot;
    int balance;
    int payout;
    int netResult;
    int raisesThisStreet;
    int maxRaisesPerStreet;

    bool canCheck;
    bool canCall;
    bool canRaise;
    bool canFold;

    int dealerHiddenCards;
    int communityHiddenCards;
    string stationId;
    string playerCards;
    string dealerCards;
    string communityCards;
    string playerHandName;
    string dealerHandName;
    string dealerLastAction;
    string message;

    void OECasinoHoldemNetState()
    {
        status = OE_HOLDEM_STATUS_IDLE;
        street = OE_HOLDEM_STREET_PREFLOP;
        startingBet = 0;
        playerContribution = 0;
        dealerContribution = 0;
        playerStreetBet = 0;
        dealerStreetBet = 0;
        amountToCall = 0;
        raiseAmount = 0;
        minRaiseAmount = 0;
        maxRaiseAmount = 0;
        pot = 0;
        balance = 0;
        payout = 0;
        netResult = 0;
        raisesThisStreet = 0;
        maxRaisesPerStreet = 0;

        canCheck = false;
        canCall = false;
        canRaise = false;
        canFold = false;

        dealerHiddenCards = 0;
        communityHiddenCards = 0;
        stationId = "";
        playerCards = "";
        dealerCards = "";
        communityCards = "";
        playerHandName = "";
        dealerHandName = "";
        dealerLastAction = "";
        message = "";
    }
}
