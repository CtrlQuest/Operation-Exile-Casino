class OEDiceBettingServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;

    void OEDiceBettingServer(OECasinoConfig config)
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
        if (!station || station.game != OE_CASINO_GAME_DICE_BETTING || !m_Config.diceBetting || m_Config.diceBetting.enabled != 1)
        {
            SendError(player, stationId, "This Dice Betting table is not available.");
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleRoll(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param4<string, int, int, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        int bet = req.param2;
        int betType = req.param3;
        int exactTarget = req.param4;

        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_DICE_BETTING || !m_Config.diceBetting || m_Config.diceBetting.enabled != 1)
        {
            SendError(player, stationId, "This Dice Betting table is not available.");
            return;
        }

        if (bet < m_Config.diceBetting.minBet || bet > m_Config.diceBetting.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        if (!IsValidBetType(betType))
        {
            SendError(player, stationId, "Invalid dice bet.");
            return;
        }

        if (betType == OE_DICE_BET_EXACT && (exactTarget < 2 || exactTarget > 12))
        {
            SendError(player, stationId, "Exact total must be between 2 and 12.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        int die1 = Math.RandomInt(1, 7);
        int die2 = Math.RandomInt(1, 7);
        int total = die1 + die2;
        bool won = DidBetWin(betType, exactTarget, die1, die2, total);

        int payout = 0;
        int netResult = -bet;
        string betLabel = GetBetLabel(betType, exactTarget);
        string message = "Rolled " + die1.ToString() + " + " + die2.ToString() + " = " + total.ToString() + ". ";
        int status = OE_DICE_STATUS_LOSS;

        if (won)
        {
            float multiplier = GetPayoutMultiplier(betType, exactTarget);
            payout = Math.Floor(bet * multiplier);
            if (payout < bet) payout = bet;

            if (!m_Currency.Credit(player, payout))
            {
                // Best effort: if payout creation fails, restore the wager so a
                // currency/inventory problem cannot silently eat the player's bet.
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Dice payout could not be created. Contact an admin.");
                return;
            }

            netResult = payout - bet;
            status = OE_DICE_STATUS_WIN;
            message += betLabel + " wins! Paid " + payout.ToString() + " chips.";
        }
        else
        {
            message += betLabel + " loses.";
        }

        SendResultState(player, stationId, bet, betType, exactTarget, die1, die2, total, payout, netResult, status, message);
        LogResult(player, stationId, bet, betType, exactTarget, die1, die2, total, payout);
    }

    protected bool IsValidBetType(int betType)
    {
        if (betType == OE_DICE_BET_LOW) return true;
        if (betType == OE_DICE_BET_HIGH) return true;
        if (betType == OE_DICE_BET_ODD) return true;
        if (betType == OE_DICE_BET_EVEN) return true;
        if (betType == OE_DICE_BET_DOUBLES) return true;
        if (betType == OE_DICE_BET_EXACT) return true;
        return false;
    }

    protected bool DidBetWin(int betType, int exactTarget, int die1, int die2, int total)
    {
        if (betType == OE_DICE_BET_LOW) return total >= 2 && total <= 6;
        if (betType == OE_DICE_BET_HIGH) return total >= 8 && total <= 12;
        if (betType == OE_DICE_BET_ODD) return (total % 2) == 1;
        if (betType == OE_DICE_BET_EVEN) return (total % 2) == 0;
        if (betType == OE_DICE_BET_DOUBLES) return die1 == die2;
        if (betType == OE_DICE_BET_EXACT) return total == exactTarget;
        return false;
    }

    protected float GetPayoutMultiplier(int betType, int exactTarget)
    {
        if (betType == OE_DICE_BET_LOW || betType == OE_DICE_BET_HIGH)
            return m_Config.diceBetting.highLowPayout;
        if (betType == OE_DICE_BET_ODD || betType == OE_DICE_BET_EVEN)
            return m_Config.diceBetting.oddEvenPayout;
        if (betType == OE_DICE_BET_DOUBLES)
            return m_Config.diceBetting.doublesPayout;

        if (betType == OE_DICE_BET_EXACT)
        {
            if (exactTarget == 2 || exactTarget == 12) return m_Config.diceBetting.exact2or12Payout;
            if (exactTarget == 3 || exactTarget == 11) return m_Config.diceBetting.exact3or11Payout;
            if (exactTarget == 4 || exactTarget == 10) return m_Config.diceBetting.exact4or10Payout;
            if (exactTarget == 5 || exactTarget == 9) return m_Config.diceBetting.exact5or9Payout;
            if (exactTarget == 6 || exactTarget == 8) return m_Config.diceBetting.exact6or8Payout;
            if (exactTarget == 7) return m_Config.diceBetting.exact7Payout;
        }

        return 1.0;
    }

    protected string GetBetLabel(int betType, int exactTarget)
    {
        if (betType == OE_DICE_BET_LOW) return "LOW (2-6)";
        if (betType == OE_DICE_BET_HIGH) return "HIGH (8-12)";
        if (betType == OE_DICE_BET_ODD) return "ODD";
        if (betType == OE_DICE_BET_EVEN) return "EVEN";
        if (betType == OE_DICE_BET_DOUBLES) return "DOUBLES";
        if (betType == OE_DICE_BET_EXACT) return "EXACT " + exactTarget.ToString();
        return "DICE BET";
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
        OEDiceBettingNetState state = new OEDiceBettingNetState();
        state.status = OE_DICE_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = "Choose a dice bet, set your wager, then ROLL DICE.";
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DICE_STATE, new Param1<ref OEDiceBettingNetState>(state), true, player.GetIdentity());
    }

    protected void SendResultState(PlayerBase player, string stationId, int bet, int betType, int exactTarget, int die1, int die2, int total, int payout, int netResult, int status, string message)
    {
        OEDiceBettingNetState state = new OEDiceBettingNetState();
        state.status = status;
        state.bet = bet;
        state.balance = m_Currency.GetBalance(player);
        state.betType = betType;
        state.exactTarget = exactTarget;
        state.die1 = die1;
        state.die2 = die2;
        state.total = total;
        state.payout = payout;
        state.netResult = netResult;
        state.stationId = stationId;
        state.message = message;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DICE_STATE, new Param1<ref OEDiceBettingNetState>(state), true, player.GetIdentity());
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OEDiceBettingNetState state = new OEDiceBettingNetState();
        state.status = OE_DICE_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.stationId = stationId;
        state.message = text;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DICE_STATE, new Param1<ref OEDiceBettingNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, string stationId, int bet, int betType, int exactTarget, int die1, int die2, int total, int payout)
    {
        if (!m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;
        FPrintln(file, "DICE_BETTING | " + player.GetIdentity().GetPlainId() + " | bet=" + bet + " | type=" + GetBetLabel(betType, exactTarget) + " | dice=" + die1 + "+" + die2 + " | total=" + total + " | payout=" + payout + " | station=" + stationId);
        CloseFile(file);
    }
}
