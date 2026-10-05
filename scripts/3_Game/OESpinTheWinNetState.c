class OESpinTheWinNetState
{
    int status = OE_SPIN_STATUS_IDLE;
    int bet = 0;
    int balance = 0;
    int segmentIndex = 0;
    int prizeType = OE_SPIN_PRIZE_LOSE;
    int payout = 0;
    int netResult = 0;
    int jackpot = 0;
    bool animate = false;
    string stationId = "";
    string prizeLabel = "";
    string message = "";
}
