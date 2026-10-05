class OERouletteServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;

    void OERouletteServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
    }

    void ApplyConfig(OECasinoConfig config)
    {
        if (!config) return;
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
    }

    void HandleSync(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_ROULETTE || !m_Config.roulette || m_Config.roulette.enabled != 1)
        {
            SendError(player, stationId, "This Roulette table is not available.");
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleSpin(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<ref OERouletteSpinRequest> reqParam;
        if (!ctx.Read(reqParam)) return;

        OERouletteSpinRequest req = reqParam.param1;
        if (!req) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.stationId;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_ROULETTE || !m_Config.roulette || m_Config.roulette.enabled != 1)
        {
            SendError(player, stationId, "This Roulette table is not available.");
            return;
        }

        string validationError = ValidateRequest(req);
        if (validationError != "")
        {
            SendError(player, stationId, validationError);
            return;
        }

        int betCount = req.betTypes.Count();
        int totalBet = 0;
        int i;
        for (i = 0; i < betCount; i++)
            totalBet += req.amounts.Get(i);

        if (totalBet > m_Config.roulette.maxTotalBet)
        {
            SendError(player, stationId, "Total Roulette bet exceeds the table limit of " + m_Config.roulette.maxTotalBet.ToString() + ".");
            return;
        }

        if (!m_Currency.TryDebit(player, totalBet))
        {
            SendError(player, stationId, "Not enough casino currency for all placed bets.");
            return;
        }

        // Server-authoritative American double-zero result.
        // 0 = 0, 1..36 = numbered pockets, 37 = 00.
        int winningNumber = Math.RandomInt(0, 38);
        string winningColor = GetNumberColor(winningNumber);

        int totalReturn = 0;
        int winningBets = 0;
        string winnersText = "";
        int winnersShown = 0;

        for (i = 0; i < betCount; i++)
        {
            int betType = req.betTypes.Get(i);
            int amount = req.amounts.Get(i);
            int a = req.a.Get(i);
            int b = req.b.Get(i);
            int c = req.c.Get(i);
            int d = req.d.Get(i);
            int e = req.e.Get(i);
            int f = req.f.Get(i);

            if (DidBetWin(betType, a, b, c, d, e, f, winningNumber))
            {
                float multiplier = GetPayoutMultiplier(betType);
                int betReturn = Math.Floor(amount * multiplier);
                if (betReturn < amount) betReturn = amount;
                totalReturn += betReturn;
                winningBets++;

                if (winnersShown < 3)
                {
                    if (winnersText != "") winnersText += " | ";
                    winnersText += GetBetLabel(betType, a, b, c, d, e, f) + " +" + betReturn.ToString();
                    winnersShown++;
                }
            }
        }

        if (totalReturn > 0)
        {
            if (!m_Currency.Credit(player, totalReturn))
            {
                // Re-credit the complete wager if payout creation fails.
                m_Currency.Credit(player, totalBet);
                SendError(player, stationId, "Roulette payout could not be created. Contact an admin.");
                return;
            }
        }

        int netResult = totalReturn - totalBet;
        int status = OE_ROULETTE_STATUS_LOSS;
        if (netResult > 0) status = OE_ROULETTE_STATUS_WIN;
        if (netResult == 0) status = OE_ROULETTE_STATUS_PUSH;

        string settlementSummary = "NO WINNING BETS";
        if (winningBets > 0)
        {
            settlementSummary = winnersText;
            if (winningBets > winnersShown)
                settlementSummary += " | +" + (winningBets - winnersShown).ToString() + " MORE";
        }

        string resultLabel = FormatPocket(winningNumber) + " " + winningColor;
        string message = "Ball landed on " + resultLabel + ". Return " + totalReturn.ToString() + ".";

        SendResultState(player, stationId, totalBet, betCount, winningNumber, winningColor, totalReturn, netResult, status, message, settlementSummary);
        LogResult(player, stationId, totalBet, betCount, winningNumber, winningColor, totalReturn, netResult, settlementSummary);
    }

    protected string ValidateRequest(OERouletteSpinRequest req)
    {
        if (!req.betTypes || !req.amounts || !req.a || !req.b || !req.c || !req.d || !req.e || !req.f)
            return "Roulette bet data is incomplete.";

        int count = req.betTypes.Count();
        if (count < 1) return "Place at least one Roulette bet before spinning.";
        if (count > OE_ROULETTE_MAX_BETS_PER_SPIN) return "Too many Roulette bets are on the table.";

        if (req.amounts.Count() != count || req.a.Count() != count || req.b.Count() != count || req.c.Count() != count || req.d.Count() != count || req.e.Count() != count || req.f.Count() != count)
            return "Roulette bet data is invalid.";

        int i;
        int totalBet = 0;
        for (i = 0; i < count; i++)
        {
            int amount = req.amounts.Get(i);
            if (amount < m_Config.roulette.minBet || amount > m_Config.roulette.maxTotalBet)
                return "A Roulette position has an invalid wager amount.";

            totalBet += amount;
            if (totalBet > m_Config.roulette.maxTotalBet)
                return "Total Roulette bet exceeds the table limit of " + m_Config.roulette.maxTotalBet.ToString() + ".";

            if (!IsValidBet(req.betTypes.Get(i), req.a.Get(i), req.b.Get(i), req.c.Get(i), req.d.Get(i), req.e.Get(i), req.f.Get(i)))
                return "One of the Roulette bets is not valid for this table.";
        }

        return "";
    }

    protected bool IsValidBet(int betType, int a, int b, int c, int d, int e, int f)
    {
        if (betType == OE_ROULETTE_BET_STRAIGHT)
            return a >= 0 && a <= 37;

        if (betType == OE_ROULETTE_BET_RED || betType == OE_ROULETTE_BET_BLACK || betType == OE_ROULETTE_BET_ODD || betType == OE_ROULETTE_BET_EVEN || betType == OE_ROULETTE_BET_LOW || betType == OE_ROULETTE_BET_HIGH)
            return true;

        if (betType == OE_ROULETTE_BET_DOZEN_1 || betType == OE_ROULETTE_BET_DOZEN_2 || betType == OE_ROULETTE_BET_DOZEN_3 || betType == OE_ROULETTE_BET_COLUMN_1 || betType == OE_ROULETTE_BET_COLUMN_2 || betType == OE_ROULETTE_BET_COLUMN_3)
            return true;

        if (betType == OE_ROULETTE_BET_SPLIT)
        {
            if (!IsRegularNumber(a) || !IsRegularNumber(b) || a == b) return false;
            int streetA = GetStreetIndex(a);
            int streetB = GetStreetIndex(b);
            int rowA = GetRowIndex(a);
            int rowB = GetRowIndex(b);
            int streetDiff = AbsInt(streetA - streetB);
            int rowDiff = AbsInt(rowA - rowB);
            if (streetA == streetB && rowDiff == 1) return true;
            if (rowA == rowB && streetDiff == 1) return true;
            return false;
        }

        if (betType == OE_ROULETTE_BET_STREET)
        {
            if (!IsRegularNumber(a) || !IsRegularNumber(b) || !IsRegularNumber(c)) return false;
            if ((a % 3) != 1) return false;
            return b == a + 1 && c == a + 2;
        }

        if (betType == OE_ROULETTE_BET_CORNER)
        {
            if (!IsRegularNumber(a) || !IsRegularNumber(b) || !IsRegularNumber(c) || !IsRegularNumber(d)) return false;
            if ((a % 3) == 0) return false;
            return b == a + 1 && c == a + 3 && d == a + 4 && d <= 36;
        }

        if (betType == OE_ROULETTE_BET_SIX_LINE)
        {
            if (!IsRegularNumber(a) || !IsRegularNumber(b) || !IsRegularNumber(c) || !IsRegularNumber(d) || !IsRegularNumber(e) || !IsRegularNumber(f)) return false;
            if ((a % 3) != 1) return false;
            return b == a + 1 && c == a + 2 && d == a + 3 && e == a + 4 && f == a + 5 && f <= 36;
        }

        if (betType == OE_ROULETTE_BET_TOP_LINE)
            return a == 0 && b == 37 && c == 1 && d == 2 && e == 3;

        return false;
    }

    protected bool DidBetWin(int betType, int a, int b, int c, int d, int e, int f, int winningNumber)
    {
        if (betType == OE_ROULETTE_BET_STRAIGHT) return winningNumber == a;
        if (betType == OE_ROULETTE_BET_SPLIT) return winningNumber == a || winningNumber == b;
        if (betType == OE_ROULETTE_BET_STREET) return winningNumber == a || winningNumber == b || winningNumber == c;
        if (betType == OE_ROULETTE_BET_CORNER) return winningNumber == a || winningNumber == b || winningNumber == c || winningNumber == d;
        if (betType == OE_ROULETTE_BET_SIX_LINE) return winningNumber == a || winningNumber == b || winningNumber == c || winningNumber == d || winningNumber == e || winningNumber == f;
        if (betType == OE_ROULETTE_BET_TOP_LINE) return winningNumber == 0 || winningNumber == 37 || winningNumber == 1 || winningNumber == 2 || winningNumber == 3;

        // Both green pockets lose the standard outside bets.
        if (winningNumber == 0 || winningNumber == 37) return false;

        if (betType == OE_ROULETTE_BET_RED) return IsRed(winningNumber);
        if (betType == OE_ROULETTE_BET_BLACK) return !IsRed(winningNumber);
        if (betType == OE_ROULETTE_BET_ODD) return (winningNumber % 2) == 1;
        if (betType == OE_ROULETTE_BET_EVEN) return (winningNumber % 2) == 0;
        if (betType == OE_ROULETTE_BET_LOW) return winningNumber >= 1 && winningNumber <= 18;
        if (betType == OE_ROULETTE_BET_HIGH) return winningNumber >= 19 && winningNumber <= 36;
        if (betType == OE_ROULETTE_BET_DOZEN_1) return winningNumber >= 1 && winningNumber <= 12;
        if (betType == OE_ROULETTE_BET_DOZEN_2) return winningNumber >= 13 && winningNumber <= 24;
        if (betType == OE_ROULETTE_BET_DOZEN_3) return winningNumber >= 25 && winningNumber <= 36;
        if (betType == OE_ROULETTE_BET_COLUMN_1) return (winningNumber % 3) == 1;
        if (betType == OE_ROULETTE_BET_COLUMN_2) return (winningNumber % 3) == 2;
        if (betType == OE_ROULETTE_BET_COLUMN_3) return (winningNumber % 3) == 0;
        return false;
    }

    protected float GetPayoutMultiplier(int betType)
    {
        if (betType == OE_ROULETTE_BET_STRAIGHT) return m_Config.roulette.straightPayout;
        if (betType == OE_ROULETTE_BET_SPLIT) return m_Config.roulette.splitPayout;
        if (betType == OE_ROULETTE_BET_STREET) return m_Config.roulette.streetPayout;
        if (betType == OE_ROULETTE_BET_CORNER) return m_Config.roulette.cornerPayout;
        if (betType == OE_ROULETTE_BET_TOP_LINE) return m_Config.roulette.topLinePayout;
        if (betType == OE_ROULETTE_BET_SIX_LINE) return m_Config.roulette.sixLinePayout;
        if (betType == OE_ROULETTE_BET_DOZEN_1 || betType == OE_ROULETTE_BET_DOZEN_2 || betType == OE_ROULETTE_BET_DOZEN_3) return m_Config.roulette.dozenColumnPayout;
        if (betType == OE_ROULETTE_BET_COLUMN_1 || betType == OE_ROULETTE_BET_COLUMN_2 || betType == OE_ROULETTE_BET_COLUMN_3) return m_Config.roulette.dozenColumnPayout;
        return m_Config.roulette.evenMoneyPayout;
    }

    protected string GetBetLabel(int betType, int a, int b, int c, int d, int e, int f)
    {
        if (betType == OE_ROULETTE_BET_STRAIGHT) return "NUMBER " + FormatPocket(a);
        if (betType == OE_ROULETTE_BET_SPLIT) return "SPLIT " + FormatPocket(a) + "/" + FormatPocket(b);
        if (betType == OE_ROULETTE_BET_STREET) return "STREET " + a.ToString() + "-" + c.ToString();
        if (betType == OE_ROULETTE_BET_CORNER) return "CORNER " + a.ToString() + "/" + b.ToString() + "/" + c.ToString() + "/" + d.ToString();
        if (betType == OE_ROULETTE_BET_SIX_LINE) return "SIX LINE " + a.ToString() + "-" + f.ToString();
        if (betType == OE_ROULETTE_BET_TOP_LINE) return "TOP LINE";
        if (betType == OE_ROULETTE_BET_RED) return "RED";
        if (betType == OE_ROULETTE_BET_BLACK) return "BLACK";
        if (betType == OE_ROULETTE_BET_ODD) return "ODD";
        if (betType == OE_ROULETTE_BET_EVEN) return "EVEN";
        if (betType == OE_ROULETTE_BET_LOW) return "1-18";
        if (betType == OE_ROULETTE_BET_HIGH) return "19-36";
        if (betType == OE_ROULETTE_BET_DOZEN_1) return "1ST 12";
        if (betType == OE_ROULETTE_BET_DOZEN_2) return "2ND 12";
        if (betType == OE_ROULETTE_BET_DOZEN_3) return "3RD 12";
        if (betType == OE_ROULETTE_BET_COLUMN_1) return "COLUMN 1";
        if (betType == OE_ROULETTE_BET_COLUMN_2) return "COLUMN 2";
        if (betType == OE_ROULETTE_BET_COLUMN_3) return "COLUMN 3";
        return "ROULETTE BET";
    }

    protected string FormatPocket(int number)
    {
        if (number == 37) return "00";
        return number.ToString();
    }

    protected bool IsRegularNumber(int number)
    {
        return number >= 1 && number <= 36;
    }

    protected int GetStreetIndex(int number)
    {
        return (number - 1) / 3;
    }

    protected int GetRowIndex(int number)
    {
        return (number - 1) % 3;
    }

    protected int AbsInt(int value)
    {
        if (value < 0) return -value;
        return value;
    }

    protected bool IsRed(int number)
    {
        if (number == 1 || number == 3 || number == 5 || number == 7 || number == 9) return true;
        if (number == 12 || number == 14 || number == 16 || number == 18) return true;
        if (number == 19 || number == 21 || number == 23 || number == 25 || number == 27) return true;
        if (number == 30 || number == 32 || number == 34 || number == 36) return true;
        return false;
    }

    protected string GetNumberColor(int number)
    {
        if (number == 0 || number == 37) return "GREEN";
        if (IsRed(number)) return "RED";
        return "BLACK";
    }

    protected PlayerBase ResolvePlayer(PlayerIdentity sender, Object target)
    {
        PlayerBase player = PlayerBase.Cast(target);
        if (!sender || !player || !player.GetIdentity()) return null;
        if (player.GetIdentity().GetId() != sender.GetId()) return null;
        return player;
    }

    protected OECasinoStation ValidateStation(PlayerBase player, string stationId)
    {
        if (!m_Config) return null;
        OECasinoStation station = m_Config.GetStationById(stationId);
        if (!station || !player) return null;
        if (vector.Distance(player.GetPosition(), station.position) > station.playDistance) return null;
        return station;
    }

    protected void SendIdleState(PlayerBase player, string stationId)
    {
        OERouletteNetState state = new OERouletteNetState();
        state.status = OE_ROULETTE_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = "Choose a chip value and place as many Roulette bets as you want before spinning.";
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_ROULETTE_STATE, new Param1<ref OERouletteNetState>(state), true, player.GetIdentity());
    }

    protected void SendResultState(PlayerBase player, string stationId, int totalBet, int betCount, int winningNumber, string winningColor, int totalReturn, int netResult, int status, string message, string settlementSummary)
    {
        OERouletteNetState state = new OERouletteNetState();
        state.status = status;
        state.bet = totalBet;
        state.betCount = betCount;
        state.balance = m_Currency.GetBalance(player);
        state.winningNumber = winningNumber;
        state.winningColor = winningColor;
        state.payout = totalReturn;
        state.totalReturn = totalReturn;
        state.netResult = netResult;
        state.stationId = stationId;
        state.message = message;
        state.settlementSummary = settlementSummary;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_ROULETTE_STATE, new Param1<ref OERouletteNetState>(state), true, player.GetIdentity());
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OERouletteNetState state = new OERouletteNetState();
        state.status = OE_ROULETTE_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_ROULETTE_STATE, new Param1<ref OERouletteNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, string stationId, int totalBet, int betCount, int winningNumber, string winningColor, int totalReturn, int netResult, string settlementSummary)
    {
        if (!m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;
        FPrintln(file, "ROULETTE | " + player.GetIdentity().GetPlainId() + " | bets=" + betCount.ToString() + " | totalBet=" + totalBet.ToString() + " | result=" + FormatPocket(winningNumber) + " " + winningColor + " | return=" + totalReturn.ToString() + " | net=" + netResult.ToString() + " | winners=" + settlementSummary + " | station=" + stationId);
        CloseFile(file);
    }
}
