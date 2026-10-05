class OERouletteNetState
{
    int status;
    int bet;
    int balance;
    int betCount;
    int totalReturn;
    int winningNumber;
    int payout;
    int netResult;
    string winningColor;
    string stationId;
    string message;
    string settlementSummary;

    void OERouletteNetState()
    {
        status = OE_ROULETTE_STATUS_IDLE;
        bet = 0;
        balance = 0;
        betCount = 0;
        totalReturn = 0;
        winningNumber = -1;
        payout = 0;
        netResult = 0;
        winningColor = "";
        stationId = "";
        message = "";
        settlementSummary = "";
    }
}
