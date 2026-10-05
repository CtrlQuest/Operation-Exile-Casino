class OEDiceBettingMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Result;
    protected TextWidget m_RollSummary;
    protected TextWidget m_Die1;
    protected TextWidget m_Die2;
    protected TextWidget m_Total;
    protected TextWidget m_Message;
    protected TextWidget m_SelectedBetText;
    protected TextWidget m_SelectedBetDescription;
    protected TextWidget m_ExactTargetText;
    protected TextWidget m_ExactTargetLabel;
    protected Widget m_ExactControlsPanel;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_Low;
    protected ButtonWidget m_High;
    protected ButtonWidget m_Odd;
    protected ButtonWidget m_Even;
    protected ButtonWidget m_Doubles;
    protected ButtonWidget m_Exact;
    protected ButtonWidget m_ExactMinus;
    protected ButtonWidget m_ExactPlus;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Roll;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected bool m_RollRequested;
    protected bool m_RollAnimating;
    protected float m_RollTickElapsed;
    protected ref OEDiceBettingNetState m_PendingRollState;
    protected int m_SelectedBet = 100;
    protected int m_SelectedBetType = OE_DICE_BET_LOW;
    protected int m_ExactTarget = 7;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/DiceBetting.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_RollSummary = TextWidget.Cast(m_Root.FindAnyWidget("rollSummaryValue"));
        m_Die1 = TextWidget.Cast(m_Root.FindAnyWidget("die1Value"));
        m_Die2 = TextWidget.Cast(m_Root.FindAnyWidget("die2Value"));
        m_Total = TextWidget.Cast(m_Root.FindAnyWidget("totalValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_SelectedBetDescription = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetDescription"));
        m_ExactTargetText = TextWidget.Cast(m_Root.FindAnyWidget("exactTargetValue"));
        m_ExactTargetLabel = TextWidget.Cast(m_Root.FindAnyWidget("exactTargetLabel"));
        m_ExactControlsPanel = m_Root.FindAnyWidget("exactControlsPanel");

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_Low = ButtonWidget.Cast(m_Root.FindAnyWidget("lowButton"));
        m_High = ButtonWidget.Cast(m_Root.FindAnyWidget("highButton"));
        m_Odd = ButtonWidget.Cast(m_Root.FindAnyWidget("oddButton"));
        m_Even = ButtonWidget.Cast(m_Root.FindAnyWidget("evenButton"));
        m_Doubles = ButtonWidget.Cast(m_Root.FindAnyWidget("doublesButton"));
        m_Exact = ButtonWidget.Cast(m_Root.FindAnyWidget("exactButton"));
        m_ExactMinus = ButtonWidget.Cast(m_Root.FindAnyWidget("exactMinus"));
        m_ExactPlus = ButtonWidget.Cast(m_Root.FindAnyWidget("exactPlus"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Roll = ButtonWidget.Cast(m_Root.FindAnyWidget("rollButton"));

        ClampSelectedBet();
        UpdateSelectionText();
        UpdateExactControlsVisibility();
        ClearDice();
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
        RequestState();
    }

    override void OnHide()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishDiceRoll);
        m_PendingRollState = null;
        m_RollRequested = false;
        m_RollAnimating = false;
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
        if (m_RollAnimating) UpdateRollingDice(timeslice);
        if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress())
            CloseMenu();
    }

    protected void UpdateRollingDice(float timeslice)
    {
        m_RollTickElapsed += timeslice;
        if (m_RollTickElapsed < 0.075) return;
        m_RollTickElapsed = 0.0;

        int preview1 = Math.RandomInt(1, 7);
        int preview2 = Math.RandomInt(1, 7);
        if (m_Die1) m_Die1.SetText(preview1.ToString());
        if (m_Die2) m_Die2.SetText(preview2.ToString());
        if (m_Total) m_Total.SetText((preview1 + preview2).ToString());
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_Close)
        {
            CloseMenu();
            return true;
        }

        if (m_WaitingForResponse) return true;

        if (w == m_Low) { SelectBetType(OE_DICE_BET_LOW); return true; }
        if (w == m_High) { SelectBetType(OE_DICE_BET_HIGH); return true; }
        if (w == m_Odd) { SelectBetType(OE_DICE_BET_ODD); return true; }
        if (w == m_Even) { SelectBetType(OE_DICE_BET_EVEN); return true; }
        if (w == m_Doubles) { SelectBetType(OE_DICE_BET_DOUBLES); return true; }
        if (w == m_Exact) { SelectBetType(OE_DICE_BET_EXACT); return true; }

        if (w == m_ExactMinus)
        {
            m_ExactTarget--;
            if (m_ExactTarget < 2) m_ExactTarget = 12;
            UpdateSelectionText();
            return true;
        }

        if (w == m_ExactPlus)
        {
            m_ExactTarget++;
            if (m_ExactTarget > 12) m_ExactTarget = 2;
            UpdateSelectionText();
            return true;
        }

        if (w == m_BetMinus100) { AdjustBet(-100); return true; }
        if (w == m_BetMinus10) { AdjustBet(-10); return true; }
        if (w == m_BetPlus10) { AdjustBet(10); return true; }
        if (w == m_BetPlus100) { AdjustBet(100); return true; }

        if (w == m_Roll)
        {
            m_WaitingForResponse = true;
            m_RollRequested = true;
            if (m_Message) m_Message.SetText("Waiting for dice...");
            SendRoll();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OEDiceBettingNetState state)
    {
        if (!state) return;

        if (m_RollRequested && state.status != OE_DICE_STATUS_IDLE && state.status != OE_DICE_STATUS_ERROR && state.die1 > 0 && state.die2 > 0)
        {
            m_RollRequested = false;
            m_RollAnimating = true;
            m_RollTickElapsed = 1.0;
            m_PendingRollState = state;
            if (m_Result) m_Result.SetText("ROLLING");
            if (m_RollSummary) m_RollSummary.SetText("DICE ROLLING...");
            UpdateRollingDice(0.0);
            if (m_Message) m_Message.SetText("Rolling dice...");
            OECasinoAudio.PlayDiceRoll();
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishDiceRoll);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishDiceRoll, 850, false);
            return;
        }

        m_RollRequested = false;
        RenderStateImmediate(state);
    }

    protected void FinishDiceRoll()
    {
        if (!m_PendingRollState) return;
        OEDiceBettingNetState state = m_PendingRollState;
        m_PendingRollState = null;
        m_RollAnimating = false;
        RenderStateImmediate(state);
    }

    protected void RenderStateImmediate(OEDiceBettingNetState state)
    {
        if (!state) return;
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Message) m_Message.SetText(state.message);

        if (state.status == OE_DICE_STATUS_IDLE)
        {
            if (m_Result) m_Result.SetText("-");
            if (m_RollSummary) m_RollSummary.SetText("-");
            ClearDice();
            return;
        }

        if (state.status == OE_DICE_STATUS_ERROR)
        {
            if (m_Result) m_Result.SetText("ERROR");
            return;
        }

        if (m_Die1) m_Die1.SetText(state.die1.ToString());
        if (m_Die2) m_Die2.SetText(state.die2.ToString());
        if (m_Total) m_Total.SetText(state.total.ToString());
        if (m_RollSummary) m_RollSummary.SetText(state.die1.ToString() + " + " + state.die2.ToString() + " = " + state.total.ToString());

        if (state.status == OE_DICE_STATUS_WIN)
        {
            if (m_Result) m_Result.SetText("WIN +" + state.netResult.ToString());
        }
        else
        {
            int lostAmount = state.netResult;
            if (lostAmount < 0) lostAmount = -lostAmount;
            if (m_Result) m_Result.SetText("LOSS -" + lostAmount.ToString());
        }
    }

    protected void SelectBetType(int betType)
    {
        m_SelectedBetType = betType;
        UpdateSelectionText();
        UpdateExactControlsVisibility();
    }

    protected void AdjustBet(int delta)
    {
        m_SelectedBet += delta;
        ClampSelectedBet();
        UpdateSelectionText();
    }

    protected void ClampSelectedBet()
    {
        int minBet = 10;
        int maxBet = 5000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.diceBetting)
        {
            minBet = OECasinoConfig.Instance.diceBetting.minBet;
            maxBet = OECasinoConfig.Instance.diceBetting.maxBet;
        }

        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
    }

    protected void UpdateSelectionText()
    {
        if (m_SelectedBetText) m_SelectedBetText.SetText(m_SelectedBet.ToString());
        if (m_ExactTargetText) m_ExactTargetText.SetText(m_ExactTarget.ToString());

        string label = GetSelectedBetLabel();
        string payout = GetSelectedPayoutText();
        if (m_SelectedBetDescription) m_SelectedBetDescription.SetText("SELECTED: " + label + "   |   PAYS " + payout);
    }

    protected void UpdateExactControlsVisibility()
    {
        bool showExactControls = (m_SelectedBetType == OE_DICE_BET_EXACT);
        if (m_ExactControlsPanel)
        {
            m_ExactControlsPanel.Show(showExactControls);
            return;
        }

        // Fallback for older layouts.
        if (m_ExactTargetLabel) m_ExactTargetLabel.Show(showExactControls);
        if (m_ExactMinus) m_ExactMinus.Show(showExactControls);
        if (m_ExactTargetText) m_ExactTargetText.Show(showExactControls);
        if (m_ExactPlus) m_ExactPlus.Show(showExactControls);
    }

    protected string GetSelectedBetLabel()
    {
        if (m_SelectedBetType == OE_DICE_BET_LOW) return "LOW (2-6)";
        if (m_SelectedBetType == OE_DICE_BET_HIGH) return "HIGH (8-12)";
        if (m_SelectedBetType == OE_DICE_BET_ODD) return "ODD TOTAL";
        if (m_SelectedBetType == OE_DICE_BET_EVEN) return "EVEN TOTAL";
        if (m_SelectedBetType == OE_DICE_BET_DOUBLES) return "DOUBLES";
        if (m_SelectedBetType == OE_DICE_BET_EXACT) return "EXACT TOTAL " + m_ExactTarget.ToString();
        return "LOW (2-6)";
    }

    protected string GetSelectedPayoutText()
    {
        if (!OECasinoConfig.Instance || !OECasinoConfig.Instance.diceBetting) return "?";
        OEDiceBettingSettings s = OECasinoConfig.Instance.diceBetting;
        float value = 1.0;

        if (m_SelectedBetType == OE_DICE_BET_LOW || m_SelectedBetType == OE_DICE_BET_HIGH) value = s.highLowPayout;
        else if (m_SelectedBetType == OE_DICE_BET_ODD || m_SelectedBetType == OE_DICE_BET_EVEN) value = s.oddEvenPayout;
        else if (m_SelectedBetType == OE_DICE_BET_DOUBLES) value = s.doublesPayout;
        else if (m_SelectedBetType == OE_DICE_BET_EXACT)
        {
            if (m_ExactTarget == 2 || m_ExactTarget == 12) value = s.exact2or12Payout;
            else if (m_ExactTarget == 3 || m_ExactTarget == 11) value = s.exact3or11Payout;
            else if (m_ExactTarget == 4 || m_ExactTarget == 10) value = s.exact4or10Payout;
            else if (m_ExactTarget == 5 || m_ExactTarget == 9) value = s.exact5or9Payout;
            else if (m_ExactTarget == 6 || m_ExactTarget == 8) value = s.exact6or8Payout;
            else if (m_ExactTarget == 7) value = s.exact7Payout;
        }

        return value.ToString() + "x";
    }

    protected void ClearDice()
    {
        if (m_Die1) m_Die1.SetText("-");
        if (m_Die2) m_Die2.SetText("-");
        if (m_Total) m_Total.SetText("-");
        if (m_RollSummary) m_RollSummary.SetText("-");
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DICE_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendRoll()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        Param4<string, int, int, int> req = new Param4<string, int, int, int>(m_StationId, m_SelectedBet, m_SelectedBetType, m_ExactTarget);
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DICE_ROLL, req, true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
