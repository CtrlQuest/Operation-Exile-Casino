class OESlotMachineMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Result;
    protected TextWidget m_Jackpot;
    protected TextWidget m_JackpotHint;
    protected TextWidget m_Message;
    protected TextWidget m_SelectedBetText;

    protected ImageWidget m_Reel1Backdrop;
    protected ImageWidget m_Reel1Top;
    protected ImageWidget m_Reel1Center;
    protected ImageWidget m_Reel1Bottom;
    protected ImageWidget m_Reel2Backdrop;
    protected ImageWidget m_Reel2Top;
    protected ImageWidget m_Reel2Center;
    protected ImageWidget m_Reel2Bottom;
    protected ImageWidget m_Reel3Backdrop;
    protected ImageWidget m_Reel3Top;
    protected ImageWidget m_Reel3Center;
    protected ImageWidget m_Reel3Bottom;

    protected ImageWidget m_PayLemonIcon;
    protected ImageWidget m_PayCherryIcon;
    protected ImageWidget m_PayGrapeIcon;
    protected ImageWidget m_PayBellIcon;
    protected ImageWidget m_PayBarIcon;
    protected ImageWidget m_PaySevenIcon;
    protected TextWidget m_PayoutTitle;
    protected TextWidget m_PayLemonText;
    protected TextWidget m_PayCherryText;
    protected TextWidget m_PayGrapeText;
    protected TextWidget m_PayBellText;
    protected TextWidget m_PayBarText;
    protected TextWidget m_PaySevenText;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinusLarge;
    protected ButtonWidget m_BetMinusSmall;
    protected ButtonWidget m_BetPlusSmall;
    protected ButtonWidget m_BetPlusLarge;
    protected ButtonWidget m_MaxBet;
    protected ButtonWidget m_Spin;

    protected string m_StationId;
    protected int m_SelectedBet = 100;
    protected bool m_WaitingForResponse;
    protected bool m_IsSpinning;
    protected float m_SpinElapsed;
    protected float m_CycleElapsed;
    protected bool m_Reel1Stopped;
    protected bool m_Reel2Stopped;
    protected bool m_Reel3Stopped;
    protected ref OESlotMachineNetState m_PendingState;
    protected int m_DeferredJackpot = -1;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/SlotMachine.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Jackpot = TextWidget.Cast(m_Root.FindAnyWidget("jackpotValue"));
        m_JackpotHint = TextWidget.Cast(m_Root.FindAnyWidget("jackpotHintText"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));

        m_Reel1Backdrop = ImageWidget.Cast(m_Root.FindAnyWidget("reel1Backdrop"));
        m_Reel1Top = ImageWidget.Cast(m_Root.FindAnyWidget("reel1Top"));
        m_Reel1Center = ImageWidget.Cast(m_Root.FindAnyWidget("reel1Center"));
        m_Reel1Bottom = ImageWidget.Cast(m_Root.FindAnyWidget("reel1Bottom"));
        m_Reel2Backdrop = ImageWidget.Cast(m_Root.FindAnyWidget("reel2Backdrop"));
        m_Reel2Top = ImageWidget.Cast(m_Root.FindAnyWidget("reel2Top"));
        m_Reel2Center = ImageWidget.Cast(m_Root.FindAnyWidget("reel2Center"));
        m_Reel2Bottom = ImageWidget.Cast(m_Root.FindAnyWidget("reel2Bottom"));
        m_Reel3Backdrop = ImageWidget.Cast(m_Root.FindAnyWidget("reel3Backdrop"));
        m_Reel3Top = ImageWidget.Cast(m_Root.FindAnyWidget("reel3Top"));
        m_Reel3Center = ImageWidget.Cast(m_Root.FindAnyWidget("reel3Center"));
        m_Reel3Bottom = ImageWidget.Cast(m_Root.FindAnyWidget("reel3Bottom"));

        m_PayLemonIcon = ImageWidget.Cast(m_Root.FindAnyWidget("payLemonIcon"));
        m_PayCherryIcon = ImageWidget.Cast(m_Root.FindAnyWidget("payCherryIcon"));
        m_PayGrapeIcon = ImageWidget.Cast(m_Root.FindAnyWidget("payGrapeIcon"));
        m_PayBellIcon = ImageWidget.Cast(m_Root.FindAnyWidget("payBellIcon"));
        m_PayBarIcon = ImageWidget.Cast(m_Root.FindAnyWidget("payBarIcon"));
        m_PaySevenIcon = ImageWidget.Cast(m_Root.FindAnyWidget("paySevenIcon"));
        m_PayoutTitle = TextWidget.Cast(m_Root.FindAnyWidget("payoutTitle"));
        m_PayLemonText = TextWidget.Cast(m_Root.FindAnyWidget("payLemonText"));
        m_PayCherryText = TextWidget.Cast(m_Root.FindAnyWidget("payCherryText"));
        m_PayGrapeText = TextWidget.Cast(m_Root.FindAnyWidget("payGrapeText"));
        m_PayBellText = TextWidget.Cast(m_Root.FindAnyWidget("payBellText"));
        m_PayBarText = TextWidget.Cast(m_Root.FindAnyWidget("payBarText"));
        m_PaySevenText = TextWidget.Cast(m_Root.FindAnyWidget("paySevenText"));

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinusLarge = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinusLarge"));
        m_BetMinusSmall = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinusSmall"));
        m_BetPlusSmall = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlusSmall"));
        m_BetPlusLarge = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlusLarge"));
        m_MaxBet = ButtonWidget.Cast(m_Root.FindAnyWidget("maxBetButton"));
        m_Spin = ButtonWidget.Cast(m_Root.FindAnyWidget("spinButton"));

        LoadReelBackdrops();
        LoadDefaultBet();
        ClampSelectedBet();
        LoadPaytableIcons();
        UpdateBetControls();
        RenderReel1(OE_SLOT_SYMBOL_LEMON);
        RenderReel2(OE_SLOT_SYMBOL_CHERRY);
        RenderReel3(OE_SLOT_SYMBOL_GRAPE);
        return m_Root;
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
        m_PendingState = null;
        m_DeferredJackpot = -1;
        RequestState();
    }

    override void OnHide()
    {
        m_IsSpinning = false;
        m_WaitingForResponse = false;
        m_PendingState = null;
        m_DeferredJackpot = -1;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.PlayDelayedPayoutSound);

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

        int smallStep = GetBetStep();
        int largeStep = smallStep * 10;
        if (w == m_BetMinusLarge) { AdjustBet(-largeStep); return true; }
        if (w == m_BetMinusSmall) { AdjustBet(-smallStep); return true; }
        if (w == m_BetPlusSmall) { AdjustBet(smallStep); return true; }
        if (w == m_BetPlusLarge) { AdjustBet(largeStep); return true; }
        if (w == m_MaxBet) { SetMaxBet(); return true; }

        if (w == m_Spin)
        {
            m_WaitingForResponse = true;
            m_DeferredJackpot = -1;
            if (m_Bet) m_Bet.SetText(m_SelectedBet.ToString());
            if (m_Result) m_Result.SetText("SPINNING");
            if (m_Message) m_Message.SetText("Waiting for the server result...");
            SendSpin();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OESlotMachineNetState state)
    {
        if (!state) return;

        if (state.status == OE_SLOT_STATUS_ERROR)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            if (m_Balance) m_Balance.SetText(state.balance.ToString());
            if (m_Jackpot) m_Jackpot.SetText(state.jackpot.ToString());
            if (m_Result) m_Result.SetText("ERROR");
            if (m_Message) m_Message.SetText(state.message);
            return;
        }

        if (state.status == OE_SLOT_STATUS_IDLE)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            if (m_Balance) m_Balance.SetText(state.balance.ToString());
            if (m_Bet) m_Bet.SetText("0");
            if (m_Jackpot) m_Jackpot.SetText(state.jackpot.ToString());
            if (m_Result) m_Result.SetText("READY");
            if (m_Message) m_Message.SetText(state.message);
            UpdateJackpotHint();
            return;
        }

        if (state.animate)
        {
            BeginSpinAnimation(state);
            return;
        }

        m_WaitingForResponse = false;
        m_IsSpinning = false;
        RenderReel1(state.reel1);
        RenderReel2(state.reel2);
        RenderReel3(state.reel3);
        ShowFinalState(state, false);
    }

    void ApplyJackpot(int amount)
    {
        if (amount < 0) return;
        if (m_WaitingForResponse || m_IsSpinning)
        {
            m_DeferredJackpot = amount;
            return;
        }
        if (m_Jackpot) m_Jackpot.SetText(amount.ToString());
    }

    protected void BeginSpinAnimation(OESlotMachineNetState state)
    {
        m_PendingState = state;
        m_WaitingForResponse = true;
        m_IsSpinning = true;
        m_SpinElapsed = 0.0;
        m_CycleElapsed = 0.0;
        m_Reel1Stopped = false;
        m_Reel2Stopped = false;
        m_Reel3Stopped = false;

        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Result) m_Result.SetText("SPINNING");
        if (m_Message) m_Message.SetText("Reels spinning...");
        OECasinoAudio.PlaySlotSpin();
    }

    protected void UpdateSpin(float timeslice)
    {
        if (!m_PendingState)
        {
            m_IsSpinning = false;
            m_WaitingForResponse = false;
            return;
        }

        m_SpinElapsed += timeslice;
        m_CycleElapsed += timeslice;

        if (m_CycleElapsed >= 0.07)
        {
            m_CycleElapsed = 0.0;
            if (!m_Reel1Stopped) RenderReel1(Math.RandomInt(0, 6));
            if (!m_Reel2Stopped) RenderReel2(Math.RandomInt(0, 6));
            if (!m_Reel3Stopped) RenderReel3(Math.RandomInt(0, 6));
        }

        if (!m_Reel1Stopped && m_SpinElapsed >= 2.85)
        {
            m_Reel1Stopped = true;
            RenderReel1(m_PendingState.reel1);
            OECasinoAudio.PlaySlotReelStop();
        }

        if (!m_Reel2Stopped && m_SpinElapsed >= 3.28)
        {
            m_Reel2Stopped = true;
            RenderReel2(m_PendingState.reel2);
            OECasinoAudio.PlaySlotReelStop();
        }

        if (!m_Reel3Stopped && m_SpinElapsed >= 3.70)
        {
            m_Reel3Stopped = true;
            RenderReel3(m_PendingState.reel3);
            OECasinoAudio.PlaySlotReelStop();
        }

        if (m_SpinElapsed >= OECasinoAudio.GetSlotSpinDuration())
        {
            if (!m_Reel1Stopped) RenderReel1(m_PendingState.reel1);
            if (!m_Reel2Stopped) RenderReel2(m_PendingState.reel2);
            if (!m_Reel3Stopped) RenderReel3(m_PendingState.reel3);

            m_IsSpinning = false;
            m_WaitingForResponse = false;
            ShowFinalState(m_PendingState, true);
            m_PendingState = null;

            if (m_DeferredJackpot >= 0)
            {
                if (m_Jackpot) m_Jackpot.SetText(m_DeferredJackpot.ToString());
                m_DeferredJackpot = -1;
            }
        }
    }

    protected void ShowFinalState(OESlotMachineNetState state, bool playOutcomeSound)
    {
        if (!state) return;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Jackpot) m_Jackpot.SetText(state.jackpot.ToString());
        if (m_Message) m_Message.SetText(state.message);

        if (m_Result)
        {
            if (state.status == OE_SLOT_STATUS_JACKPOT)
                m_Result.SetText("JACKPOT +" + state.netResult.ToString());
            else if (state.status == OE_SLOT_STATUS_WIN)
                m_Result.SetText("WIN +" + state.netResult.ToString());
            else if (state.status == OE_SLOT_STATUS_RETURN)
                m_Result.SetText("BET RETURNED");
            else
                m_Result.SetText("LOSS -" + state.bet.ToString());
        }

        if (playOutcomeSound)
        {
            if (state.status == OE_SLOT_STATUS_JACKPOT)
            {
                OECasinoAudio.PlaySlotJackpot();
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.PlayDelayedPayoutSound, 650, false);
            }
            else if (state.status == OE_SLOT_STATUS_WIN)
            {
                OECasinoAudio.PlaySlotWin();
            }
            else if (state.status == OE_SLOT_STATUS_LOSS)
            {
                OECasinoAudio.PlaySlotLose();
            }
        }

        UpdateJackpotHint();
    }

    protected void PlayDelayedPayoutSound()
    {
        OECasinoAudio.PlaySlotPayout();
    }

    protected void LoadReelBackdrops()
    {
        string reelBackdropPath = "OperationExileCasino/data/slots/slot_reel_window.edds";

        if (m_Reel1Backdrop)
        {
            m_Reel1Backdrop.LoadImageFile(0, reelBackdropPath);
            m_Reel1Backdrop.SetImage(0);
        }

        if (m_Reel2Backdrop)
        {
            m_Reel2Backdrop.LoadImageFile(0, reelBackdropPath);
            m_Reel2Backdrop.SetImage(0);
        }

        if (m_Reel3Backdrop)
        {
            m_Reel3Backdrop.LoadImageFile(0, reelBackdropPath);
            m_Reel3Backdrop.SetImage(0);
        }
    }

    protected string SymbolTexturePath(int symbol)
    {
        if (symbol == OE_SLOT_SYMBOL_LEMON) return "OperationExileCasino/data/slots/slot_lemon.edds";
        if (symbol == OE_SLOT_SYMBOL_CHERRY) return "OperationExileCasino/data/slots/slot_cherry.edds";
        if (symbol == OE_SLOT_SYMBOL_GRAPE) return "OperationExileCasino/data/slots/slot_grape.edds";
        if (symbol == OE_SLOT_SYMBOL_BELL) return "OperationExileCasino/data/slots/slot_bell.edds";
        if (symbol == OE_SLOT_SYMBOL_BAR) return "OperationExileCasino/data/slots/slot_bar.edds";
        if (symbol == OE_SLOT_SYMBOL_SEVEN) return "OperationExileCasino/data/slots/slot_seven.edds";
        return "OperationExileCasino/data/slots/slot_lemon.edds";
    }

    protected void RenderSymbol(ImageWidget widget, int symbol)
    {
        if (!widget) return;
        widget.LoadImageFile(0, SymbolTexturePath(symbol));
        widget.SetImage(0);
        widget.Show(true);
    }

    protected void LoadPaytableIcons()
    {
        RenderSymbol(m_PayLemonIcon, OE_SLOT_SYMBOL_LEMON);
        RenderSymbol(m_PayCherryIcon, OE_SLOT_SYMBOL_CHERRY);
        RenderSymbol(m_PayGrapeIcon, OE_SLOT_SYMBOL_GRAPE);
        RenderSymbol(m_PayBellIcon, OE_SLOT_SYMBOL_BELL);
        RenderSymbol(m_PayBarIcon, OE_SLOT_SYMBOL_BAR);
        RenderSymbol(m_PaySevenIcon, OE_SLOT_SYMBOL_SEVEN);

        int pairMultiplier = 1;
        int lemonMultiplier = 5;
        int cherryMultiplier = 8;
        int grapeMultiplier = 12;
        int bellMultiplier = 18;
        int barMultiplier = 30;
        int sevenMultiplier = 75;

        if (OECasinoConfig.Instance && OECasinoConfig.Instance.slotMachine)
        {
            pairMultiplier = OECasinoConfig.Instance.slotMachine.pairReturnMultiplier;
            lemonMultiplier = OECasinoConfig.Instance.slotMachine.lemonTripleMultiplier;
            cherryMultiplier = OECasinoConfig.Instance.slotMachine.cherryTripleMultiplier;
            grapeMultiplier = OECasinoConfig.Instance.slotMachine.grapeTripleMultiplier;
            bellMultiplier = OECasinoConfig.Instance.slotMachine.bellTripleMultiplier;
            barMultiplier = OECasinoConfig.Instance.slotMachine.barTripleMultiplier;
            sevenMultiplier = OECasinoConfig.Instance.slotMachine.sevenTripleMultiplier;
        }

        if (m_PayoutTitle)
        {
            if (pairMultiplier == 1)
                m_PayoutTitle.SetText("PAYOUTS | PAIR = BET BACK | 7-7-7 = JACKPOT ON ELIGIBLE BET");
            else
                m_PayoutTitle.SetText("PAYOUTS | PAIR x" + pairMultiplier.ToString() + " | 7-7-7 = JACKPOT ON ELIGIBLE BET");
        }

        if (m_PayLemonText) m_PayLemonText.SetText("x" + lemonMultiplier.ToString());
        if (m_PayCherryText) m_PayCherryText.SetText("x" + cherryMultiplier.ToString());
        if (m_PayGrapeText) m_PayGrapeText.SetText("x" + grapeMultiplier.ToString());
        if (m_PayBellText) m_PayBellText.SetText("x" + bellMultiplier.ToString());
        if (m_PayBarText) m_PayBarText.SetText("x" + barMultiplier.ToString());
        if (m_PaySevenText) m_PaySevenText.SetText("x" + sevenMultiplier.ToString() + " / JACKPOT");
    }

    protected int PreviousSymbol(int symbol)
    {
        int value = symbol - 1;
        if (value < 0) value = 5;
        return value;
    }

    protected int NextSymbol(int symbol)
    {
        int value = symbol + 1;
        if (value > 5) value = 0;
        return value;
    }

    protected void RenderReel1(int symbol)
    {
        RenderSymbol(m_Reel1Top, PreviousSymbol(symbol));
        RenderSymbol(m_Reel1Center, symbol);
        RenderSymbol(m_Reel1Bottom, NextSymbol(symbol));
    }

    protected void RenderReel2(int symbol)
    {
        RenderSymbol(m_Reel2Top, PreviousSymbol(symbol));
        RenderSymbol(m_Reel2Center, symbol);
        RenderSymbol(m_Reel2Bottom, NextSymbol(symbol));
    }

    protected void RenderReel3(int symbol)
    {
        RenderSymbol(m_Reel3Top, PreviousSymbol(symbol));
        RenderSymbol(m_Reel3Center, symbol);
        RenderSymbol(m_Reel3Bottom, NextSymbol(symbol));
    }

    protected int GetBetStep()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.slotMachine && OECasinoConfig.Instance.slotMachine.betStep > 0)
            return OECasinoConfig.Instance.slotMachine.betStep;
        return 10;
    }

    protected void LoadDefaultBet()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.slotMachine)
            m_SelectedBet = OECasinoConfig.Instance.slotMachine.defaultBet;
    }

    protected void AdjustBet(int delta)
    {
        m_SelectedBet += delta;
        ClampSelectedBet();
        UpdateBetControls();
    }

    protected void SetMaxBet()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.slotMachine)
            m_SelectedBet = OECasinoConfig.Instance.slotMachine.maxBet;
        ClampSelectedBet();
        UpdateBetControls();
    }

    protected void ClampSelectedBet()
    {
        int minBet = 10;
        int maxBet = 1000;
        int step = GetBetStep();

        if (OECasinoConfig.Instance && OECasinoConfig.Instance.slotMachine)
        {
            minBet = OECasinoConfig.Instance.slotMachine.minBet;
            maxBet = OECasinoConfig.Instance.slotMachine.maxBet;
        }

        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;

        if (step > 1)
        {
            int offset = m_SelectedBet - minBet;
            int steps = offset / step;
            m_SelectedBet = minBet + (steps * step);
            if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        }
    }

    protected void UpdateBetControls()
    {
        if (m_SelectedBetText) m_SelectedBetText.SetText(m_SelectedBet.ToString());
        int step = GetBetStep();
        if (m_BetMinusSmall) m_BetMinusSmall.SetText("-" + step.ToString());
        if (m_BetPlusSmall) m_BetPlusSmall.SetText("+" + step.ToString());
        if (m_BetMinusLarge) m_BetMinusLarge.SetText("-" + (step * 10).ToString());
        if (m_BetPlusLarge) m_BetPlusLarge.SetText("+" + (step * 10).ToString());
        UpdateJackpotHint();
    }

    protected void UpdateJackpotHint()
    {
        if (!m_JackpotHint) return;
        if (!OECasinoConfig.Instance || !OECasinoConfig.Instance.slotMachine) return;

        if (OECasinoConfig.Instance.slotMachine.jackpotRequiresMaxBet == 0)
        {
            m_JackpotHint.SetText("7-7-7 WINS THE PROGRESSIVE JACKPOT");
            return;
        }

        int maxBet = OECasinoConfig.Instance.slotMachine.maxBet;
        if (m_SelectedBet >= maxBet)
            m_JackpotHint.SetText("JACKPOT ELIGIBLE | 7-7-7 WINS THE POT");
        else
            m_JackpotHint.SetText("MAX BET " + maxBet.ToString() + " | 7-7-7 WINS JACKPOT");
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SLOT_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendSpin()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_SLOT_SPIN, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
