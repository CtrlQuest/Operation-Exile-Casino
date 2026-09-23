class OECrapsBetActionRequest
{
    string stationId;
    int action;
    int betType;
    int target;
    int amount;

    void OECrapsBetActionRequest()
    {
        stationId = "";
        action = 0;
        betType = 0;
        target = 0;
        amount = 0;
    }
}

class OECrapsNetState
{
    int status;
    int balance;
    int bet;
    int lineBetType;
    int phase;
    int point;
    int die1;
    int die2;
    int total;
    int payout;
    int netResult;
    int rollNumber;
    int sideBetTotal;
    int lastSideReturn;
    int lastSideNet;
    bool roundActive;
    bool tableActive;

    int oddsBet;
    int comePending;
    int dontComePending;
    int fieldBet;

    int place4;
    int place5;
    int place6;
    int place8;
    int place9;
    int place10;

    int come4;
    int come5;
    int come6;
    int come8;
    int come9;
    int come10;

    int dontCome4;
    int dontCome5;
    int dontCome6;
    int dontCome8;
    int dontCome9;
    int dontCome10;

    string stationId;
    string message;
    string sideSummary;

    void OECrapsNetState()
    {
        status = OE_CRAPS_STATUS_IDLE;
        balance = 0;
        bet = 0;
        lineBetType = OE_CRAPS_BET_PASS_LINE;
        phase = OE_CRAPS_PHASE_COME_OUT;
        point = 0;
        die1 = 0;
        die2 = 0;
        total = 0;
        payout = 0;
        netResult = 0;
        rollNumber = 0;
        sideBetTotal = 0;
        lastSideReturn = 0;
        lastSideNet = 0;
        roundActive = false;
        tableActive = false;

        oddsBet = 0;
        comePending = 0;
        dontComePending = 0;
        fieldBet = 0;

        place4 = 0;
        place5 = 0;
        place6 = 0;
        place8 = 0;
        place9 = 0;
        place10 = 0;

        come4 = 0;
        come5 = 0;
        come6 = 0;
        come8 = 0;
        come9 = 0;
        come10 = 0;

        dontCome4 = 0;
        dontCome5 = 0;
        dontCome6 = 0;
        dontCome8 = 0;
        dontCome9 = 0;
        dontCome10 = 0;

        stationId = "";
        message = "";
        sideSummary = "";
    }
}
