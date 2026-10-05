class OESpinTheWinMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Result;
    protected TextWidget m_Jackpot;
    protected TextWidget m_CurrentPrize;
    protected TextWidget m_Message;
    protected TextWidget m_SelectedBetText;

    protected ImageWidget m_Wheel;
    protected ImageWidget m_LightsA;
    protected ImageWidget m_LightsB;
    protected ImageWidget m_Pointer;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Spin;

    protected string m_StationId;
    protected int m_SelectedBet = 100;
    protected bool m_WaitingForResponse;
    protected bool m_IsSpinning;
    protected float m_SpinElapsed;
    protected float m_SpinDuration;
    protected float m_StartRotation;
    protected float m_EndRotation;
    protected float m_CurrentRotation;
    protected float m_LightElapsed;
    protected bool m_LightPhase;
    protected ref OESpinTheWinNetState m_PendingState;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/SpinTheWin.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Jackpot = TextWidget.Cast(m_Root.FindAnyWidget("jackpotValue"));
        m_CurrentPrize = TextWidget.Cast(m_Root.FindAnyWidget("currentPrizeText"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));

        m_Wheel = ImageWidget.Cast(m_Root.FindAnyWidget("wheelImage"));
        m_LightsA = ImageWidget.Cast(m_Root.FindAnyWidget("wheelLightsA"));
        m_LightsB = ImageWidget.Cast(m_Root.FindAnyWidget("wheelLightsB"));
        m_Pointer = ImageWidget.Cast(m_Root.FindAnyWidget("wheelPointer"));

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Spin = ButtonWidget.Cast(m_Root.FindAnyWidget("spinButton"));

        LoadAssets();
        LoadDefaultBet();
        ClampSelectedBet();
        UpdateSelectedBetText();
        ResetWheelVisuals();
        return m_Root;
    }

    protected void LoadAssets()
    {
        if (m_Wheel)
        {
            m_Wheel.LoadImageFile(0, "OperationExileCasino/data/spin/oe_spin_wheel.edds");
            m_Wheel.SetImage(0);
        }

        if (m_LightsA)
        {
            m_LightsA.LoadImageFile(0, "OperationExileCasino/data/spin/oe_spin_lights_a.edds");
            m_LightsA.SetImage(0);
            m_LightsA.Show(false);
        }

        if (m_LightsB)
        {
            m_LightsB.LoadImageFile(0, "OperationExileCasino/data/spin/oe_spin_lights_b.edds");
            m_LightsB.SetImage(0);
            m_LightsB.Show(false);
        }

        if (m_Pointer)
        {
            m_Pointer.LoadImageFile(0, "OperationExileCasino/data/spin/oe_spin_pointer.edds");
            m_Pointer.SetImage(0);
        }
    }

    override void OnShow()
    {
        super.OnShow();
        PPEffects.SetBlurMenu(0.35);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_INVENTORY);
        GetGame().GetUIManager().ShowUICursor(true);
        GetGame().GetUIManager().ShowCursor(true);
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().GetHud().Show(false);

        m_WaitingForResponse = false;
        m_IsSpinning = false;
        HideLights();
        RequestState();
    }

    override void OnHide()
    {
        m_IsSpinning = false;
        m_WaitingForResponse = false;
        m_PendingState = null;
        HideLights();

        super.OnHide();
        PPEffects.SetBlurMenu(0);
        GetGame().GetUIManager().ShowCursor(false);
        GetGame().GetUIManager().ShowUICursor(false);
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetMission().PlayerControlEnable(true);
        GetGame().GetMission().GetHud().Show(true);
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_IsSpinning)
            UpdateSpin(timeslice);

        if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress())
            CloseMenu();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_Close)
        {
            CloseMenu();
            return true;
        }

        if (m_WaitingForResponse || m_IsSpinning) return true;

        if (w == m_BetMinus100) { AdjustBet(-100); return true; }
        if (w == m_BetMinus10) { AdjustBet(-10); return true; }
        if (w == m_BetPlus10) { AdjustBet(10); return true; }
        if (w == m_BetPlus100) { AdjustBet(100); return true; }

        if (w == m_Spin)
        {
            m_WaitingForResponse = true;
            if (m_Result) m_Result.SetText("SPINNING");
            if (m_CurrentPrize) m_CurrentPrize.SetText("WAITING FOR SERVER...");
            if (m_Message) m_Message.SetText("The server is choosing the winning segment.");
            SendSpin();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OESpinTheWinNetState state)
    {
        if (!state) return;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Jackpot) m_Jackpot.SetText(state.jackpot.ToString());

        if (state.status == OE_SPIN_STATUS_ERROR)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            HideLights();
            if (m_Result) m_Result.SetText("ERROR");
            if (m_Message) m_Message.SetText(state.message);
            if (m_CurrentPrize) m_CurrentPrize.SetText("READY TO SPIN");
            return;
        }

        if (state.status == OE_SPIN_STATUS_IDLE)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            if (m_Bet) m_Bet.SetText("0");
            if (m_Result) m_Result.SetText("-");
            if (m_CurrentPrize) m_CurrentPrize.SetText("READY TO SPIN");
            if (m_Message) m_Message.SetText(state.message);
            HideLights();
            return;
        }

        if (state.animate)
        {
            BeginSpinAnimation(state);
            return;
        }

        m_WaitingForResponse = false;
        m_IsSpinning = false;
        SetWheelToSegment(state.segmentIndex);
        ShowFinalState(state);
    }

    protected void BeginSpinAnimation(OESpinTheWinNetState state)
    {
        m_PendingState = state;
        m_WaitingForResponse = true;
        m_IsSpinning = true;
        m_SpinElapsed = 0.0;
        m_LightElapsed = 0.0;
        m_LightPhase = false;
        // Keep the wheel moving for the full supplied spin audio.
        m_SpinDuration = OECasinoAudio.GetSpinTheWinDuration();
        m_StartRotation = NormalizeAngle(m_CurrentRotation);

        float targetRotation = GetTargetRotationForSegment(state.segmentIndex);
        float delta = targetRotation - m_StartRotation;
        while (delta < 0.0) delta += 360.0;

        m_EndRotation = m_StartRotation + (GetMinimumRotations() * 360.0) + delta;

        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Result) m_Result.SetText("SPINNING");
        if (m_CurrentPrize) m_CurrentPrize.SetText("CURRENT: " + GetPrizeLabelForRotation(m_StartRotation));
        if (m_Message) m_Message.SetText("Wheel spinning...");
        ShowLightPhase(false);
        OECasinoAudio.PlaySpinTheWin();
    }

    protected void UpdateSpin(float timeslice)
    {
        if (!m_PendingState)
        {
            m_IsSpinning = false;
            m_WaitingForResponse = false;
            HideLights();
            return;
        }

        m_SpinElapsed += timeslice;
        float t = m_SpinElapsed / m_SpinDuration;
        if (t > 1.0) t = 1.0;

        float inv = 1.0 - t;
        float eased = 1.0 - (inv * inv * inv);
        m_CurrentRotation = m_StartRotation + ((m_EndRotation - m_StartRotation) * eased);

        if (m_Wheel) m_Wheel.SetRotation(0, 0, m_CurrentRotation);
        if (m_CurrentPrize) m_CurrentPrize.SetText("CURRENT: " + GetPrizeLabelForRotation(m_CurrentRotation));

        m_LightElapsed += timeslice;
        if (m_LightElapsed >= 0.11)
        {
            m_LightElapsed = 0.0;
            m_LightPhase = !m_LightPhase;
            ShowLightPhase(m_LightPhase);
        }

        if (t >= 1.0)
        {
            m_CurrentRotation = GetTargetRotationForSegment(m_PendingState.segmentIndex);
            if (m_Wheel) m_Wheel.SetRotation(0, 0, m_CurrentRotation);

            m_IsSpinning = false;
            m_WaitingForResponse = false;

            if (m_PendingState.prizeType == OE_SPIN_PRIZE_LOSE)
                HideLights();
            else
                ShowAllLights();

            ShowFinalState(m_PendingState);
            m_PendingState = null;
        }
    }

    protected void ShowFinalState(OESpinTheWinNetState state)
    {
        if (!state) return;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Jackpot) m_Jackpot.SetText(state.jackpot.ToString());
        if (m_CurrentPrize) m_CurrentPrize.SetText("LANDED ON: " + state.prizeLabel);
        if (m_Message) m_Message.SetText(state.message);

        if (m_Result)
        {
            if (state.prizeType == OE_SPIN_PRIZE_JACKPOT)
                m_Result.SetText("JACKPOT +" + state.netResult.ToString());
            else if (state.prizeType == OE_SPIN_PRIZE_LOSE)
                m_Result.SetText("LOSS -" + state.bet.ToString());
            else if (state.netResult == 0)
                m_Result.SetText("RETURN 0");
            else
                m_Result.SetText("WIN +" + state.netResult.ToString());
        }
    }

    protected void SetWheelToSegment(int segmentIndex)
    {
        m_CurrentRotation = GetTargetRotationForSegment(segmentIndex);
        if (m_Wheel) m_Wheel.SetRotation(0, 0, m_CurrentRotation);
    }

    protected float GetTargetRotationForSegment(int segmentIndex)
    {
        float target = 360.0 - (segmentIndex * 22.5);
        return NormalizeAngle(target);
    }

    protected float NormalizeAngle(float angle)
    {
        while (angle >= 360.0) angle -= 360.0;
        while (angle < 0.0) angle += 360.0;
        return angle;
    }

    protected string GetPrizeLabelForRotation(float rotation)
    {
        float normalized = NormalizeAngle(rotation);
        float originalAngleAtPointer = 360.0 - normalized;
        if (originalAngleAtPointer >= 360.0) originalAngleAtPointer -= 360.0;

        int segmentIndex = Math.Floor((originalAngleAtPointer + 11.25) / 22.5);
        if (segmentIndex >= 16) segmentIndex = 0;
        return GetPrizeLabelForSegment(segmentIndex);
    }

    protected string GetPrizeLabelForSegment(int segmentIndex)
    {
        if (segmentIndex == 0) return "JACKPOT";
        if (segmentIndex == 2 || segmentIndex == 6 || segmentIndex == 10 || segmentIndex == 14) return "1x";
        if (segmentIndex == 4 || segmentIndex == 12) return "2x";
        if (segmentIndex == 8 || segmentIndex == 15) return "5x";
        return "LOSE";
    }

    protected float GetSpinDuration()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.spinTheWin)
            return OECasinoConfig.Instance.spinTheWin.spinDurationSeconds;
        return 5.0;
    }

    protected int GetMinimumRotations()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.spinTheWin)
            return OECasinoConfig.Instance.spinTheWin.minimumRotations;
        return 4;
    }

    protected void LoadDefaultBet()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.spinTheWin)
            m_SelectedBet = OECasinoConfig.Instance.spinTheWin.defaultBet;
    }

    protected void AdjustBet(int delta)
    {
        m_SelectedBet += delta;
        ClampSelectedBet();
        UpdateSelectedBetText();
    }

    protected void ClampSelectedBet()
    {
        int minBet = 100;
        int maxBet = 5000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.spinTheWin)
        {
            minBet = OECasinoConfig.Instance.spinTheWin.minBet;
            maxBet = OECasinoConfig.Instance.spinTheWin.maxBet;
        }

        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
    }

    protected void UpdateSelectedBetText()
    {
        if (m_SelectedBetText) m_SelectedBetText.SetText(m_SelectedBet.ToString());
    }

    protected void ResetWheelVisuals()
    {
        m_CurrentRotation = 0.0;
        if (m_Wheel) m_Wheel.SetRotation(0, 0, 0);
        HideLights();
    }

    protected void ShowLightPhase(bool phaseB)
    {
        if (m_LightsA) m_LightsA.Show(!phaseB);
        if (m_LightsB) m_LightsB.Show(phaseB);
    }

    protected void ShowAllLights()
    {
        if (m_LightsA) m_LightsA.Show(true);
        if (m_LightsB) m_LightsB.Show(true);
    }

    protected void HideLights()
    {
        if (m_LightsA) m_LightsA.Show(false);
        if (m_LightsB) m_LightsB.Show(false);
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SPIN_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendSpin()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SPIN_START, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
