class OESlotMachineServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref OESlotMachineJackpotData m_JackpotData;
    protected ref map<string, ref OESlotMachineNetState> m_LastStates;
    protected ref map<string, int> m_NextSpinAt;

    void OESlotMachineServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_LastStates = new map<string, ref OESlotMachineNetState>;
        m_NextSpinAt = new map<string, int>;
        LoadJackpot();
    }

    void ApplyConfig(OECasinoConfig config)
    {
        if (!config) return;
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        int previousJackpot = -1;
        if (m_JackpotData) previousJackpot = m_JackpotData.amount;
        EnsureJackpotMinimum();
        if (m_JackpotData && m_JackpotData.amount != previousJackpot)
        {
            SaveJackpot();
            BroadcastJackpot();
        }
    }

    void HandleSync(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_SLOT_MACHINE || !m_Config.slotMachine || m_Config.slotMachine.enabled != 1)
        {
            SendError(player, stationId, "This Slot Machine station is not available.");
            return;
        }

        string uid = player.GetIdentity().GetId();
        OESlotMachineNetState previous;
        if (m_LastStates.Find(uid, previous) && previous)
        {
            OESlotMachineNetState syncState = CopyState(previous, false);
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
        if (!station || station.game != OE_CASINO_GAME_SLOT_MACHINE || !m_Config.slotMachine || m_Config.slotMachine.enabled != 1)
        {
            SendError(player, stationId, "This Slot Machine station is not available.");
            return;
        }

        if (bet < m_Config.slotMachine.minBet || bet > m_Config.slotMachine.maxBet)
        {
            SendError(player, stationId, "Bet is outside the allowed range.");
            return;
        }

        int step = m_Config.slotMachine.betStep;
        if (step < 1) step = 1;
        if ((bet % step) != 0)
        {
            SendError(player, stationId, "Bet must use the configured wager step.");
            return;
        }

        string uid = player.GetIdentity().GetId();
        int now = GetGame().GetTime();
        int nextAllowed = 0;
        if (m_NextSpinAt.Find(uid, nextAllowed) && now < nextAllowed)
        {
            SendError(player, stationId, "The reels are still spinning.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendError(player, stationId, "Not enough casino currency.");
            return;
        }

        int reel1;
        int reel2;
        int reel3;

        reel1 = RollSymbol();
        reel2 = RollSymbol();
        reel3 = RollSymbol();

        int status = OE_SLOT_STATUS_LOSS;
        int payout = 0;
        int netResult = -bet;
        bool jackpotWin = false;
        bool jackpotEligible = IsJackpotEligible(bet);
        string resultLabel = "NO WIN";
        string message = "No winning line this spin.";

        bool triple = (reel1 == reel2 && reel2 == reel3);
        bool pair = (!triple && (reel1 == reel2 || reel1 == reel3 || reel2 == reel3));

        if (triple && reel1 == OE_SLOT_SYMBOL_SEVEN && jackpotEligible)
        {
            status = OE_SLOT_STATUS_JACKPOT;
            payout = GetJackpot();
            if (payout < m_Config.slotMachine.jackpotSeed)
                payout = m_Config.slotMachine.jackpotSeed;

            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Jackpot payout could not be created. Contact an admin.");
                return;
            }

            netResult = payout - bet;
            jackpotWin = true;
            resultLabel = "JACKPOT";
            message = "JACKPOT! Paid " + payout.ToString() + " chips.";
            SetJackpot(m_Config.slotMachine.jackpotSeed);
        }
        else if (triple)
        {
            int multiplier = GetTripleMultiplier(reel1);
            payout = bet * multiplier;
            status = OE_SLOT_STATUS_WIN;
            resultLabel = SymbolName(reel1) + " x3";

            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Slot payout could not be created. Contact an admin.");
                return;
            }

            netResult = payout - bet;
            if (reel1 == OE_SLOT_SYMBOL_SEVEN && !jackpotEligible)
                message = "7-7-7 paid " + multiplier.ToString() + "x. Max bet required for jackpot.";
            else
                message = resultLabel + " paid " + multiplier.ToString() + "x (" + payout.ToString() + " chips).";
        }
        else if (pair)
        {
            int pairMultiplier = m_Config.slotMachine.pairReturnMultiplier;
            if (pairMultiplier < 1) pairMultiplier = 1;
            payout = bet * pairMultiplier;
            status = OE_SLOT_STATUS_RETURN;
            resultLabel = "PAIR";

            if (!m_Currency.Credit(player, payout))
            {
                m_Currency.Credit(player, bet);
                SendError(player, stationId, "Slot return could not be created. Contact an admin.");
                return;
            }

            netResult = payout - bet;
            message = "Matching pair - stake returned.";
        }
        else
        {
            int jackpotBefore = GetJackpot();
            int contribution = Math.Floor(bet * (m_Config.slotMachine.jackpotContributionPercent / 100.0));
            if (contribution > 0)
                SetJackpot(jackpotBefore + contribution);

            int jackpotAfter = GetJackpot();
            int amountAdded = jackpotAfter - jackpotBefore;
            if (amountAdded > 0)
                message = "No winning line. " + amountAdded.ToString() + " chips added to the jackpot.";
            else if (jackpotAfter >= m_Config.slotMachine.jackpotMax)
                message = "No winning line. The progressive jackpot is at its configured cap.";
            else
                message = "No winning line this spin.";
        }

        m_NextSpinAt.Set(uid, now + 4000);

        OESlotMachineNetState storedState = BuildResultState(player, stationId, status, bet, reel1, reel2, reel3, payout, netResult, jackpotEligible, resultLabel, message, false);
        m_LastStates.Set(uid, storedState);

        OESlotMachineNetState animatedState = CopyState(storedState, true);
        SendState(player, animatedState);
        LogResult(player, stationId, bet, reel1, reel2, reel3, status, payout, netResult, jackpotWin);
    }

    protected int RollSymbol()
    {
        // Virtual 20-stop reel weighting:
        // LEMON 5, CHERRY 4, GRAPE 4, BELL 3, BAR 3, SEVEN 1.
        int roll = Math.RandomInt(0, 20);
        if (roll < 5) return OE_SLOT_SYMBOL_LEMON;
        if (roll < 9) return OE_SLOT_SYMBOL_CHERRY;
        if (roll < 13) return OE_SLOT_SYMBOL_GRAPE;
        if (roll < 16) return OE_SLOT_SYMBOL_BELL;
        if (roll < 19) return OE_SLOT_SYMBOL_BAR;
        return OE_SLOT_SYMBOL_SEVEN;
    }

    protected int GetTripleMultiplier(int symbol)
    {
        if (!m_Config || !m_Config.slotMachine) return 1;
        if (symbol == OE_SLOT_SYMBOL_LEMON) return m_Config.slotMachine.lemonTripleMultiplier;
        if (symbol == OE_SLOT_SYMBOL_CHERRY) return m_Config.slotMachine.cherryTripleMultiplier;
        if (symbol == OE_SLOT_SYMBOL_GRAPE) return m_Config.slotMachine.grapeTripleMultiplier;
        if (symbol == OE_SLOT_SYMBOL_BELL) return m_Config.slotMachine.bellTripleMultiplier;
        if (symbol == OE_SLOT_SYMBOL_BAR) return m_Config.slotMachine.barTripleMultiplier;
        if (symbol == OE_SLOT_SYMBOL_SEVEN) return m_Config.slotMachine.sevenTripleMultiplier;
        return 1;
    }

    protected bool IsJackpotEligible(int bet)
    {
        if (!m_Config || !m_Config.slotMachine) return false;
        if (m_Config.slotMachine.jackpotRequiresMaxBet == 0) return true;
        return bet >= m_Config.slotMachine.maxBet;
    }

    protected string SymbolName(int symbol)
    {
        if (symbol == OE_SLOT_SYMBOL_LEMON) return "LEMON";
        if (symbol == OE_SLOT_SYMBOL_CHERRY) return "CHERRY";
        if (symbol == OE_SLOT_SYMBOL_GRAPE) return "GRAPE";
        if (symbol == OE_SLOT_SYMBOL_BELL) return "BELL";
        if (symbol == OE_SLOT_SYMBOL_BAR) return "BAR";
        if (symbol == OE_SLOT_SYMBOL_SEVEN) return "7";
        return "?";
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
        OESlotMachineNetState state = new OESlotMachineNetState();
        state.status = OE_SLOT_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.jackpot = GetJackpot();
        state.jackpotEligible = IsJackpotEligible(m_Config.slotMachine.defaultBet);
        state.stationId = stationId;
        state.resultLabel = "READY";
        state.message = "Choose a wager and spin.";
        SendState(player, state);
    }

    protected OESlotMachineNetState BuildResultState(PlayerBase player, string stationId, int status, int bet, int reel1, int reel2, int reel3, int payout, int netResult, bool jackpotEligible, string resultLabel, string message, bool animate)
    {
        OESlotMachineNetState state = new OESlotMachineNetState();
        state.status = status;
        state.bet = bet;
        state.balance = m_Currency.GetBalance(player);
        state.reel1 = reel1;
        state.reel2 = reel2;
        state.reel3 = reel3;
        state.payout = payout;
        state.netResult = netResult;
        state.jackpot = GetJackpot();
        state.animate = animate;
        state.jackpotEligible = jackpotEligible;
        state.stationId = stationId;
        state.resultLabel = resultLabel;
        state.message = message;
        return state;
    }

    protected OESlotMachineNetState CopyState(OESlotMachineNetState source, bool animate)
    {
        OESlotMachineNetState state = new OESlotMachineNetState();
        state.status = source.status;
        state.bet = source.bet;
        state.balance = source.balance;
        state.reel1 = source.reel1;
        state.reel2 = source.reel2;
        state.reel3 = source.reel3;
        state.payout = source.payout;
        state.netResult = source.netResult;
        state.jackpot = source.jackpot;
        state.animate = animate;
        state.jackpotEligible = source.jackpotEligible;
        state.stationId = source.stationId;
        state.resultLabel = source.resultLabel;
        state.message = source.message;
        return state;
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OESlotMachineNetState state = new OESlotMachineNetState();
        state.status = OE_SLOT_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.jackpot = GetJackpot();
        state.stationId = stationId;
        state.resultLabel = "ERROR";
        state.message = text;
        SendState(player, state);
    }

    protected void SendState(PlayerBase player, OESlotMachineNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SLOT_STATE, new Param1<ref OESlotMachineNetState>(state), true, player.GetIdentity());
    }

    protected void LoadJackpot()
    {
        m_JackpotData = new OESlotMachineJackpotData();
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        if (FileExist(OE_CASINO_SLOT_JACKPOT_FILE))
            JsonFileLoader<OESlotMachineJackpotData>.JsonLoadFile(OE_CASINO_SLOT_JACKPOT_FILE, m_JackpotData);

        EnsureJackpotMinimum();
        SaveJackpot();
    }

    protected void EnsureJackpotMinimum()
    {
        if (!m_JackpotData) m_JackpotData = new OESlotMachineJackpotData();
        int startValue = 5000;
        int maxValue = 50000;
        if (m_Config && m_Config.slotMachine)
        {
            startValue = m_Config.slotMachine.jackpotSeed;
            maxValue = m_Config.slotMachine.jackpotMax;
        }
        if (startValue < 1) startValue = 5000;
        if (maxValue < startValue) maxValue = startValue;
        if (m_JackpotData.amount < 1) m_JackpotData.amount = startValue;
        if (m_JackpotData.amount > maxValue) m_JackpotData.amount = maxValue;
    }

    protected int GetJackpot()
    {
        EnsureJackpotMinimum();
        return m_JackpotData.amount;
    }

    protected void SetJackpot(int value)
    {
        EnsureJackpotMinimum();
        int startValue = 5000;
        int maxValue = 50000;
        if (m_Config && m_Config.slotMachine)
        {
            startValue = m_Config.slotMachine.jackpotSeed;
            maxValue = m_Config.slotMachine.jackpotMax;
        }
        if (startValue < 1) startValue = 5000;
        if (maxValue < startValue) maxValue = startValue;
        if (value < 1) value = startValue;
        if (value > maxValue) value = maxValue;
        if (m_JackpotData.amount == value) return;
        m_JackpotData.amount = value;
        SaveJackpot();
        BroadcastJackpot();
    }

    protected void SaveJackpot()
    {
        if (!m_JackpotData) return;
        JsonFileLoader<OESlotMachineJackpotData>.JsonSaveFile(OE_CASINO_SLOT_JACKPOT_FILE, m_JackpotData);
    }

    protected void BroadcastJackpot()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player || !player.GetIdentity()) continue;
            GetGame().RPCSingleParam(player, OE_CASINO_RPC_SLOT_JACKPOT, new Param1<int>(GetJackpot()), true, player.GetIdentity());
        }
    }

    protected void LogResult(PlayerBase player, string stationId, int bet, int reel1, int reel2, int reel3, int status, int payout, int netResult, bool jackpotWin)
    {
        if (!m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;
        string jackpotFlag = "0";
        if (jackpotWin) jackpotFlag = "1";
        FPrintln(file, "SLOT_MACHINE | " + player.GetIdentity().GetPlainId() + " | bet=" + bet + " | reels=" + SymbolName(reel1) + "," + SymbolName(reel2) + "," + SymbolName(reel3) + " | status=" + status + " | payout=" + payout + " | net=" + netResult + " | jackpot=" + GetJackpot() + " | jackpotWin=" + jackpotFlag + " | station=" + stationId);
        CloseFile(file);
    }
}
