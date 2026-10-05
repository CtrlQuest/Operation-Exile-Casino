class OESpinTheWinServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref OESpinTheWinJackpotData m_JackpotData;
    protected ref map<string, ref OESpinTheWinNetState> m_LastStates;
    protected ref map<string, int> m_NextSpinAt;

    void OESpinTheWinServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_LastStates = new map<string, ref OESpinTheWinNetState>;
        m_NextSpinAt = new map<string, int>;
        LoadJackpot();
    }

    void ApplyConfig(OECasinoConfig config)
    {
        if (!config) return;
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        EnsureJackpotMinimum();
    }

    void HandleSync(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_SPIN_THE_WIN || !m_Config.spinTheWin || m_Config.spinTheWin.enabled != 1)
        {
            SendError(player, stationId, "This Spin the Win station is not available.");
            return;
        }

        string uid = player.GetIdentity().GetId();
        OESpinTheWinNetState previous;
        if (m_LastStates.Find(uid, previous) && previous)
        {
            OESpinTheWinNetState syncState = CopyState(previous, false);
            syncState.balance = m_Currency.GetBalance(player);
            syncState.jackpot = GetJackpot();
            syncState.stationId = stationId;
            SendState(player, syncState);
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleSpin(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param2<string, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        int bet = req.param2;

        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_SPIN_THE_WIN || !m_Config.spinTheWin || m_Config.spinTheWin.enabled != 1)
        {
            SendError(player, stationId, "This Spin the Win station is not available.");
            return;
        }

        if (bet < m_Config.spinTheWin.minBet || bet > m_Config.spinTheWin.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        string uid = player.GetIdentity().GetId();
        int now = GetGame().GetTime();
        int nextAllowed = 0;
        if (m_NextSpinAt.Find(uid, nextAllowed) && now < nextAllowed)
        {
            SendError(player, stationId, "The wheel is still spinning.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        int segmentIndex = Math.RandomInt(0, 16);
        int prizeType = GetPrizeTypeForSegment(segmentIndex);
        string prizeLabel = GetPrizeLabel(prizeType);
        int payout = 0;
        int netResult = -bet;
        bool jackpotWin = false;
        string message = "";

        if (prizeType == OE_SPIN_PRIZE_JACKPOT)
        {
            payout = GetJackpot();
            if (payout < m_Config.spinTheWin.jackpotStart)
                payout = m_Config.spinTheWin.jackpotStart;

            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Jackpot payout could not be created. Contact an admin.");
                return;
            }

            netResult = payout - bet;
            jackpotWin = true;
            SetJackpot(m_Config.spinTheWin.jackpotStart);
            message = "JACKPOT! Paid " + payout.ToString() + " chips.";
        }
        else if (prizeType == OE_SPIN_PRIZE_1X)
        {
            payout = bet;
            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Wheel payout could not be created. Contact an admin.");
                return;
            }
            netResult = 0;
            message = "Landed on 1x. Your stake was returned.";
        }
        else if (prizeType == OE_SPIN_PRIZE_2X)
        {
            payout = bet * 2;
            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Wheel payout could not be created. Contact an admin.");
                return;
            }
            netResult = payout - bet;
            message = "Landed on 2x. Paid " + payout.ToString() + " chips.";
        }
        else if (prizeType == OE_SPIN_PRIZE_5X)
        {
            payout = bet * 5;
            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Wheel payout could not be created. Contact an admin.");
                return;
            }
            netResult = payout - bet;
            message = "Landed on 5x. Paid " + payout.ToString() + " chips.";
        }
        else
        {
            int contribution = Math.Floor(bet * (m_Config.spinTheWin.jackpotContributionPercent / 100.0));
            if (contribution > 0)
                SetJackpot(GetJackpot() + contribution);
            message = "Landed on LOSE. The wager is gone.";
        }

        int cooldownMs = Math.Floor(m_Config.spinTheWin.spinDurationSeconds * 1000.0);
        if (cooldownMs < 1000) cooldownMs = 1000;
        m_NextSpinAt.Set(uid, now + cooldownMs);

        OESpinTheWinNetState storedState = BuildResultState(player, stationId, bet, segmentIndex, prizeType, prizeLabel, payout, netResult, message, false);
        m_LastStates.Set(uid, storedState);

        OESpinTheWinNetState animatedState = CopyState(storedState, true);
        SendState(player, animatedState);
        LogResult(player, stationId, bet, segmentIndex, prizeLabel, payout, netResult, jackpotWin);
    }

    protected int GetPrizeTypeForSegment(int segmentIndex)
    {
        // Artwork order, starting at the 12 o'clock segment and moving clockwise:
        // JACKPOT, LOSE, 1x, LOSE, 2x, LOSE, 1x, LOSE,
        // 5x, LOSE, 1x, LOSE, 2x, LOSE, 1x, 5x.
        if (segmentIndex == 0) return OE_SPIN_PRIZE_JACKPOT;
        if (segmentIndex == 2 || segmentIndex == 6 || segmentIndex == 10 || segmentIndex == 14) return OE_SPIN_PRIZE_1X;
        if (segmentIndex == 4 || segmentIndex == 12) return OE_SPIN_PRIZE_2X;
        if (segmentIndex == 8 || segmentIndex == 15) return OE_SPIN_PRIZE_5X;
        return OE_SPIN_PRIZE_LOSE;
    }

    protected string GetPrizeLabel(int prizeType)
    {
        if (prizeType == OE_SPIN_PRIZE_JACKPOT) return "JACKPOT";
        if (prizeType == OE_SPIN_PRIZE_5X) return "5x";
        if (prizeType == OE_SPIN_PRIZE_2X) return "2x";
        if (prizeType == OE_SPIN_PRIZE_1X) return "1x";
        return "LOSE";
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
        OESpinTheWinNetState state = new OESpinTheWinNetState();
        state.status = OE_SPIN_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.jackpot = GetJackpot();
        state.stationId = stationId;
        state.message = "Set your wager and SPIN THE WHEEL.";
        SendState(player, state);
    }

    protected OESpinTheWinNetState BuildResultState(PlayerBase player, string stationId, int bet, int segmentIndex, int prizeType, string prizeLabel, int payout, int netResult, string message, bool animate)
    {
        OESpinTheWinNetState state = new OESpinTheWinNetState();
        state.status = OE_SPIN_STATUS_RESULT;
        state.bet = bet;
        state.balance = m_Currency.GetBalance(player);
        state.segmentIndex = segmentIndex;
        state.prizeType = prizeType;
        state.prizeLabel = prizeLabel;
        state.payout = payout;
        state.netResult = netResult;
        state.jackpot = GetJackpot();
        state.animate = animate;
        state.stationId = stationId;
        state.message = message;
        return state;
    }

    protected OESpinTheWinNetState CopyState(OESpinTheWinNetState source, bool animate)
    {
        OESpinTheWinNetState state = new OESpinTheWinNetState();
        state.status = source.status;
        state.bet = source.bet;
        state.balance = source.balance;
        state.segmentIndex = source.segmentIndex;
        state.prizeType = source.prizeType;
        state.payout = source.payout;
        state.netResult = source.netResult;
        state.jackpot = source.jackpot;
        state.animate = animate;
        state.stationId = source.stationId;
        state.prizeLabel = source.prizeLabel;
        state.message = source.message;
        return state;
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OESpinTheWinNetState state = new OESpinTheWinNetState();
        state.status = OE_SPIN_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.jackpot = GetJackpot();
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendState(PlayerBase player, OESpinTheWinNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SPIN_STATE, new Param1<ref OESpinTheWinNetState>(state), true, player.GetIdentity());
    }

    protected void LoadJackpot()
    {
        m_JackpotData = new OESpinTheWinJackpotData();
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        if (FileExist(OE_CASINO_SPIN_JACKPOT_FILE))
            JsonFileLoader<OESpinTheWinJackpotData>.JsonLoadFile(OE_CASINO_SPIN_JACKPOT_FILE, m_JackpotData);

        EnsureJackpotMinimum();
        SaveJackpot();
    }

    protected void EnsureJackpotMinimum()
    {
        if (!m_JackpotData) m_JackpotData = new OESpinTheWinJackpotData();
        int startValue = 10000;
        if (m_Config && m_Config.spinTheWin) startValue = m_Config.spinTheWin.jackpotStart;
        if (startValue < 1) startValue = 10000;
        if (m_JackpotData.amount < 1) m_JackpotData.amount = startValue;
    }

    protected int GetJackpot()
    {
        EnsureJackpotMinimum();
        return m_JackpotData.amount;
    }

    protected void SetJackpot(int value)
    {
        EnsureJackpotMinimum();
        if (value < 1) value = 1;
        m_JackpotData.amount = value;
        SaveJackpot();
    }

    protected void SaveJackpot()
    {
        if (!m_JackpotData) return;
        JsonFileLoader<OESpinTheWinJackpotData>.JsonSaveFile(OE_CASINO_SPIN_JACKPOT_FILE, m_JackpotData);
    }

    protected void LogResult(PlayerBase player, string stationId, int bet, int segmentIndex, string prizeLabel, int payout, int netResult, bool jackpotWin)
    {
        if (!m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;
        string jackpotFlag = "0";
        if (jackpotWin) jackpotFlag = "1";
        FPrintln(file, "SPIN_THE_WIN | " + player.GetIdentity().GetPlainId() + " | bet=" + bet + " | segment=" + segmentIndex + " | prize=" + prizeLabel + " | payout=" + payout + " | net=" + netResult + " | jackpot=" + GetJackpot() + " | jackpotWin=" + jackpotFlag + " | station=" + stationId);
        CloseFile(file);
    }
}
