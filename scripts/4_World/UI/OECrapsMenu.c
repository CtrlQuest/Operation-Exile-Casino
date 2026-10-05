class OECrapsMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Stake;
    protected TextWidget m_SideTotal;
    protected TextWidget m_Result;
    protected TextWidget m_LineValue;
    protected TextWidget m_PhaseValue;
    protected TextWidget m_PointValue;
    protected TextWidget m_Die1;
    protected TextWidget m_Die2;
    protected TextWidget m_Total;
    protected TextWidget m_RollNumber;
    protected TextWidget m_Message;
    protected TextWidget m_MessageSub;
    protected TextWidget m_SideSummary;
    protected TextWidget m_SelectedBetValue;
    protected TextWidget m_SelectedLineText;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_PassLine;
    protected ButtonWidget m_DontPass;
    protected ButtonWidget m_Odds;
    protected ButtonWidget m_Come;
    protected ButtonWidget m_DontCome;
    protected ButtonWidget m_Field;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Undo;
    protected ButtonWidget m_Clear;
    protected ButtonWidget m_Roll;

    protected ref array<int> m_PointNumbers;
    protected ref array<ButtonWidget> m_PlaceButtons;
    protected ref array<Widget> m_PointActivePanels;
    protected ref array<Widget> m_PlaceBadges;
    protected ref array<TextWidget> m_PlaceBadgeTexts;
    protected ref array<TextWidget> m_ComeBadgeTexts;
    protected ref array<TextWidget> m_DontComeBadgeTexts;

    protected Widget m_OddsBadge;
    protected Widget m_ComeBadge;
    protected Widget m_DontComeBadge;
    protected Widget m_FieldBadge;
    protected TextWidget m_OddsBadgeText;
    protected TextWidget m_ComeBadgeText;
    protected TextWidget m_DontComeBadgeText;
    protected TextWidget m_FieldBadgeText;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected bool m_RollRequested;
    protected bool m_RollAnimating;
    protected float m_RollTickElapsed;
    protected ref OECrapsNetState m_PendingRollState;
    protected bool m_LineBetActive;
    protected bool m_TableActive;
    protected int m_Phase;
    protected int m_SelectedBet = 100;
    protected int m_SelectedLine = OE_CRAPS_BET_PASS_LINE;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/Craps.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Stake = TextWidget.Cast(m_Root.FindAnyWidget("stakeValue"));
        m_SideTotal = TextWidget.Cast(m_Root.FindAnyWidget("sideTotalValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_LineValue = TextWidget.Cast(m_Root.FindAnyWidget("lineValue"));
        m_PhaseValue = TextWidget.Cast(m_Root.FindAnyWidget("phaseValue"));
        m_PointValue = TextWidget.Cast(m_Root.FindAnyWidget("pointValue"));
        m_Die1 = TextWidget.Cast(m_Root.FindAnyWidget("die1Value"));
        m_Die2 = TextWidget.Cast(m_Root.FindAnyWidget("die2Value"));
        m_Total = TextWidget.Cast(m_Root.FindAnyWidget("totalValue"));
        m_RollNumber = TextWidget.Cast(m_Root.FindAnyWidget("rollNumberValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageMainText"));
        m_MessageSub = TextWidget.Cast(m_Root.FindAnyWidget("messageSubText"));
        m_SideSummary = TextWidget.Cast(m_Root.FindAnyWidget("sideSummaryText"));
        m_SelectedBetValue = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_SelectedLineText = TextWidget.Cast(m_Root.FindAnyWidget("selectedLineText"));

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_PassLine = ButtonWidget.Cast(m_Root.FindAnyWidget("passLineButton"));
        m_DontPass = ButtonWidget.Cast(m_Root.FindAnyWidget("dontPassButton"));
        m_Odds = ButtonWidget.Cast(m_Root.FindAnyWidget("oddsButton"));
        m_Come = ButtonWidget.Cast(m_Root.FindAnyWidget("comeButton"));
        m_DontCome = ButtonWidget.Cast(m_Root.FindAnyWidget("dontComeButton"));
        m_Field = ButtonWidget.Cast(m_Root.FindAnyWidget("fieldButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Undo = ButtonWidget.Cast(m_Root.FindAnyWidget("undoButton"));
        m_Clear = ButtonWidget.Cast(m_Root.FindAnyWidget("clearButton"));
        m_Roll = ButtonWidget.Cast(m_Root.FindAnyWidget("rollButton"));

        m_OddsBadge = m_Root.FindAnyWidget("oddsBadge");
        m_ComeBadge = m_Root.FindAnyWidget("comeBadge");
        m_DontComeBadge = m_Root.FindAnyWidget("dontComeBadge");
        m_FieldBadge = m_Root.FindAnyWidget("fieldBadge");
        m_OddsBadgeText = TextWidget.Cast(m_Root.FindAnyWidget("oddsBadgeText"));
        m_ComeBadgeText = TextWidget.Cast(m_Root.FindAnyWidget("comeBadgeText"));
        m_DontComeBadgeText = TextWidget.Cast(m_Root.FindAnyWidget("dontComeBadgeText"));
        m_FieldBadgeText = TextWidget.Cast(m_Root.FindAnyWidget("fieldBadgeText"));

        m_PointNumbers = new array<int>;
        m_PlaceButtons = new array<ButtonWidget>;
        m_PointActivePanels = new array<Widget>;
        m_PlaceBadges = new array<Widget>;
        m_PlaceBadgeTexts = new array<TextWidget>;
        m_ComeBadgeTexts = new array<TextWidget>;
        m_DontComeBadgeTexts = new array<TextWidget>;

        RegisterPoint(4);
        RegisterPoint(5);
        RegisterPoint(6);
        RegisterPoint(8);
        RegisterPoint(9);
        RegisterPoint(10);

        m_LineBetActive = false;
        m_TableActive = false;
        m_Phase = OE_CRAPS_PHASE_COME_OUT;
        ClampSelectedBet();
        UpdateSelectionText();
        ClearRoll();
        ClearBetMarkers();
        SetIdleDisplay();
        return m_Root;
    }

    protected void RegisterPoint(int number)
    {
        m_PointNumbers.Insert(number);
        m_PlaceButtons.Insert(ButtonWidget.Cast(m_Root.FindAnyWidget("place" + number.ToString() + "Button")));
        m_PointActivePanels.Insert(m_Root.FindAnyWidget("pointActive" + number.ToString()));
        m_PlaceBadges.Insert(m_Root.FindAnyWidget("placeBadge" + number.ToString()));
        m_PlaceBadgeTexts.Insert(TextWidget.Cast(m_Root.FindAnyWidget("placeBadgeText" + number.ToString())));
        m_ComeBadgeTexts.Insert(TextWidget.Cast(m_Root.FindAnyWidget("comeBadgeText" + number.ToString())));
        m_DontComeBadgeTexts.Insert(TextWidget.Cast(m_Root.FindAnyWidget("dontComeBadgeText" + number.ToString())));
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
        if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress()) CloseMenu();
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
        if (w == m_Close) { CloseMenu(); return true; }
        if (m_WaitingForResponse) return true;

        if (!m_LineBetActive)
        {
            if (w == m_PassLine) { m_SelectedLine = OE_CRAPS_BET_PASS_LINE; UpdateSelectionText(); return true; }
            if (w == m_DontPass) { m_SelectedLine = OE_CRAPS_BET_DONT_PASS_LINE; UpdateSelectionText(); return true; }
        }

        if (w == m_BetMinus100) { AdjustBet(-100); return true; }
        if (w == m_BetMinus10) { AdjustBet(-10); return true; }
        if (w == m_BetPlus10) { AdjustBet(10); return true; }
        if (w == m_BetPlus100) { AdjustBet(100); return true; }

        if (w == m_Undo) { SendBetAction(OE_CRAPS_ACTION_UNDO, 0, 0, 0); return true; }
        if (w == m_Clear) { SendBetAction(OE_CRAPS_ACTION_CLEAR, 0, 0, 0); return true; }

        if (CanPlaceSideBet())
        {
            if (w == m_Odds) { AddSideBet(OE_CRAPS_SIDE_ODDS, 0); return true; }
            if (w == m_Come) { AddSideBet(OE_CRAPS_SIDE_COME, 0); return true; }
            if (w == m_DontCome) { AddSideBet(OE_CRAPS_SIDE_DONT_COME, 0); return true; }
            if (w == m_Field) { AddSideBet(OE_CRAPS_SIDE_FIELD, 0); return true; }

            int placeNumber = FindPlaceButton(w);
            if (placeNumber > 0) { AddSideBet(OE_CRAPS_SIDE_PLACE, placeNumber); return true; }
        }

        if (w == m_Roll)
        {
            m_WaitingForResponse = true;
            m_RollRequested = true;
            SetControlsEnabled(false);
            SetMessageText("Waiting for dice...");

            if (m_LineBetActive) SendRoll();
            else SendStart();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    protected int FindPlaceButton(Widget w)
    {
        int i;
        for (i = 0; i < m_PlaceButtons.Count(); i++)
        {
            if (w == m_PlaceButtons.Get(i)) return m_PointNumbers.Get(i);
        }
        return 0;
    }

    protected bool CanPlaceSideBet()
    {
        if (!m_LineBetActive) return false;
        if (m_Phase != OE_CRAPS_PHASE_POINT) return false;
        return true;
    }

    protected void AddSideBet(int betType, int target)
    {
        m_WaitingForResponse = true;
        SetControlsEnabled(false);
        SendBetAction(OE_CRAPS_ACTION_ADD, betType, target, m_SelectedBet);
    }

    protected void SetMessageText(string text)
    {
        if (!m_Message) return;

        if (!m_MessageSub)
        {
            m_Message.SetText(text);
            return;
        }

        TStringArray parts = new TStringArray;
        text.Split("  ", parts);

        if (parts.Count() <= 1)
        {
            m_Message.SetText(text);
            m_MessageSub.SetText("");
            return;
        }

        m_Message.SetText(parts.Get(0));

        string secondLine = "";
        for (int i = 1; i < parts.Count(); i++)
        {
            string part = parts.Get(i);
            if (part == "") continue;

            if (secondLine != "") secondLine += " | ";
            secondLine += part;
        }

        m_MessageSub.SetText(secondLine);
    }

    void ApplyState(OECrapsNetState state)
    {
        if (!state) return;

        if (m_RollRequested && state.status != OE_CRAPS_STATUS_IDLE && state.status != OE_CRAPS_STATUS_ERROR && state.status != OE_CRAPS_STATUS_REJECTED && state.die1 > 0 && state.die2 > 0)
        {
            m_RollRequested = false;
            m_RollAnimating = true;
            m_RollTickElapsed = 1.0;
            m_PendingRollState = state;
            if (m_Result) m_Result.SetText("ROLLING");
            UpdateRollingDice(0.0);
            SetMessageText("Rolling dice...");
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
        OECrapsNetState state = m_PendingRollState;
        m_PendingRollState = null;
        m_RollAnimating = false;
        RenderStateImmediate(state);
    }

    protected void RenderStateImmediate(OECrapsNetState state)
    {
        if (!state) return;
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Stake) m_Stake.SetText(state.bet.ToString());
        if (m_SideTotal) m_SideTotal.SetText(state.sideBetTotal.ToString());
        SetMessageText(state.message);
        if (m_SideSummary)
        {
            if (state.sideSummary == "") m_SideSummary.SetText(GetTableHelperText(state));
            else m_SideSummary.SetText(state.sideSummary);
        }

        m_LineBetActive = state.roundActive;
        m_TableActive = state.tableActive;
        m_Phase = state.phase;

        if (state.status == OE_CRAPS_STATUS_IDLE)
        {
            m_LineBetActive = false;
            m_TableActive = false;
            m_Phase = OE_CRAPS_PHASE_COME_OUT;
            if (m_Result) m_Result.SetText("-");
            if (m_LineValue) m_LineValue.SetText(GetLineName(m_SelectedLine));
            SetIdleDisplay();
            ClearRoll();
            ClearBetMarkers();
            UpdateSelectionText();
            SetControlsEnabled(true);
            return;
        }

        m_SelectedLine = state.lineBetType;
        if (m_LineValue)
        {
            if (state.roundActive) m_LineValue.SetText(GetLineName(state.lineBetType));
            else m_LineValue.SetText("CHOOSE LINE");
        }

        if (state.die1 > 0) m_Die1.SetText(state.die1.ToString());
        else m_Die1.SetText("-");
        if (state.die2 > 0) m_Die2.SetText(state.die2.ToString());
        else m_Die2.SetText("-");
        if (state.total > 0) m_Total.SetText(state.total.ToString());
        else m_Total.SetText("-");
        if (m_RollNumber) m_RollNumber.SetText(state.rollNumber.ToString());

        if (state.roundActive && state.phase == OE_CRAPS_PHASE_POINT)
        {
            if (m_PhaseValue) m_PhaseValue.SetText("POINT PHASE");
            if (m_PointValue) m_PointValue.SetText(state.point.ToString());
        }
        else
        {
            if (m_PhaseValue) m_PhaseValue.SetText("COME-OUT");
            if (m_PointValue) m_PointValue.SetText("OFF");
        }

        RefreshBetMarkers(state);

        if (state.status == OE_CRAPS_STATUS_ACTIVE)
        {
            if (m_Result)
            {
                if (state.lastSideNet > 0) m_Result.SetText("SIDE +" + state.lastSideNet.ToString());
                else if (state.lastSideNet < 0)
                {
                    int sideLoss = 0 - state.lastSideNet;
                    m_Result.SetText("SIDE -" + sideLoss.ToString());
                }
                else if (state.roundActive && state.phase == OE_CRAPS_PHASE_POINT) m_Result.SetText("POINT " + state.point.ToString());
                else m_Result.SetText("IN PLAY");
            }
            UpdateSelectionText();
            SetControlsEnabled(true);
            return;
        }

        if (state.status == OE_CRAPS_STATUS_REJECTED)
        {
            if (m_Result) m_Result.SetText("BET NOT PLACED");
            UpdateSelectionText();
            SetControlsEnabled(true);
            return;
        }

        if (state.status == OE_CRAPS_STATUS_ERROR)
        {
            if (m_Result) m_Result.SetText("ERROR");
            UpdateSelectionText();
            SetControlsEnabled(true);
            return;
        }

        if (state.status == OE_CRAPS_STATUS_WIN)
        {
            if (m_Result) m_Result.SetText("WIN +" + state.netResult.ToString());
        }
        else if (state.status == OE_CRAPS_STATUS_PUSH)
        {
            if (m_Result) m_Result.SetText("PUSH 0");
        }
        else if (state.status == OE_CRAPS_STATUS_LOSS)
        {
            int loss = state.netResult;
            if (loss < 0) loss = 0 - loss;
            if (m_Result) m_Result.SetText("LOSS -" + loss.ToString());
        }

        UpdateSelectionText();
        SetControlsEnabled(true);
    }

    protected string GetTableHelperText(OECrapsNetState state)
    {
        if (!state) return "Side bets unlock when a POINT is active.";

        if (state.roundActive && state.phase == OE_CRAPS_PHASE_POINT)
            return "Table bets are open. Place bets are working.";

        int placeTotal = state.place4 + state.place5 + state.place6 + state.place8 + state.place9 + state.place10;
        int travellingTotal = state.come4 + state.come5 + state.come6 + state.come8 + state.come9 + state.come10;
        travellingTotal += state.dontCome4 + state.dontCome5 + state.dontCome6 + state.dontCome8 + state.dontCome9 + state.dontCome10;

        if (placeTotal > 0 && travellingTotal > 0)
            return "COME-OUT: Place bets OFF. Existing COME / DON'T COME bets still work.";

        if (placeTotal > 0)
            return "COME-OUT: Place bets are OFF until the next POINT.";

        if (travellingTotal > 0)
            return "COME-OUT: Existing COME / DON'T COME bets remain working.";

        return "Side bets unlock when a POINT is active.";
    }

    protected void SetIdleDisplay()
    {
        if (m_PhaseValue) m_PhaseValue.SetText("COME-OUT");
        if (m_PointValue) m_PointValue.SetText("OFF");
        if (m_RollNumber) m_RollNumber.SetText("0");
        if (m_SideTotal) m_SideTotal.SetText("0");
        if (m_SideSummary) m_SideSummary.SetText("Side bets unlock when a POINT is active.");
    }

    protected void ClearRoll()
    {
        if (m_Die1) m_Die1.SetText("-");
        if (m_Die2) m_Die2.SetText("-");
        if (m_Total) m_Total.SetText("-");
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
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.craps)
        {
            if (m_LineBetActive && m_Phase == OE_CRAPS_PHASE_POINT)
            {
                minBet = OECasinoConfig.Instance.craps.sideMinBet;
                maxBet = OECasinoConfig.Instance.craps.maxSideBet;
            }
            else
            {
                minBet = OECasinoConfig.Instance.craps.minBet;
                maxBet = OECasinoConfig.Instance.craps.maxBet;
            }
        }

        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
    }

    protected void UpdateSelectionText()
    {
        ClampSelectedBet();
        if (m_SelectedBetValue) m_SelectedBetValue.SetText(m_SelectedBet.ToString());
        if (m_SelectedLineText)
        {
            if (m_LineBetActive && m_Phase == OE_CRAPS_PHASE_POINT)
                m_SelectedLineText.SetText("POINT ACTIVE  |  CHIP " + m_SelectedBet.ToString() + "  |  PLACE / COME / FIELD / ODDS");
            else if (m_LineBetActive)
                m_SelectedLineText.SetText("COME-OUT IN PLAY");
            else
                m_SelectedLineText.SetText("SELECTED: " + GetLineName(m_SelectedLine) + "  |  LINE WAGER " + m_SelectedBet.ToString());
        }
    }

    protected void SetControlsEnabled(bool enabled)
    {
        bool canChooseLine = enabled && !m_LineBetActive;
        bool canPlaceSide = enabled && CanPlaceSideBet();

        if (m_PassLine) m_PassLine.Enable(canChooseLine);
        if (m_DontPass) m_DontPass.Enable(canChooseLine);

        if (m_Odds) m_Odds.Enable(canPlaceSide);
        if (m_Come) m_Come.Enable(canPlaceSide);
        if (m_DontCome) m_DontCome.Enable(canPlaceSide);
        if (m_Field) m_Field.Enable(canPlaceSide);

        int i;
        for (i = 0; i < m_PlaceButtons.Count(); i++)
        {
            if (m_PlaceButtons.Get(i)) m_PlaceButtons.Get(i).Enable(canPlaceSide);
        }

        if (m_BetMinus100) m_BetMinus100.Enable(enabled);
        if (m_BetMinus10) m_BetMinus10.Enable(enabled);
        if (m_BetPlus10) m_BetPlus10.Enable(enabled);
        if (m_BetPlus100) m_BetPlus100.Enable(enabled);
        if (m_Undo) m_Undo.Enable(enabled && m_TableActive);
        if (m_Clear) m_Clear.Enable(enabled && m_TableActive);
        if (m_Roll) m_Roll.Enable(enabled);
    }

    protected void ClearBetMarkers()
    {
        if (m_OddsBadge) m_OddsBadge.Show(false);
        if (m_ComeBadge) m_ComeBadge.Show(false);
        if (m_DontComeBadge) m_DontComeBadge.Show(false);
        if (m_FieldBadge) m_FieldBadge.Show(false);

        int i;
        for (i = 0; i < m_PointNumbers.Count(); i++)
        {
            if (m_PointActivePanels.Get(i)) m_PointActivePanels.Get(i).Show(false);
            if (m_PlaceBadges.Get(i)) m_PlaceBadges.Get(i).Show(false);
            if (m_ComeBadgeTexts.Get(i)) m_ComeBadgeTexts.Get(i).SetText("");
            if (m_DontComeBadgeTexts.Get(i)) m_DontComeBadgeTexts.Get(i).SetText("");
        }
    }

    protected void RefreshBetMarkers(OECrapsNetState state)
    {
        SetBadge(m_OddsBadge, m_OddsBadgeText, state.oddsBet);
        SetBadge(m_ComeBadge, m_ComeBadgeText, state.comePending);
        SetBadge(m_DontComeBadge, m_DontComeBadgeText, state.dontComePending);
        SetBadge(m_FieldBadge, m_FieldBadgeText, state.fieldBet);

        int i;
        for (i = 0; i < m_PointNumbers.Count(); i++)
        {
            int number = m_PointNumbers.Get(i);
            int placeAmount = GetPlaceAmount(state, number);
            int comeAmount = GetComeAmount(state, number);
            int dontAmount = GetDontComeAmount(state, number);

            SetBadge(m_PlaceBadges.Get(i), m_PlaceBadgeTexts.Get(i), placeAmount);

            if (m_ComeBadgeTexts.Get(i))
            {
                if (comeAmount > 0) m_ComeBadgeTexts.Get(i).SetText("COME " + comeAmount.ToString());
                else m_ComeBadgeTexts.Get(i).SetText("");
            }

            if (m_DontComeBadgeTexts.Get(i))
            {
                if (dontAmount > 0) m_DontComeBadgeTexts.Get(i).SetText("DC " + dontAmount.ToString());
                else m_DontComeBadgeTexts.Get(i).SetText("");
            }

            if (m_PointActivePanels.Get(i))
            {
                if (state.roundActive && state.phase == OE_CRAPS_PHASE_POINT && state.point == number) m_PointActivePanels.Get(i).Show(true);
                else m_PointActivePanels.Get(i).Show(false);
            }
        }
    }

    protected void SetBadge(Widget panel, TextWidget text, int amount)
    {
        if (!panel || !text) return;
        if (amount > 0)
        {
            text.SetText(amount.ToString());
            panel.Show(true);
        }
        else
        {
            panel.Show(false);
        }
    }

    protected int GetPlaceAmount(OECrapsNetState state, int number)
    {
        if (number == 4) return state.place4;
        if (number == 5) return state.place5;
        if (number == 6) return state.place6;
        if (number == 8) return state.place8;
        if (number == 9) return state.place9;
        if (number == 10) return state.place10;
        return 0;
    }

    protected int GetComeAmount(OECrapsNetState state, int number)
    {
        if (number == 4) return state.come4;
        if (number == 5) return state.come5;
        if (number == 6) return state.come6;
        if (number == 8) return state.come8;
        if (number == 9) return state.come9;
        if (number == 10) return state.come10;
        return 0;
    }

    protected int GetDontComeAmount(OECrapsNetState state, int number)
    {
        if (number == 4) return state.dontCome4;
        if (number == 5) return state.dontCome5;
        if (number == 6) return state.dontCome6;
        if (number == 8) return state.dontCome8;
        if (number == 9) return state.dontCome9;
        if (number == 10) return state.dontCome10;
        return 0;
    }

    protected string GetLineName(int lineBetType)
    {
        if (lineBetType == OE_CRAPS_BET_DONT_PASS_LINE) return "DON'T PASS";
        return "PASS LINE";
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CRAPS_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        Param3<string, int, int> req = new Param3<string, int, int>(m_StationId, m_SelectedBet, m_SelectedLine);
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CRAPS_START, req, true);
    }

    protected void SendRoll()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CRAPS_ROLL, new Param1<string>(m_StationId), true);
    }

    protected void SendBetAction(int action, int betType, int target, int amount)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;

        OECrapsBetActionRequest req = new OECrapsBetActionRequest();
        req.stationId = m_StationId;
        req.action = action;
        req.betType = betType;
        req.target = target;
        req.amount = amount;

        m_WaitingForResponse = true;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CRAPS_BET_ACTION, new Param1<ref OECrapsBetActionRequest>(req), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
