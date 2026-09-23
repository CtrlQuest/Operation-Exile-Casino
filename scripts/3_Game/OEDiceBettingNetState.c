class OEDiceBettingNetState
{
    int status;
    int bet;
    int balance;
    int betType;
    int exactTarget;
    int die1;
    int die2;
    int total;
    int payout;
    int netResult;
    string stationId;
    string message;

    void OEDiceBettingNetState()
    {
        status = OE_DICE_STATUS_IDLE;
        bet = 0;
        balance = 0;
        betType = OE_DICE_BET_LOW;
        exactTarget = 7;
        die1 = 0;
        die2 = 0;
        total = 0;
        payout = 0;
        netResult = 0;
        stationId = "";
        message = "";
    }
}
