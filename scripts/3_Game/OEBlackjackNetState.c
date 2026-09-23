class OEBlackjackNetState
{
    int status;
    int bet;
    int balance;
    int playerTotal;
    int dealerTotal;
    bool canHit;
    bool canStand;
    bool canDouble;
    bool canSplit;
    int dealerHiddenCards;
    string stationId;
    string playerCards;
    string dealerCards;
    string message;

    bool splitActive;
    int activeSplitHand;
    int splitHandBet;
    int splitHand1Total;
    int splitHand2Total;
    string splitHand1Cards;
    string splitHand2Cards;
    string splitHand1Result;
    string splitHand2Result;
    int splitNetResult;

    void OEBlackjackNetState()
    {
        status = OE_BJ_STATUS_IDLE;
        bet = 0;
        balance = 0;
        playerTotal = 0;
        dealerTotal = 0;
        canHit = false;
        canStand = false;
        canDouble = false;
        canSplit = false;
        dealerHiddenCards = 0;
        stationId = "";
        playerCards = "";
        dealerCards = "";
        message = "";

        splitActive = false;
        activeSplitHand = 0;
        splitHandBet = 0;
        splitHand1Total = 0;
        splitHand2Total = 0;
        splitHand1Cards = "";
        splitHand2Cards = "";
        splitHand1Result = "";
        splitHand2Result = "";
        splitNetResult = 0;
    }
}
