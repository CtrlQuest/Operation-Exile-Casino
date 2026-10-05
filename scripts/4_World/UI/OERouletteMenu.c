class OERouletteMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_TotalBetText;
    protected TextWidget m_Result;
    protected TextWidget m_LastSpin;
    protected TextWidget m_WinningNumber;
    protected TextWidget m_WinningColor;
    protected TextWidget m_Message;
    protected TextWidget m_ModeDescription;
    protected TextWidget m_ActiveBetsText;
    protected TextWidget m_ActiveHelpText;
    protected TextWidget m_SelectedChipText;

    protected ButtonWidget m_Close;
    protected ref array<ButtonWidget> m_NumberButtons;
    protected ButtonWidget m_Num00;

    protected ButtonWidget m_Red;
    protected ButtonWidget m_Black;
    protected ButtonWidget m_Odd;
    protected ButtonWidget m_Even;
    protected ButtonWidget m_Low;
    protected ButtonWidget m_High;
    protected ButtonWidget m_Dozen1;
    protected ButtonWidget m_Dozen2;
    protected ButtonWidget m_Dozen3;
    protected ButtonWidget m_Column1;
    protected ButtonWidget m_Column2;
    protected ButtonWidget m_Column3;

    protected ButtonWidget m_ModeStraight;
    protected ButtonWidget m_ModeSplit;
    protected ButtonWidget m_ModeStreet;
    protected ButtonWidget m_ModeCorner;
    protected ButtonWidget m_ModeSixLine;
    protected ButtonWidget m_TopLine;

    protected ButtonWidget m_ChipMinus100;
    protected ButtonWidget m_ChipMinus10;
    protected ButtonWidget m_ChipPlus10;
    protected ButtonWidget m_ChipPlus100;
    protected ButtonWidget m_Undo;
    protected ButtonWidget m_Clear;
    protected ButtonWidget m_Spin;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected int m_SelectedChip = 100;
    protected int m_CurrentMode = OE_ROULETTE_BET_STRAIGHT;
    protected int m_FirstPick = -1;
    protected int m_TotalPlaced;
    protected int m_KnownBalance;

    protected ref array<int> m_BetTypes;
    protected ref array<int> m_BetAmounts;
    protected ref array<int> m_A;
    protected ref array<int> m_B;
    protected ref array<int> m_C;
    protected ref array<int> m_D;
    protected ref array<int> m_E;
    protected ref array<int> m_F;

    // Each click is kept in a lightweight history so UNDO removes one chip
    // even when several clicks have been merged into one table position.
    protected ref array<int> m_HistoryBetTypes;
    protected ref array<int> m_HistoryAmounts;
    protected ref array<int> m_HistoryA;
    protected ref array<int> m_HistoryB;
    protected ref array<int> m_HistoryC;
    protected ref array<int> m_HistoryD;
    protected ref array<int> m_HistoryE;
    protected ref array<int> m_HistoryF;

    // Straight-number chip markers shown directly on the Roulette felt.
    protected ref array<Widget> m_NumberBadgePanels;
    protected ref array<TextWidget> m_NumberBadgeTexts;
    protected Widget m_Num00BadgePanel;
    protected TextWidget m_Num00BadgeText;

    // Position markers for Split/Street/Corner/Six-Line/Top-Line and all
    // outside bets. These are static layout widgets placed where a real
    // roulette chip would sit on the felt.
    protected ref array<string> m_TableMarkerIds;
    protected ref array<Widget> m_TableMarkerPanels;
    protected ref array<TextWidget> m_TableMarkerTexts;

    // Client-only visual spin animation. The final result has already been
    // selected and settled by the server before this animation begins.
    protected bool m_IsSpinning;
    protected float m_SpinElapsed;
    protected float m_SpinTickElapsed;
    protected ref OERouletteNetState m_PendingState;


    protected void RegisterTableBetMarker(string id)
    {
        if (!m_TableMarkerIds || !m_TableMarkerPanels || !m_TableMarkerTexts) return;

        m_TableMarkerIds.Insert(id);
        m_TableMarkerPanels.Insert(m_Root.FindAnyWidget("betMarker_" + id));
        m_TableMarkerTexts.Insert(TextWidget.Cast(m_Root.FindAnyWidget("betMarkerText_" + id)));
    }

    protected void RegisterTableBetMarkers()
    {
        // Outside/table bets.
        RegisterTableBetMarker("RED");
        RegisterTableBetMarker("BLACK");
        RegisterTableBetMarker("ODD");
        RegisterTableBetMarker("EVEN");
        RegisterTableBetMarker("LOW");
        RegisterTableBetMarker("HIGH");
        RegisterTableBetMarker("D1");
        RegisterTableBetMarker("D2");
        RegisterTableBetMarker("D3");
        RegisterTableBetMarker("C1");
        RegisterTableBetMarker("C2");
        RegisterTableBetMarker("C3");
        RegisterTableBetMarker("TOP");

        int street;
        int row;
        int a;
        int b;

        // Every legal split position.
        for (street = 0; street < 12; street++)
        {
            for (row = 0; row < 3; row++)
            {
                a = street * 3 + row + 1;

                // Vertical split within the same street: 1/2, 2/3, etc.
                if (row < 2)
                {
                    b = a + 1;
                    RegisterTableBetMarker("S" + a.ToString() + "_" + b.ToString());
                }

                // Split between neighbouring streets: 1/4, 2/5, 3/6, etc.
                if (street < 11)
                {
                    b = a + 3;
                    RegisterTableBetMarker("S" + a.ToString() + "_" + b.ToString());
                }
            }
        }

        // Streets 1-3 through 34-36.
        for (street = 0; street < 12; street++)
        {
            a = street * 3 + 1;
            RegisterTableBetMarker("T" + a.ToString());
        }

        // Corners.
        for (street = 0; street < 11; street++)
        {
            for (row = 0; row < 2; row++)
            {
                a = street * 3 + row + 1;
                RegisterTableBetMarker("K" + a.ToString());
            }
        }

        // Six-line positions.
        for (street = 0; street < 11; street++)
        {
            a = street * 3 + 1;
            RegisterTableBetMarker("L" + a.ToString());
        }
    }

    protected int FindTableMarkerIndex(string id)
    {
        if (!m_TableMarkerIds) return -1;
        int i;
        for (i = 0; i < m_TableMarkerIds.Count(); i++)
        {
            if (m_TableMarkerIds.Get(i) == id) return i;
        }
        return -1;
    }

    protected void HideAllTableBetMarkers()
    {
        if (!m_TableMarkerPanels) return;
        int i;
        for (i = 0; i < m_TableMarkerPanels.Count(); i++)
        {
            Widget panel = m_TableMarkerPanels.Get(i);
            if (panel) panel.Show(false);
        }
    }

    protected void ShowTableBetMarker(string id, int amount)
    {
        int index = FindTableMarkerIndex(id);
        if (index < 0) return;

        Widget panel = m_TableMarkerPanels.Get(index);
        TextWidget text = m_TableMarkerTexts.Get(index);
        if (panel) panel.Show(true);
        if (text) text.SetText(amount.ToString());
    }

    protected string GetMarkerIdForBet(int index)
    {
        if (index < 0 || index >= m_BetTypes.Count()) return "";

        int type = m_BetTypes.Get(index);
        int a = m_A.Get(index);
        int b = m_B.Get(index);

        if (type == OE_ROULETTE_BET_SPLIT)
            return "S" + a.ToString() + "_" + b.ToString();
        if (type == OE_ROULETTE_BET_STREET)
            return "T" + a.ToString();
        if (type == OE_ROULETTE_BET_CORNER)
            return "K" + a.ToString();
        if (type == OE_ROULETTE_BET_SIX_LINE)
            return "L" + a.ToString();
        if (type == OE_ROULETTE_BET_TOP_LINE)
            return "TOP";

        if (type == OE_ROULETTE_BET_RED) return "RED";
        if (type == OE_ROULETTE_BET_BLACK) return "BLACK";
        if (type == OE_ROULETTE_BET_ODD) return "ODD";
        if (type == OE_ROULETTE_BET_EVEN) return "EVEN";
        if (type == OE_ROULETTE_BET_LOW) return "LOW";
        if (type == OE_ROULETTE_BET_HIGH) return "HIGH";
        if (type == OE_ROULETTE_BET_DOZEN_1) return "D1";
        if (type == OE_ROULETTE_BET_DOZEN_2) return "D2";
        if (type == OE_ROULETTE_BET_DOZEN_3) return "D3";
        if (type == OE_ROULETTE_BET_COLUMN_1) return "C1";
        if (type == OE_ROULETTE_BET_COLUMN_2) return "C2";
        if (type == OE_ROULETTE_BET_COLUMN_3) return "C3";

        return "";
    }

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    protected void EnsureBetArrays()
    {
        if (!m_BetTypes) m_BetTypes = new array<int>;
        if (!m_BetAmounts) m_BetAmounts = new array<int>;
        if (!m_A) m_A = new array<int>;
        if (!m_B) m_B = new array<int>;
        if (!m_C) m_C = new array<int>;
        if (!m_D) m_D = new array<int>;
        if (!m_E) m_E = new array<int>;
        if (!m_F) m_F = new array<int>;

        if (!m_HistoryBetTypes) m_HistoryBetTypes = new array<int>;
        if (!m_HistoryAmounts) m_HistoryAmounts = new array<int>;
        if (!m_HistoryA) m_HistoryA = new array<int>;
        if (!m_HistoryB) m_HistoryB = new array<int>;
        if (!m_HistoryC) m_HistoryC = new array<int>;
        if (!m_HistoryD) m_HistoryD = new array<int>;
        if (!m_HistoryE) m_HistoryE = new array<int>;
        if (!m_HistoryF) m_HistoryF = new array<int>;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        EnsureBetArrays();
        m_NumberButtons = new array<ButtonWidget>;
        m_NumberBadgePanels = new array<Widget>;
        m_NumberBadgeTexts = new array<TextWidget>;
        m_TableMarkerIds = new array<string>;
        m_TableMarkerPanels = new array<Widget>;
        m_TableMarkerTexts = new array<TextWidget>;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/Roulette.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_TotalBetText = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_LastSpin = TextWidget.Cast(m_Root.FindAnyWidget("lastSpinValue"));
        m_WinningNumber = TextWidget.Cast(m_Root.FindAnyWidget("winningNumber"));
        m_WinningColor = TextWidget.Cast(m_Root.FindAnyWidget("winningColor"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_ModeDescription = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetDescription"));
        m_ActiveBetsText = TextWidget.Cast(m_Root.FindAnyWidget("activeBetsText"));
        m_ActiveHelpText = TextWidget.Cast(m_Root.FindAnyWidget("activeHelpText"));
        m_SelectedChipText = TextWidget.Cast(m_Root.FindAnyWidget("selectedChipValue"));

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));

        int i;
        for (i = 0; i <= 36; i++)
        {
            ButtonWidget numberButton = ButtonWidget.Cast(m_Root.FindAnyWidget("num" + i.ToString()));
            m_NumberButtons.Insert(numberButton);

            Widget badgePanel = m_Root.FindAnyWidget("chipBadge" + i.ToString());
            TextWidget badgeText = TextWidget.Cast(m_Root.FindAnyWidget("chipBadgeText" + i.ToString()));
            m_NumberBadgePanels.Insert(badgePanel);
            m_NumberBadgeTexts.Insert(badgeText);
        }
        m_Num00 = ButtonWidget.Cast(m_Root.FindAnyWidget("num00"));
        m_Num00BadgePanel = m_Root.FindAnyWidget("chipBadge00");
        m_Num00BadgeText = TextWidget.Cast(m_Root.FindAnyWidget("chipBadgeText00"));

        RegisterTableBetMarkers();

        m_Red = ButtonWidget.Cast(m_Root.FindAnyWidget("redButton"));
        m_Black = ButtonWidget.Cast(m_Root.FindAnyWidget("blackButton"));
        m_Odd = ButtonWidget.Cast(m_Root.FindAnyWidget("oddButton"));
        m_Even = ButtonWidget.Cast(m_Root.FindAnyWidget("evenButton"));
        m_Low = ButtonWidget.Cast(m_Root.FindAnyWidget("lowButton"));
        m_High = ButtonWidget.Cast(m_Root.FindAnyWidget("highButton"));
        m_Dozen1 = ButtonWidget.Cast(m_Root.FindAnyWidget("dozen1Button"));
        m_Dozen2 = ButtonWidget.Cast(m_Root.FindAnyWidget("dozen2Button"));
        m_Dozen3 = ButtonWidget.Cast(m_Root.FindAnyWidget("dozen3Button"));
        m_Column1 = ButtonWidget.Cast(m_Root.FindAnyWidget("column1Button"));
        m_Column2 = ButtonWidget.Cast(m_Root.FindAnyWidget("column2Button"));
        m_Column3 = ButtonWidget.Cast(m_Root.FindAnyWidget("column3Button"));

        m_ModeStraight = ButtonWidget.Cast(m_Root.FindAnyWidget("modeStraight"));
        m_ModeSplit = ButtonWidget.Cast(m_Root.FindAnyWidget("modeSplit"));
        m_ModeStreet = ButtonWidget.Cast(m_Root.FindAnyWidget("modeStreet"));
        m_ModeCorner = ButtonWidget.Cast(m_Root.FindAnyWidget("modeCorner"));
        m_ModeSixLine = ButtonWidget.Cast(m_Root.FindAnyWidget("modeSixLine"));
        m_TopLine = ButtonWidget.Cast(m_Root.FindAnyWidget("topLineButton"));

        m_ChipMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("chipMinus100"));
        m_ChipMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("chipMinus10"));
        m_ChipPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("chipPlus10"));
        m_ChipPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("chipPlus100"));
        m_Undo = ButtonWidget.Cast(m_Root.FindAnyWidget("undoButton"));
        m_Clear = ButtonWidget.Cast(m_Root.FindAnyWidget("clearButton"));
        m_Spin = ButtonWidget.Cast(m_Root.FindAnyWidget("spinButton"));

        ClampSelectedChip();
        UpdateModeText();
        UpdateActiveBetsText();
        RefreshBetMarkers();
        ClearResult();
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
        m_SpinElapsed = 0.0;
        m_SpinTickElapsed = 0.0;
        m_PendingState = null;
        RequestState();
    }

    override void OnHide()
    {
        bool hadSubmittedSpin = m_IsSpinning || m_WaitingForResponse;
        m_IsSpinning = false;
        m_WaitingForResponse = false;
        m_PendingState = null;
        if (hadSubmittedSpin) ClearPlacedBets(true);

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
        if (m_IsSpinning) UpdateSpinAnimation(timeslice);
        if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress()) CloseMenu();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_Close) { CloseMenu(); return true; }
        if (m_WaitingForResponse || m_IsSpinning) return true;

        if (w == m_ModeStraight) { SetMode(OE_ROULETTE_BET_STRAIGHT); return true; }
        if (w == m_ModeSplit) { SetMode(OE_ROULETTE_BET_SPLIT); return true; }
        if (w == m_ModeStreet) { SetMode(OE_ROULETTE_BET_STREET); return true; }
        if (w == m_ModeCorner) { SetMode(OE_ROULETTE_BET_CORNER); return true; }
        if (w == m_ModeSixLine) { SetMode(OE_ROULETTE_BET_SIX_LINE); return true; }
        if (w == m_TopLine) { PlaceBet(OE_ROULETTE_BET_TOP_LINE, 0, 37, 1, 2, 3, -1); return true; }

        int number = FindClickedNumber(w);
        if (number >= 0)
        {
            HandleNumberClick(number);
            return true;
        }
        if (w == m_Num00)
        {
            HandleNumberClick(37);
            return true;
        }

        if (w == m_Red) { PlaceOutside(OE_ROULETTE_BET_RED); return true; }
        if (w == m_Black) { PlaceOutside(OE_ROULETTE_BET_BLACK); return true; }
        if (w == m_Odd) { PlaceOutside(OE_ROULETTE_BET_ODD); return true; }
        if (w == m_Even) { PlaceOutside(OE_ROULETTE_BET_EVEN); return true; }
        if (w == m_Low) { PlaceOutside(OE_ROULETTE_BET_LOW); return true; }
        if (w == m_High) { PlaceOutside(OE_ROULETTE_BET_HIGH); return true; }
        if (w == m_Dozen1) { PlaceOutside(OE_ROULETTE_BET_DOZEN_1); return true; }
        if (w == m_Dozen2) { PlaceOutside(OE_ROULETTE_BET_DOZEN_2); return true; }
        if (w == m_Dozen3) { PlaceOutside(OE_ROULETTE_BET_DOZEN_3); return true; }
        if (w == m_Column1) { PlaceOutside(OE_ROULETTE_BET_COLUMN_1); return true; }
        if (w == m_Column2) { PlaceOutside(OE_ROULETTE_BET_COLUMN_2); return true; }
        if (w == m_Column3) { PlaceOutside(OE_ROULETTE_BET_COLUMN_3); return true; }

        if (w == m_ChipMinus100) { AdjustChip(-100); return true; }
        if (w == m_ChipMinus10) { AdjustChip(-10); return true; }
        if (w == m_ChipPlus10) { AdjustChip(10); return true; }
        if (w == m_ChipPlus100) { AdjustChip(100); return true; }
        if (w == m_Undo) { UndoLastBet(); return true; }
        if (w == m_Clear) { ClearPlacedBets(true); return true; }

        if (w == m_Spin)
        {
            if (!m_BetTypes || m_BetTypes.Count() < 1)
            {
                SetMessage("Place at least one bet before spinning.");
                return true;
            }

            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Bets locked. Waiting for the server...");
            if (m_Result) m_Result.SetText("SPINNING");
            if (m_WinningNumber) m_WinningNumber.SetText("-");
            if (m_WinningColor) m_WinningColor.SetText("BALL SPINNING");
            SendSpin();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    protected int FindClickedNumber(Widget w)
    {
        if (!m_NumberButtons) return -1;
        int i;
        for (i = 0; i < m_NumberButtons.Count(); i++)
        {
            if (w == m_NumberButtons.Get(i)) return i;
        }
        return -1;
    }

    protected void SetMode(int mode)
    {
        m_CurrentMode = mode;
        m_FirstPick = -1;
        UpdateModeText();
        SetModeHelp();
    }

    protected void SetModeHelp()
    {
        if (!m_ActiveHelpText) return;
        if (m_CurrentMode == OE_ROULETTE_BET_STRAIGHT) m_ActiveHelpText.SetText("STRAIGHT: click one number. Outside bets can be added at any time.");
        else if (m_CurrentMode == OE_ROULETTE_BET_SPLIT) m_ActiveHelpText.SetText("SPLIT: click two adjacent numbers.");
        else if (m_CurrentMode == OE_ROULETTE_BET_STREET) m_ActiveHelpText.SetText("STREET: click any number in the 3-number street.");
        else if (m_CurrentMode == OE_ROULETTE_BET_CORNER) m_ActiveHelpText.SetText("CORNER: click two diagonal corner numbers of the four-number block.");
        else if (m_CurrentMode == OE_ROULETTE_BET_SIX_LINE) m_ActiveHelpText.SetText("SIX LINE: click a number in the LEFT street of the two streets.");
    }

    protected void HandleNumberClick(int number)
    {
        if (number == 0 || number == 37)
        {
            if (m_CurrentMode != OE_ROULETTE_BET_STRAIGHT)
            {
                SetMessage("0 and 00 are straight-up bets here. Use TOP LINE for 0/00/1/2/3.");
                return;
            }
            PlaceBet(OE_ROULETTE_BET_STRAIGHT, number, -1, -1, -1, -1, -1);
            return;
        }

        if (m_CurrentMode == OE_ROULETTE_BET_STRAIGHT)
        {
            PlaceBet(OE_ROULETTE_BET_STRAIGHT, number, -1, -1, -1, -1, -1);
            return;
        }

        if (m_CurrentMode == OE_ROULETTE_BET_SPLIT)
        {
            HandleSplitPick(number);
            return;
        }

        if (m_CurrentMode == OE_ROULETTE_BET_STREET)
        {
            int start = GetStreetStart(number);
            PlaceBet(OE_ROULETTE_BET_STREET, start, start + 1, start + 2, -1, -1, -1);
            return;
        }

        if (m_CurrentMode == OE_ROULETTE_BET_CORNER)
        {
            HandleCornerPick(number);
            return;
        }

        if (m_CurrentMode == OE_ROULETTE_BET_SIX_LINE)
        {
            int street = GetStreetIndex(number);
            if (street >= 11)
            {
                SetMessage("For SIX LINE, click a number in the left street of the two streets.");
                return;
            }
            int sixStart = street * 3 + 1;
            PlaceBet(OE_ROULETTE_BET_SIX_LINE, sixStart, sixStart + 1, sixStart + 2, sixStart + 3, sixStart + 4, sixStart + 5);
            return;
        }
    }

    protected void HandleSplitPick(int number)
    {
        if (m_FirstPick < 1)
        {
            m_FirstPick = number;
            SetMessage("SPLIT: first number " + number.ToString() + ". Choose an adjacent second number.");
            return;
        }

        int first = m_FirstPick;
        m_FirstPick = -1;
        if (!AreSplitAdjacent(first, number))
        {
            SetMessage("Those numbers do not form a valid SPLIT. Try again.");
            return;
        }

        int a = first;
        int b = number;
        if (b < a)
        {
            int temp = a;
            a = b;
            b = temp;
        }
        PlaceBet(OE_ROULETTE_BET_SPLIT, a, b, -1, -1, -1, -1);
    }

    protected void HandleCornerPick(int number)
    {
        if (m_FirstPick < 1)
        {
            m_FirstPick = number;
            SetMessage("CORNER: first corner " + number.ToString() + ". Choose the diagonal number.");
            return;
        }

        int first = m_FirstPick;
        m_FirstPick = -1;

        int streetA = GetStreetIndex(first);
        int streetB = GetStreetIndex(number);
        int rowA = GetRowIndex(first);
        int rowB = GetRowIndex(number);
        if (AbsInt(streetA - streetB) != 1 || AbsInt(rowA - rowB) != 1)
        {
            SetMessage("Those numbers do not form a valid CORNER. Choose diagonal corners.");
            return;
        }

        int minStreet = streetA;
        if (streetB < minStreet) minStreet = streetB;
        int minRow = rowA;
        if (rowB < minRow) minRow = rowB;

        int a = minStreet * 3 + minRow + 1;
        PlaceBet(OE_ROULETTE_BET_CORNER, a, a + 1, a + 3, a + 4, -1, -1);
    }

    protected bool AreSplitAdjacent(int a, int b)
    {
        if (a < 1 || a > 36 || b < 1 || b > 36 || a == b) return false;
        int streetA = GetStreetIndex(a);
        int streetB = GetStreetIndex(b);
        int rowA = GetRowIndex(a);
        int rowB = GetRowIndex(b);
        if (streetA == streetB && AbsInt(rowA - rowB) == 1) return true;
        if (rowA == rowB && AbsInt(streetA - streetB) == 1) return true;
        return false;
    }

    protected void PlaceOutside(int betType)
    {
        PlaceBet(betType, -1, -1, -1, -1, -1, -1);
    }

    protected void PlaceBet(int betType, int a, int b, int c, int d, int e, int f)
    {
        EnsureBetArrays();

        int existing = FindExistingBetIndex(betType, a, b, c, d, e, f);
        bool isNewPosition = existing < 0;
        if (!CanPlaceChip(isNewPosition)) return;

        if (isNewPosition)
        {
            m_BetTypes.Insert(betType);
            m_BetAmounts.Insert(m_SelectedChip);
            m_A.Insert(a);
            m_B.Insert(b);
            m_C.Insert(c);
            m_D.Insert(d);
            m_E.Insert(e);
            m_F.Insert(f);
            existing = m_BetTypes.Count() - 1;
        }
        else
        {
            m_BetAmounts.Set(existing, m_BetAmounts.Get(existing) + m_SelectedChip);
        }

        // Preserve every click for one-chip-at-a-time UNDO.
        m_HistoryBetTypes.Insert(betType);
        m_HistoryAmounts.Insert(m_SelectedChip);
        m_HistoryA.Insert(a);
        m_HistoryB.Insert(b);
        m_HistoryC.Insert(c);
        m_HistoryD.Insert(d);
        m_HistoryE.Insert(e);
        m_HistoryF.Insert(f);

        m_TotalPlaced += m_SelectedChip;
        m_FirstPick = -1;

        if (m_TotalBetText) m_TotalBetText.SetText(m_TotalPlaced.ToString());
        SetMessage("Placed " + m_SelectedChip.ToString() + " on " + GetLocalBetLabel(existing) + ". Position total: " + m_BetAmounts.Get(existing).ToString() + ".");
        UpdateActiveBetsText();
        RefreshBetMarkers();
    }

    protected int FindExistingBetIndex(int betType, int a, int b, int c, int d, int e, int f)
    {
        EnsureBetArrays();

        int i;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            if (m_BetTypes.Get(i) != betType) continue;
            if (m_A.Get(i) != a) continue;
            if (m_B.Get(i) != b) continue;
            if (m_C.Get(i) != c) continue;
            if (m_D.Get(i) != d) continue;
            if (m_E.Get(i) != e) continue;
            if (m_F.Get(i) != f) continue;
            return i;
        }
        return -1;
    }

    protected bool CanPlaceChip(bool isNewPosition)
    {
        if (isNewPosition && m_BetTypes && m_BetTypes.Count() >= OE_ROULETTE_MAX_BETS_PER_SPIN)
        {
            SetMessage("Maximum number of Roulette positions reached for one spin.");
            return false;
        }

        int maxTotal = 25000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.roulette)
            maxTotal = OECasinoConfig.Instance.roulette.maxTotalBet;

        if (m_TotalPlaced + m_SelectedChip > maxTotal)
        {
            SetMessage("That would exceed the table total-bet limit of " + maxTotal.ToString() + ".");
            return false;
        }

        if (m_KnownBalance > 0 && m_TotalPlaced + m_SelectedChip > m_KnownBalance)
        {
            SetMessage("You do not have enough chips for that placement.");
            return false;
        }
        return true;
    }

    protected void UndoLastBet()
    {
        EnsureBetArrays();

        int historyCount = m_HistoryBetTypes.Count();
        if (historyCount < 1)
        {
            SetMessage("There are no placed chips to undo.");
            return;
        }

        int historyIndex = historyCount - 1;
        int betType = m_HistoryBetTypes.Get(historyIndex);
        int amount = m_HistoryAmounts.Get(historyIndex);
        int a = m_HistoryA.Get(historyIndex);
        int b = m_HistoryB.Get(historyIndex);
        int c = m_HistoryC.Get(historyIndex);
        int d = m_HistoryD.Get(historyIndex);
        int e = m_HistoryE.Get(historyIndex);
        int f = m_HistoryF.Get(historyIndex);

        int index = FindExistingBetIndex(betType, a, b, c, d, e, f);
        if (index >= 0)
        {
            int remaining = m_BetAmounts.Get(index) - amount;
            if (remaining <= 0)
            {
                m_BetTypes.Remove(index);
                m_BetAmounts.Remove(index);
                m_A.Remove(index);
                m_B.Remove(index);
                m_C.Remove(index);
                m_D.Remove(index);
                m_E.Remove(index);
                m_F.Remove(index);
            }
            else
            {
                m_BetAmounts.Set(index, remaining);
            }
        }

        m_HistoryBetTypes.Remove(historyIndex);
        m_HistoryAmounts.Remove(historyIndex);
        m_HistoryA.Remove(historyIndex);
        m_HistoryB.Remove(historyIndex);
        m_HistoryC.Remove(historyIndex);
        m_HistoryD.Remove(historyIndex);
        m_HistoryE.Remove(historyIndex);
        m_HistoryF.Remove(historyIndex);

        m_TotalPlaced -= amount;
        if (m_TotalPlaced < 0) m_TotalPlaced = 0;

        if (m_TotalBetText) m_TotalBetText.SetText(m_TotalPlaced.ToString());
        SetMessage("Last chip removed from the table.");
        UpdateActiveBetsText();
        RefreshBetMarkers();
    }

    protected void ClearPlacedBets(bool updateTop)
    {
        EnsureBetArrays();
        m_BetTypes.Clear();
        m_BetAmounts.Clear();
        m_A.Clear();
        m_B.Clear();
        m_C.Clear();
        m_D.Clear();
        m_E.Clear();
        m_F.Clear();

        m_HistoryBetTypes.Clear();
        m_HistoryAmounts.Clear();
        m_HistoryA.Clear();
        m_HistoryB.Clear();
        m_HistoryC.Clear();
        m_HistoryD.Clear();
        m_HistoryE.Clear();
        m_HistoryF.Clear();

        m_TotalPlaced = 0;
        m_FirstPick = -1;
        if (updateTop && m_TotalBetText) m_TotalBetText.SetText("0");
        UpdateActiveBetsText();
        RefreshBetMarkers();
        if (updateTop) SetMessage("All placed bets cleared.");
    }

    protected void AdjustChip(int delta)
    {
        m_SelectedChip += delta;
        ClampSelectedChip();
        UpdateModeText();
    }

    protected void ClampSelectedChip()
    {
        int minBet = 10;
        int maxBet = 5000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.roulette)
        {
            minBet = OECasinoConfig.Instance.roulette.minBet;
            maxBet = OECasinoConfig.Instance.roulette.maxBet;
        }
        if (m_SelectedChip < minBet) m_SelectedChip = minBet;
        if (m_SelectedChip > maxBet) m_SelectedChip = maxBet;
        if (m_SelectedChipText) m_SelectedChipText.SetText(m_SelectedChip.ToString());
    }

    protected void UpdateModeText()
    {
        if (m_SelectedChipText) m_SelectedChipText.SetText(m_SelectedChip.ToString());
        if (m_ModeDescription) m_ModeDescription.SetText("MODE: " + GetModeLabel() + "   |   CHIP " + m_SelectedChip.ToString() + "   |   PAYS " + GetModePayoutText());
    }

    protected string GetModeLabel()
    {
        if (m_CurrentMode == OE_ROULETTE_BET_SPLIT) return "SPLIT";
        if (m_CurrentMode == OE_ROULETTE_BET_STREET) return "STREET";
        if (m_CurrentMode == OE_ROULETTE_BET_CORNER) return "CORNER";
        if (m_CurrentMode == OE_ROULETTE_BET_SIX_LINE) return "SIX LINE";
        return "STRAIGHT";
    }

    protected string GetModePayoutText()
    {
        // Traditional casino notation shows PROFIT odds. Our config keeps
        // TOTAL RETURN multipliers for settlement, so only the display changes:
        // 36x total return = 35:1 profit, 18x = 17:1, etc.
        if (m_CurrentMode == OE_ROULETTE_BET_SPLIT) return "17:1";
        if (m_CurrentMode == OE_ROULETTE_BET_STREET) return "11:1";
        if (m_CurrentMode == OE_ROULETTE_BET_CORNER) return "8:1";
        if (m_CurrentMode == OE_ROULETTE_BET_SIX_LINE) return "5:1";
        return "35:1";
    }

    protected void UpdateActiveBetsText()
    {
        if (!m_ActiveBetsText) return;
        EnsureBetArrays();

        int count = m_BetTypes.Count();
        if (count < 1)
        {
            m_ActiveBetsText.SetText("BETS 0   |   TOTAL 0");
            if (m_ActiveHelpText) SetModeHelp();
            return;
        }

        m_ActiveBetsText.SetText("BETS " + count.ToString() + "   |   TOTAL " + m_TotalPlaced.ToString());
    }

    protected int GetStraightBetAmount(int pocket)
    {
        int i;
        int amount = 0;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            if (m_BetTypes.Get(i) == OE_ROULETTE_BET_STRAIGHT && m_A.Get(i) == pocket)
                amount += m_BetAmounts.Get(i);
        }
        return amount;
    }

    protected int GetBetTypeAmount(int betType)
    {
        int i;
        int amount = 0;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            if (m_BetTypes.Get(i) == betType)
                amount += m_BetAmounts.Get(i);
        }
        return amount;
    }

    protected bool BetCoversNumber(int index, int pocket)
    {
        if (index < 0 || index >= m_BetTypes.Count()) return false;

        int type = m_BetTypes.Get(index);
        if (type == OE_ROULETTE_BET_STRAIGHT) return m_A.Get(index) == pocket;
        if (type == OE_ROULETTE_BET_SPLIT) return m_A.Get(index) == pocket || m_B.Get(index) == pocket;
        if (type == OE_ROULETTE_BET_STREET) return m_A.Get(index) == pocket || m_B.Get(index) == pocket || m_C.Get(index) == pocket;
        if (type == OE_ROULETTE_BET_CORNER) return m_A.Get(index) == pocket || m_B.Get(index) == pocket || m_C.Get(index) == pocket || m_D.Get(index) == pocket;
        if (type == OE_ROULETTE_BET_SIX_LINE) return m_A.Get(index) == pocket || m_B.Get(index) == pocket || m_C.Get(index) == pocket || m_D.Get(index) == pocket || m_E.Get(index) == pocket || m_F.Get(index) == pocket;
        if (type == OE_ROULETTE_BET_TOP_LINE)
            return pocket == 0 || pocket == 37 || pocket == 1 || pocket == 2 || pocket == 3;
        return false;
    }

    protected bool AnyInsideBetCoversNumber(int pocket)
    {
        int i;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            int type = m_BetTypes.Get(i);
            if (type == OE_ROULETTE_BET_RED || type == OE_ROULETTE_BET_BLACK || type == OE_ROULETTE_BET_ODD || type == OE_ROULETTE_BET_EVEN || type == OE_ROULETTE_BET_LOW || type == OE_ROULETTE_BET_HIGH) continue;
            if (type == OE_ROULETTE_BET_DOZEN_1 || type == OE_ROULETTE_BET_DOZEN_2 || type == OE_ROULETTE_BET_DOZEN_3) continue;
            if (type == OE_ROULETTE_BET_COLUMN_1 || type == OE_ROULETTE_BET_COLUMN_2 || type == OE_ROULETTE_BET_COLUMN_3) continue;
            if (BetCoversNumber(i, pocket)) return true;
        }
        return false;
    }

    protected void SetNumberBaseColor(ButtonWidget button, int pocket)
    {
        if (!button) return;
        if (pocket == 0 || pocket == 37)
        {
            button.SetColor(ARGB(255, 13, 92, 31));
            return;
        }
        if (IsRedPocket(pocket))
        {
            button.SetColor(ARGB(255, 122, 13, 13));
            return;
        }
        button.SetColor(ARGB(255, 9, 9, 9));
    }

    protected void RefreshBetMarkers()
    {
        EnsureBetArrays();

        int i;
        for (i = 0; i <= 36; i++)
        {
            ButtonWidget button = m_NumberButtons.Get(i);
            SetNumberBaseColor(button, i);

            bool covered = AnyInsideBetCoversNumber(i);
            int straightAmount = GetStraightBetAmount(i);

            if (covered && button)
                button.SetColor(ARGB(255, 92, 72, 24));

            if (straightAmount > 0 && button)
                button.SetColor(ARGB(255, 178, 126, 16));

            if (i < m_NumberBadgePanels.Count())
            {
                Widget badge = m_NumberBadgePanels.Get(i);
                TextWidget badgeText = m_NumberBadgeTexts.Get(i);
                if (badge)
                {
                    bool showBadge = straightAmount > 0;
                    badge.Show(showBadge);
                    if (showBadge && badgeText) badgeText.SetText(straightAmount.ToString());
                }
            }
        }

        SetNumberBaseColor(m_Num00, 37);
        bool covered00 = AnyInsideBetCoversNumber(37);
        int straight00 = GetStraightBetAmount(37);
        if (covered00 && m_Num00) m_Num00.SetColor(ARGB(255, 92, 72, 24));
        if (straight00 > 0 && m_Num00) m_Num00.SetColor(ARGB(255, 178, 126, 16));
        if (m_Num00BadgePanel)
        {
            m_Num00BadgePanel.Show(straight00 > 0);
            if (straight00 > 0 && m_Num00BadgeText) m_Num00BadgeText.SetText(straight00.ToString());
        }

        RefreshOutsideButton(m_Red, OE_ROULETTE_BET_RED, ARGB(255, 122, 13, 13));
        RefreshOutsideButton(m_Black, OE_ROULETTE_BET_BLACK, ARGB(255, 9, 9, 9));
        RefreshOutsideButton(m_Odd, OE_ROULETTE_BET_ODD, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Even, OE_ROULETTE_BET_EVEN, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Low, OE_ROULETTE_BET_LOW, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_High, OE_ROULETTE_BET_HIGH, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Dozen1, OE_ROULETTE_BET_DOZEN_1, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Dozen2, OE_ROULETTE_BET_DOZEN_2, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Dozen3, OE_ROULETTE_BET_DOZEN_3, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Column1, OE_ROULETTE_BET_COLUMN_1, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Column2, OE_ROULETTE_BET_COLUMN_2, ARGB(255, 26, 26, 26));
        RefreshOutsideButton(m_Column3, OE_ROULETTE_BET_COLUMN_3, ARGB(255, 26, 26, 26));

        if (m_TopLine)
        {
            if (GetBetTypeAmount(OE_ROULETTE_BET_TOP_LINE) > 0)
                m_TopLine.SetColor(ARGB(255, 178, 126, 16));
            else
                m_TopLine.SetColor(ARGB(255, 77, 31, 87));
        }

        // Straight-up wagers use the number-cell badges above. All other
        // Roulette wagers use a marker placed on their actual felt position.
        HideAllTableBetMarkers();
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            if (m_BetTypes.Get(i) == OE_ROULETTE_BET_STRAIGHT) continue;

            string markerId = GetMarkerIdForBet(i);
            if (markerId != "")
                ShowTableBetMarker(markerId, m_BetAmounts.Get(i));
        }
    }

    protected void RefreshOutsideButton(ButtonWidget button, int betType, int baseColor)
    {
        if (!button) return;
        if (GetBetTypeAmount(betType) > 0)
            button.SetColor(ARGB(255, 178, 126, 16));
        else
            button.SetColor(baseColor);
    }


    protected bool DoesLocalBetWin(int index, int winningNumber)
    {
        if (index < 0 || index >= m_BetTypes.Count()) return false;

        int type = m_BetTypes.Get(index);
        int a = m_A.Get(index);
        int b = m_B.Get(index);
        int c = m_C.Get(index);
        int d = m_D.Get(index);
        int e = m_E.Get(index);
        int f = m_F.Get(index);

        if (type == OE_ROULETTE_BET_STRAIGHT) return winningNumber == a;
        if (type == OE_ROULETTE_BET_SPLIT) return winningNumber == a || winningNumber == b;
        if (type == OE_ROULETTE_BET_STREET) return winningNumber == a || winningNumber == b || winningNumber == c;
        if (type == OE_ROULETTE_BET_CORNER) return winningNumber == a || winningNumber == b || winningNumber == c || winningNumber == d;
        if (type == OE_ROULETTE_BET_SIX_LINE) return winningNumber == a || winningNumber == b || winningNumber == c || winningNumber == d || winningNumber == e || winningNumber == f;
        if (type == OE_ROULETTE_BET_TOP_LINE)
            return winningNumber == 0 || winningNumber == 37 || winningNumber == 1 || winningNumber == 2 || winningNumber == 3;

        // 0 and 00 lose the standard outside bets.
        if (winningNumber == 0 || winningNumber == 37) return false;

        if (type == OE_ROULETTE_BET_RED) return IsRedPocket(winningNumber);
        if (type == OE_ROULETTE_BET_BLACK) return !IsRedPocket(winningNumber);
        if (type == OE_ROULETTE_BET_ODD) return (winningNumber % 2) == 1;
        if (type == OE_ROULETTE_BET_EVEN) return (winningNumber % 2) == 0;
        if (type == OE_ROULETTE_BET_LOW) return winningNumber >= 1 && winningNumber <= 18;
        if (type == OE_ROULETTE_BET_HIGH) return winningNumber >= 19 && winningNumber <= 36;
        if (type == OE_ROULETTE_BET_DOZEN_1) return winningNumber >= 1 && winningNumber <= 12;
        if (type == OE_ROULETTE_BET_DOZEN_2) return winningNumber >= 13 && winningNumber <= 24;
        if (type == OE_ROULETTE_BET_DOZEN_3) return winningNumber >= 25 && winningNumber <= 36;
        if (type == OE_ROULETTE_BET_COLUMN_1) return (winningNumber % 3) == 1;
        if (type == OE_ROULETTE_BET_COLUMN_2) return (winningNumber % 3) == 2;
        if (type == OE_ROULETTE_BET_COLUMN_3) return (winningNumber % 3) == 0;

        return false;
    }

    protected void ResetBetVisualsForResult()
    {
        int i;
        for (i = 0; i <= 36; i++)
        {
            if (i < m_NumberButtons.Count())
                SetNumberBaseColor(m_NumberButtons.Get(i), i);

            if (i < m_NumberBadgePanels.Count())
            {
                Widget badge = m_NumberBadgePanels.Get(i);
                if (badge) badge.Show(false);
            }
        }

        SetNumberBaseColor(m_Num00, 37);
        if (m_Num00BadgePanel) m_Num00BadgePanel.Show(false);

        if (m_Red) m_Red.SetColor(ARGB(255, 122, 13, 13));
        if (m_Black) m_Black.SetColor(ARGB(255, 9, 9, 9));
        if (m_Odd) m_Odd.SetColor(ARGB(255, 26, 26, 26));
        if (m_Even) m_Even.SetColor(ARGB(255, 26, 26, 26));
        if (m_Low) m_Low.SetColor(ARGB(255, 26, 26, 26));
        if (m_High) m_High.SetColor(ARGB(255, 26, 26, 26));
        if (m_Dozen1) m_Dozen1.SetColor(ARGB(255, 26, 26, 26));
        if (m_Dozen2) m_Dozen2.SetColor(ARGB(255, 26, 26, 26));
        if (m_Dozen3) m_Dozen3.SetColor(ARGB(255, 26, 26, 26));
        if (m_Column1) m_Column1.SetColor(ARGB(255, 26, 26, 26));
        if (m_Column2) m_Column2.SetColor(ARGB(255, 26, 26, 26));
        if (m_Column3) m_Column3.SetColor(ARGB(255, 26, 26, 26));
        if (m_TopLine) m_TopLine.SetColor(ARGB(255, 77, 31, 87));

        HideAllTableBetMarkers();
    }

    protected void HighlightNumberGold(int pocket)
    {
        if (pocket == 37)
        {
            if (m_Num00) m_Num00.SetColor(ARGB(255, 210, 156, 20));
            return;
        }

        if (pocket >= 0 && pocket <= 36 && pocket < m_NumberButtons.Count())
        {
            ButtonWidget button = m_NumberButtons.Get(pocket);
            if (button) button.SetColor(ARGB(255, 210, 156, 20));
        }
    }

    protected void HighlightCoveredNumbersGold(int index)
    {
        if (index < 0 || index >= m_BetTypes.Count()) return;

        int type = m_BetTypes.Get(index);

        if (type == OE_ROULETTE_BET_SPLIT)
        {
            HighlightNumberGold(m_A.Get(index));
            HighlightNumberGold(m_B.Get(index));
        }
        else if (type == OE_ROULETTE_BET_STREET)
        {
            HighlightNumberGold(m_A.Get(index));
            HighlightNumberGold(m_B.Get(index));
            HighlightNumberGold(m_C.Get(index));
        }
        else if (type == OE_ROULETTE_BET_CORNER)
        {
            HighlightNumberGold(m_A.Get(index));
            HighlightNumberGold(m_B.Get(index));
            HighlightNumberGold(m_C.Get(index));
            HighlightNumberGold(m_D.Get(index));
        }
        else if (type == OE_ROULETTE_BET_SIX_LINE)
        {
            HighlightNumberGold(m_A.Get(index));
            HighlightNumberGold(m_B.Get(index));
            HighlightNumberGold(m_C.Get(index));
            HighlightNumberGold(m_D.Get(index));
            HighlightNumberGold(m_E.Get(index));
            HighlightNumberGold(m_F.Get(index));
        }
        else if (type == OE_ROULETTE_BET_TOP_LINE)
        {
            HighlightNumberGold(0);
            HighlightNumberGold(37);
            HighlightNumberGold(1);
            HighlightNumberGold(2);
            HighlightNumberGold(3);
        }
    }

    protected void HighlightWinningOutsideButton(int betType)
    {
        int gold = ARGB(255, 210, 156, 20);

        if (betType == OE_ROULETTE_BET_RED && m_Red) m_Red.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_BLACK && m_Black) m_Black.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_ODD && m_Odd) m_Odd.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_EVEN && m_Even) m_Even.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_LOW && m_Low) m_Low.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_HIGH && m_High) m_High.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_DOZEN_1 && m_Dozen1) m_Dozen1.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_DOZEN_2 && m_Dozen2) m_Dozen2.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_DOZEN_3 && m_Dozen3) m_Dozen3.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_COLUMN_1 && m_Column1) m_Column1.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_COLUMN_2 && m_Column2) m_Column2.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_COLUMN_3 && m_Column3) m_Column3.SetColor(gold);
        else if (betType == OE_ROULETTE_BET_TOP_LINE && m_TopLine) m_TopLine.SetColor(gold);
    }

    protected void ShowWinningBetHighlights(int winningNumber)
    {
        ResetBetVisualsForResult();

        int i;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            if (!DoesLocalBetWin(i, winningNumber)) continue;

            int type = m_BetTypes.Get(i);
            int amount = m_BetAmounts.Get(i);

            if (type == OE_ROULETTE_BET_STRAIGHT)
            {
                int pocket = m_A.Get(i);
                HighlightNumberGold(pocket);

                if (pocket == 37)
                {
                    if (m_Num00BadgePanel) m_Num00BadgePanel.Show(true);
                    if (m_Num00BadgeText) m_Num00BadgeText.SetText(amount.ToString());
                }
                else if (pocket >= 0 && pocket < m_NumberBadgePanels.Count())
                {
                    Widget badge = m_NumberBadgePanels.Get(pocket);
                    TextWidget badgeText = m_NumberBadgeTexts.Get(pocket);
                    if (badge) badge.Show(true);
                    if (badgeText) badgeText.SetText(amount.ToString());
                }
            }
            else
            {
                string markerId = GetMarkerIdForBet(i);
                if (markerId != "")
                    ShowTableBetMarker(markerId, amount);

                HighlightCoveredNumbersGold(i);
                HighlightWinningOutsideButton(type);
            }
        }
    }

    protected void ClearBetDataAfterResult()
    {
        EnsureBetArrays();

        m_BetTypes.Clear();
        m_BetAmounts.Clear();
        m_A.Clear();
        m_B.Clear();
        m_C.Clear();
        m_D.Clear();
        m_E.Clear();
        m_F.Clear();

        m_HistoryBetTypes.Clear();
        m_HistoryAmounts.Clear();
        m_HistoryA.Clear();
        m_HistoryB.Clear();
        m_HistoryC.Clear();
        m_HistoryD.Clear();
        m_HistoryE.Clear();
        m_HistoryF.Clear();

        m_TotalPlaced = 0;
        m_FirstPick = -1;
        UpdateActiveBetsText();
    }

    protected string GetLocalBetLabel(int index)
    {
        if (!m_BetTypes || index < 0 || index >= m_BetTypes.Count()) return "BET";
        int type = m_BetTypes.Get(index);
        int a = m_A.Get(index);
        int b = m_B.Get(index);
        int c = m_C.Get(index);
        int d = m_D.Get(index);
        int f = m_F.Get(index);

        if (type == OE_ROULETTE_BET_STRAIGHT) return "#" + FormatPocket(a);
        if (type == OE_ROULETTE_BET_SPLIT) return "SPLIT " + a.ToString() + "/" + b.ToString();
        if (type == OE_ROULETTE_BET_STREET) return "STREET " + a.ToString() + "-" + c.ToString();
        if (type == OE_ROULETTE_BET_CORNER) return "CORNER " + a.ToString() + "-" + d.ToString();
        if (type == OE_ROULETTE_BET_SIX_LINE) return "6-LINE " + a.ToString() + "-" + f.ToString();
        if (type == OE_ROULETTE_BET_TOP_LINE) return "TOP LINE";
        if (type == OE_ROULETTE_BET_RED) return "RED";
        if (type == OE_ROULETTE_BET_BLACK) return "BLACK";
        if (type == OE_ROULETTE_BET_ODD) return "ODD";
        if (type == OE_ROULETTE_BET_EVEN) return "EVEN";
        if (type == OE_ROULETTE_BET_LOW) return "1-18";
        if (type == OE_ROULETTE_BET_HIGH) return "19-36";
        if (type == OE_ROULETTE_BET_DOZEN_1) return "1ST 12";
        if (type == OE_ROULETTE_BET_DOZEN_2) return "2ND 12";
        if (type == OE_ROULETTE_BET_DOZEN_3) return "3RD 12";
        if (type == OE_ROULETTE_BET_COLUMN_1) return "COL 1";
        if (type == OE_ROULETTE_BET_COLUMN_2) return "COL 2";
        if (type == OE_ROULETTE_BET_COLUMN_3) return "COL 3";
        return "BET";
    }

    void ApplyState(OERouletteNetState state)
    {
        if (!state) return;
        m_KnownBalance = state.balance;

        if (state.status == OE_ROULETTE_STATUS_IDLE)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            m_PendingState = null;

            if (m_Balance) m_Balance.SetText(state.balance.ToString());
            if (m_TotalBetText)
            {
                if (m_TotalPlaced > 0) m_TotalBetText.SetText(m_TotalPlaced.ToString());
                else m_TotalBetText.SetText("0");
            }
            if (m_Result) m_Result.SetText("-");
            if (m_LastSpin) m_LastSpin.SetText("-");
            if (m_Message) m_Message.SetText(state.message);
            ClearResult();
            UpdateActiveBetsText();
            return;
        }

        if (state.status == OE_ROULETTE_STATUS_ERROR)
        {
            m_WaitingForResponse = false;
            m_IsSpinning = false;
            m_PendingState = null;
            if (m_Balance) m_Balance.SetText(state.balance.ToString());
            if (m_Result) m_Result.SetText("ERROR");
            if (m_Message) m_Message.SetText(state.message);
            return;
        }

        BeginSpinAnimation(state);
    }

    protected void BeginSpinAnimation(OERouletteNetState state)
    {
        m_PendingState = state;
        m_WaitingForResponse = true;
        m_IsSpinning = true;
        m_SpinElapsed = 0.0;
        m_SpinTickElapsed = 0.0;

        if (m_TotalBetText) m_TotalBetText.SetText(state.bet.ToString());
        if (m_Result) m_Result.SetText("SPINNING");
        if (m_Message) m_Message.SetText("Ball spinning... " + state.betCount.ToString() + " bets locked.");

        // Start the trimmed wheel audio first, then start the visual preview in
        // the same frame. The animation duration is tied to the exact clip
        // length so the final pocket lands as the sound ends.
        OECasinoAudio.PlayRouletteWheel();
        ShowSpinPreview(Math.RandomInt(0, 38));
    }

    protected void UpdateSpinAnimation(float timeslice)
    {
        if (!m_PendingState)
        {
            m_IsSpinning = false;
            m_WaitingForResponse = false;
            return;
        }

        m_SpinElapsed += timeslice;
        m_SpinTickElapsed += timeslice;

        // DayZ's UI sound playback finishes slightly ahead of the nominal OGG
        // duration in-game. End the visual preview 0.75 s earlier so the ball
        // lands on the final server result at the audible stop rather than
        // continuing through an extra slow-down beat.
        float spinDuration = OECasinoAudio.GetRouletteWheelDuration() - 0.75;
        if (spinDuration < 0.5) spinDuration = 0.5;
        float progress = m_SpinElapsed / spinDuration;

        // Fast early pockets, then progressively slow with the wheel audio.
        float interval = 0.055;
        if (progress >= 0.55) interval = 0.09;
        if (progress >= 0.72) interval = 0.15;
        if (progress >= 0.86) interval = 0.24;
        if (progress >= 0.95) interval = 0.36;

        if (m_SpinTickElapsed >= interval)
        {
            m_SpinTickElapsed = 0.0;
            ShowSpinPreview(Math.RandomInt(0, 38));
        }

        if (m_SpinElapsed >= spinDuration)
        {
            m_IsSpinning = false;
            m_WaitingForResponse = false;
            ShowFinalState(m_PendingState);
            m_PendingState = null;
        }
    }

    protected void ShowSpinPreview(int pocket)
    {
        if (m_WinningNumber) m_WinningNumber.SetText(FormatPocket(pocket));
        if (m_WinningColor) m_WinningColor.SetText(GetPocketColor(pocket));
    }

    protected void ShowFinalState(OERouletteNetState state)
    {
        if (!state) return;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_TotalBetText) m_TotalBetText.SetText(state.bet.ToString());
        if (m_WinningNumber) m_WinningNumber.SetText(FormatPocket(state.winningNumber));
        if (m_WinningColor) m_WinningColor.SetText(state.winningColor);
        if (m_LastSpin) m_LastSpin.SetText(FormatPocket(state.winningNumber) + " " + state.winningColor);

        // Keep the player-facing result short. The detailed settlement
        // breakdown is still retained server-side in play logs.
        if (m_Message) m_Message.SetText(state.message);

        if (state.status == OE_ROULETTE_STATUS_WIN)
        {
            if (m_Result) m_Result.SetText("WIN +" + state.netResult.ToString());
        }
        else if (state.status == OE_ROULETTE_STATUS_PUSH)
        {
            if (m_Result) m_Result.SetText("BREAK EVEN");
        }
        else
        {
            int lostAmount = state.netResult;
            if (lostAmount < 0) lostAmount = -lostAmount;
            if (m_Result) m_Result.SetText("LOSS -" + lostAmount.ToString());
        }

        // Remove losing markers and leave only the winning table positions
        // highlighted in gold. Then clear the underlying wager data so the
        // next spin starts fresh.
        ShowWinningBetHighlights(state.winningNumber);
        ClearBetDataAfterResult();
    }

    protected void SetMessage(string text)
    {
        if (m_Message) m_Message.SetText(text);
    }

    protected int GetStreetStart(int number)
    {
        return GetStreetIndex(number) * 3 + 1;
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

    protected string FormatPocket(int number)
    {
        if (number == 37) return "00";
        return number.ToString();
    }

    protected string GetPocketColor(int number)
    {
        if (number == 0 || number == 37) return "GREEN";
        if (IsRedPocket(number)) return "RED";
        return "BLACK";
    }

    protected bool IsRedPocket(int number)
    {
        if (number == 1 || number == 3 || number == 5 || number == 7 || number == 9) return true;
        if (number == 12 || number == 14 || number == 16 || number == 18) return true;
        if (number == 19 || number == 21 || number == 23 || number == 25 || number == 27) return true;
        if (number == 30 || number == 32 || number == 34 || number == 36) return true;
        return false;
    }

    protected void ClearResult()
    {
        if (m_WinningNumber) m_WinningNumber.SetText("-");
        if (m_WinningColor) m_WinningColor.SetText("WAITING FOR SPIN");
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_ROULETTE_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendSpin()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;

        OERouletteSpinRequest req = new OERouletteSpinRequest();
        req.stationId = m_StationId;

        int i;
        for (i = 0; i < m_BetTypes.Count(); i++)
        {
            req.betTypes.Insert(m_BetTypes.Get(i));
            req.amounts.Insert(m_BetAmounts.Get(i));
            req.a.Insert(m_A.Get(i));
            req.b.Insert(m_B.Get(i));
            req.c.Insert(m_C.Get(i));
            req.d.Insert(m_D.Get(i));
            req.e.Insert(m_E.Get(i));
            req.f.Insert(m_F.Get(i));
        }

        GetGame().RPCSingleParam(player, OE_CASINO_RPC_ROULETTE_SPIN, new Param1<ref OERouletteSpinRequest>(req), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
