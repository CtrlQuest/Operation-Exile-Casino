class OECrapsSession
{
    string playerId;
    string stationId;

    int originalBet;
    int lineBetType;
    int phase;
    int point;
    int rollNumber;
    int lastDie1;
    int lastDie2;
    int lastTotal;
    int payout;
    int netResult;
    bool lineBetActive;
    bool finished;

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

    int lastActionType;
    int lastActionTarget;
    int lastActionAmount;

    int lastSideReturn;
    int lastSideNet;
    string lastSideSummary;

    void OECrapsSession(string uid, string station, int wager, int betType)
    {
        playerId = uid;
        stationId = station;

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

        lastActionType = 0;
        lastActionTarget = 0;
        lastActionAmount = 0;
        lastSideReturn = 0;
        lastSideNet = 0;
        lastSideSummary = "";
        finished = false;

        BeginLine(wager, betType);
    }

    void BeginLine(int wager, int betType)
    {
        originalBet = wager;
        lineBetType = betType;
        phase = OE_CRAPS_PHASE_COME_OUT;
        point = 0;
        rollNumber = 0;
        lastDie1 = 0;
        lastDie2 = 0;
        lastTotal = 0;
        payout = 0;
        netResult = 0 - wager;
        lineBetActive = true;
        finished = false;
        ResetLastAction();
        ResetLastSideResult();
    }

    void ResetLastAction()
    {
        lastActionType = 0;
        lastActionTarget = 0;
        lastActionAmount = 0;
    }

    void ResetLastSideResult()
    {
        lastSideReturn = 0;
        lastSideNet = 0;
        lastSideSummary = "";
    }

    int GetPlaceBet(int number)
    {
        if (number == 4) return place4;
        if (number == 5) return place5;
        if (number == 6) return place6;
        if (number == 8) return place8;
        if (number == 9) return place9;
        if (number == 10) return place10;
        return 0;
    }

    void AddPlaceBet(int number, int amount)
    {
        if (number == 4) place4 += amount;
        else if (number == 5) place5 += amount;
        else if (number == 6) place6 += amount;
        else if (number == 8) place8 += amount;
        else if (number == 9) place9 += amount;
        else if (number == 10) place10 += amount;
    }

    void RemovePlaceBet(int number, int amount)
    {
        if (number == 4) { place4 -= amount; if (place4 < 0) place4 = 0; }
        else if (number == 5) { place5 -= amount; if (place5 < 0) place5 = 0; }
        else if (number == 6) { place6 -= amount; if (place6 < 0) place6 = 0; }
        else if (number == 8) { place8 -= amount; if (place8 < 0) place8 = 0; }
        else if (number == 9) { place9 -= amount; if (place9 < 0) place9 = 0; }
        else if (number == 10) { place10 -= amount; if (place10 < 0) place10 = 0; }
    }

    int GetComePoint(int number)
    {
        if (number == 4) return come4;
        if (number == 5) return come5;
        if (number == 6) return come6;
        if (number == 8) return come8;
        if (number == 9) return come9;
        if (number == 10) return come10;
        return 0;
    }

    void AddComePoint(int number, int amount)
    {
        if (number == 4) come4 += amount;
        else if (number == 5) come5 += amount;
        else if (number == 6) come6 += amount;
        else if (number == 8) come8 += amount;
        else if (number == 9) come9 += amount;
        else if (number == 10) come10 += amount;
    }

    void ClearComePoint(int number)
    {
        if (number == 4) come4 = 0;
        else if (number == 5) come5 = 0;
        else if (number == 6) come6 = 0;
        else if (number == 8) come8 = 0;
        else if (number == 9) come9 = 0;
        else if (number == 10) come10 = 0;
    }

    int GetDontComePoint(int number)
    {
        if (number == 4) return dontCome4;
        if (number == 5) return dontCome5;
        if (number == 6) return dontCome6;
        if (number == 8) return dontCome8;
        if (number == 9) return dontCome9;
        if (number == 10) return dontCome10;
        return 0;
    }

    void AddDontComePoint(int number, int amount)
    {
        if (number == 4) dontCome4 += amount;
        else if (number == 5) dontCome5 += amount;
        else if (number == 6) dontCome6 += amount;
        else if (number == 8) dontCome8 += amount;
        else if (number == 9) dontCome9 += amount;
        else if (number == 10) dontCome10 += amount;
    }

    void ClearDontComePoint(int number)
    {
        if (number == 4) dontCome4 = 0;
        else if (number == 5) dontCome5 = 0;
        else if (number == 6) dontCome6 = 0;
        else if (number == 8) dontCome8 = 0;
        else if (number == 9) dontCome9 = 0;
        else if (number == 10) dontCome10 = 0;
    }

    int GetPlaceTotal()
    {
        return place4 + place5 + place6 + place8 + place9 + place10;
    }

    int GetComePointTotal()
    {
        return come4 + come5 + come6 + come8 + come9 + come10;
    }

    int GetDontComePointTotal()
    {
        return dontCome4 + dontCome5 + dontCome6 + dontCome8 + dontCome9 + dontCome10;
    }

    int GetSideBetTotal()
    {
        return oddsBet + comePending + dontComePending + fieldBet + GetPlaceTotal() + GetComePointTotal() + GetDontComePointTotal();
    }

    bool HasSideBets()
    {
        return GetSideBetTotal() > 0;
    }
}
