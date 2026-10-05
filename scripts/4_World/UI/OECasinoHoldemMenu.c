class OECasinoHoldemMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Pot;
    protected TextWidget m_PlayerTotal;
    protected TextWidget m_DealerTotal;
    protected TextWidget m_Result;
    protected TextWidget m_Street;
    protected TextWidget m_StatusPrompt;
    protected TextWidget m_Message;
    protected TextWidget m_DealerAction;
    protected TextWidget m_DealerHand;
    protected TextWidget m_PlayerHand;
    protected TextWidget m_SelectedBetValue;
    protected TextWidget m_ActionInfo;
    protected TextWidget m_SelectedRaiseValue;

    protected ref array<ImageWidget> m_DealerCards;
    protected ref array<ImageWidget> m_CommunityCards;
    protected ref array<ImageWidget> m_PlayerCards;

    protected ref array<Widget> m_DealerFlipBars;
    protected ref array<Widget> m_CommunityFlipBars;
    protected ref array<Widget> m_PlayerFlipBars;
    protected ref OECasinoHoldemNetState m_PendingRevealState;
    protected bool m_RevealInProgress;
    protected int m_RevealStage;
    protected int m_PendingAction;
    protected int m_LastCommunityCount;
    protected int m_LastDealerHiddenCards;

    protected Widget m_StartPanel;
    protected Widget m_ActionPanel;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Deal;
    protected ButtonWidget m_Fold;
    protected ButtonWidget m_Primary;
    protected ButtonWidget m_Raise;
    protected ButtonWidget m_RaiseMinus1000;
    protected ButtonWidget m_RaiseMinus100;
    protected ButtonWidget m_RaiseMinus10;
    protected ButtonWidget m_RaisePlus10;
    protected ButtonWidget m_RaisePlus100;
    protected ButtonWidget m_RaisePlus1000;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected int m_SelectedBet = 100;
    protected int m_PrimaryAction = OE_HOLDEM_ACTION_CHECK;
    protected int m_SelectedRaise = 100;
    protected int m_MinRaise = 10;
    protected int m_MaxRaise = 300;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/CasinoHoldem.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Pot = TextWidget.Cast(m_Root.FindAnyWidget("potValue"));
        m_PlayerTotal = TextWidget.Cast(m_Root.FindAnyWidget("playerTotalValue"));
        m_DealerTotal = TextWidget.Cast(m_Root.FindAnyWidget("dealerTotalValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Street = TextWidget.Cast(m_Root.FindAnyWidget("streetValue"));
        m_StatusPrompt = TextWidget.Cast(m_Root.FindAnyWidget("statusPrompt"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_DealerAction = TextWidget.Cast(m_Root.FindAnyWidget("dealerActionValue"));
        m_DealerHand = TextWidget.Cast(m_Root.FindAnyWidget("dealerHandLabel"));
        m_PlayerHand = TextWidget.Cast(m_Root.FindAnyWidget("playerHandLabel"));
        m_SelectedBetValue = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_ActionInfo = TextWidget.Cast(m_Root.FindAnyWidget("actionInfo"));
        m_SelectedRaiseValue = TextWidget.Cast(m_Root.FindAnyWidget("selectedRaiseValue"));

        m_DealerCards = new array<ImageWidget>;
        m_DealerCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("dealerCard1")));
        m_DealerCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("dealerCard2")));

        m_CommunityCards = new array<ImageWidget>;
        m_CommunityCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("communityCard1")));
        m_CommunityCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("communityCard2")));
        m_CommunityCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("communityCard3")));
        m_CommunityCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("communityCard4")));
        m_CommunityCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("communityCard5")));

        m_PlayerCards = new array<ImageWidget>;
        m_PlayerCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("playerCard1")));
        m_PlayerCards.Insert(ImageWidget.Cast(m_Root.FindAnyWidget("playerCard2")));

        m_DealerFlipBars = new array<Widget>;
        m_DealerFlipBars.Insert(m_Root.FindAnyWidget("dealerFlipBar1"));
        m_DealerFlipBars.Insert(m_Root.FindAnyWidget("dealerFlipBar2"));

        m_CommunityFlipBars = new array<Widget>;
        m_CommunityFlipBars.Insert(m_Root.FindAnyWidget("communityFlipBar1"));
        m_CommunityFlipBars.Insert(m_Root.FindAnyWidget("communityFlipBar2"));
        m_CommunityFlipBars.Insert(m_Root.FindAnyWidget("communityFlipBar3"));
        m_CommunityFlipBars.Insert(m_Root.FindAnyWidget("communityFlipBar4"));
        m_CommunityFlipBars.Insert(m_Root.FindAnyWidget("communityFlipBar5"));

        m_PlayerFlipBars = new array<Widget>;
        m_PlayerFlipBars.Insert(m_Root.FindAnyWidget("playerFlipBar1"));
        m_PlayerFlipBars.Insert(m_Root.FindAnyWidget("playerFlipBar2"));

        m_StartPanel = m_Root.FindAnyWidget("startPanel");
        m_ActionPanel = m_Root.FindAnyWidget("actionPanel");

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Deal = ButtonWidget.Cast(m_Root.FindAnyWidget("dealButton"));
        m_Fold = ButtonWidget.Cast(m_Root.FindAnyWidget("foldButton"));
        m_Primary = ButtonWidget.Cast(m_Root.FindAnyWidget("primaryButton"));
        m_Raise = ButtonWidget.Cast(m_Root.FindAnyWidget("raiseButton"));
        m_RaiseMinus1000 = ButtonWidget.Cast(m_Root.FindAnyWidget("raiseMinus1000"));
        m_RaiseMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("raiseMinus100"));
        m_RaiseMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("raiseMinus10"));
        m_RaisePlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("raisePlus10"));
        m_RaisePlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("raisePlus100"));
        m_RaisePlus1000 = ButtonWidget.Cast(m_Root.FindAnyWidget("raisePlus1000"));

        ClampSelectedBet();
        UpdateSelectedBetText();
        m_RevealInProgress = false;
        m_RevealStage = 0;
        m_PendingAction = 0;
        m_LastCommunityCount = 0;
        m_LastDealerHiddenCards = 2;
        HideFlipBars();
        ShowIdleCards();
        ShowStartControls();
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
        CancelRevealCallbacks();
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        m_RevealStage = 0;
        m_PendingAction = 0;
        m_WaitingForResponse = false;
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

        if (m_WaitingForResponse) return true;

        if (w == m_BetMinus100) { AdjustBet(-100); return true; }
        if (w == m_BetMinus10) { AdjustBet(-10); return true; }
        if (w == m_BetPlus10) { AdjustBet(10); return true; }
        if (w == m_BetPlus100) { AdjustBet(100); return true; }

        if (w == m_RaiseMinus1000) { AdjustRaise(-1000); return true; }
        if (w == m_RaiseMinus100) { AdjustRaise(-100); return true; }
        if (w == m_RaiseMinus10) { AdjustRaise(-10); return true; }
        if (w == m_RaisePlus10) { AdjustRaise(10); return true; }
        if (w == m_RaisePlus100) { AdjustRaise(100); return true; }
        if (w == m_RaisePlus1000) { AdjustRaise(1000); return true; }

        if (w == m_Deal)
        {
            m_WaitingForResponse = true;
            if (m_Result) m_Result.SetText("DEALING");
            if (m_Message) m_Message.SetText("Locking your opening bet and dealing private cards...");
            m_SelectedRaise = m_SelectedBet;
            m_PendingAction = 10;
            SendStart();
            return true;
        }

        if (w == m_Fold)
        {
            m_WaitingForResponse = true;
            if (m_Result) m_Result.SetText("FOLDING");
            m_PendingAction = OE_HOLDEM_ACTION_FOLD;
            SendAction(OE_HOLDEM_ACTION_FOLD, 0);
            return true;
        }

        if (w == m_Primary)
        {
            m_WaitingForResponse = true;
            if (m_PrimaryAction == OE_HOLDEM_ACTION_CALL)
            {
                if (m_Result) m_Result.SetText("CALLING");
                m_PendingAction = OE_HOLDEM_ACTION_CALL;
                SendAction(OE_HOLDEM_ACTION_CALL, 0);
            }
            else
            {
                if (m_Result) m_Result.SetText("CHECKING");
                m_PendingAction = OE_HOLDEM_ACTION_CHECK;
                SendAction(OE_HOLDEM_ACTION_CHECK, 0);
            }
            return true;
        }

        if (w == m_Raise)
        {
            m_WaitingForResponse = true;
            if (m_Result) m_Result.SetText("BETTING");
            m_PendingAction = OE_HOLDEM_ACTION_RAISE;
            SendAction(OE_HOLDEM_ACTION_RAISE, m_SelectedRaise);
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OECasinoHoldemNetState state)
    {
        if (!state) return;

        int requestedAction = m_PendingAction;
        m_PendingAction = 0;

        // Only animate fresh actions from this open menu. A sync/recovery state
        // renders immediately so reconnecting/reopening never waits on effects.
        if (requestedAction == 10 && state.status == OE_HOLDEM_STATUS_PLAYER_TURN && CountEncodedCards(state.playerCards) >= 2)
        {
            BeginCardReveal(state, 1); // Private cards.
            return;
        }

        if (requestedAction == OE_HOLDEM_ACTION_CHECK || requestedAction == OE_HOLDEM_ACTION_CALL || requestedAction == OE_HOLDEM_ACTION_RAISE)
        {
            int communityCount = CountEncodedCards(state.communityCards);
            if (state.status == OE_HOLDEM_STATUS_PLAYER_TURN && communityCount > m_LastCommunityCount)
            {
                if (communityCount >= 5 && m_LastCommunityCount < 5) BeginCardReveal(state, 4); // River.
                else if (communityCount >= 4 && m_LastCommunityCount < 4) BeginCardReveal(state, 3); // Turn.
                else if (communityCount >= 3 && m_LastCommunityCount < 3) BeginCardReveal(state, 2); // Flop.
                else RenderStateImmediate(state);
                return;
            }

            bool showdown = state.status == OE_HOLDEM_STATUS_PLAYER_WIN || state.status == OE_HOLDEM_STATUS_DEALER_WIN || state.status == OE_HOLDEM_STATUS_PUSH;
            if (showdown && state.dealerHiddenCards == 0 && CountEncodedCards(state.dealerCards) >= 2 && m_LastDealerHiddenCards > 0)
            {
                BeginCardReveal(state, 5); // Dealer showdown cards.
                return;
            }
        }

        RenderStateImmediate(state);
    }

    protected void RenderStateImmediate(OECasinoHoldemNetState state)
    {
        if (!state) return;

        CancelRevealCallbacks();
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        m_RevealStage = 0;
        HideFlipBars();
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Pot) m_Pot.SetText(state.pot.ToString());
        if (m_PlayerTotal) m_PlayerTotal.SetText(state.playerContribution.ToString());
        if (m_DealerTotal) m_DealerTotal.SetText(state.dealerContribution.ToString());
        if (m_Message) m_Message.SetText(state.message);
        if (m_Street) m_Street.SetText(GetStreetName(state.street));
        if (m_DealerAction)
        {
            if (state.dealerLastAction == "") m_DealerAction.SetText("WAITING FOR HAND");
            else m_DealerAction.SetText(state.dealerLastAction);
        }

        RememberState(state);

        if (state.status == OE_HOLDEM_STATUS_IDLE)
        {
            if (m_Result) m_Result.SetText("-");
            if (m_Street) m_Street.SetText("-");
            if (m_StatusPrompt) m_StatusPrompt.SetText("CHOOSE A BET TO DEAL");
            if (m_DealerHand) m_DealerHand.SetText("DEALER HAND: HIDDEN");
            if (m_PlayerHand) m_PlayerHand.SetText("YOUR HAND: -");
            ShowIdleCards();
            ShowStartControls();
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_REJECTED)
        {
            if (m_Result) m_Result.SetText("BET NOT PLACED");
            if (m_StatusPrompt) m_StatusPrompt.SetText("ADJUST YOUR BET AND TRY AGAIN");
            ShowIdleCards();
            ShowStartControls();
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_ERROR)
        {
            if (m_Result) m_Result.SetText("ERROR");
            if (m_StatusPrompt) m_StatusPrompt.SetText("TABLE ERROR - CHECK MESSAGE");
            ShowStartControls();
            return;
        }

        RenderCardList(state.playerCards, m_PlayerCards, 0);
        RenderCardList(state.communityCards, m_CommunityCards, state.communityHiddenCards);
        RenderCardList(state.dealerCards, m_DealerCards, state.dealerHiddenCards);

        if (state.playerHandName != "")
        {
            if (m_PlayerHand) m_PlayerHand.SetText("YOUR HAND: " + state.playerHandName);
        }
        else
        {
            if (m_PlayerHand) m_PlayerHand.SetText("YOUR HAND: PRE-FLOP");
        }

        if (state.dealerHandName != "")
        {
            if (m_DealerHand) m_DealerHand.SetText("DEALER HAND: " + state.dealerHandName);
        }
        else
        {
            if (m_DealerHand) m_DealerHand.SetText("DEALER HAND: HIDDEN");
        }

        if (state.status == OE_HOLDEM_STATUS_PLAYER_TURN)
        {
            if (m_Result) m_Result.SetText("PLAYING");

            string prompt = GetStreetName(state.street) + " - YOUR TURN";
            if (state.amountToCall > 0)
                prompt = prompt + " - DEALER BET " + state.amountToCall.ToString() + " TO YOU";
            if (m_StatusPrompt) m_StatusPrompt.SetText(prompt);

            ShowActionControls(state);
            return;
        }

        ShowStartControls();
        if (m_Deal) m_Deal.SetText("DEAL NEW HAND");

        if (state.status == OE_HOLDEM_STATUS_PLAYER_FOLD)
        {
            // Folded hole cards are mucked immediately; do not leave the old
            // hand face-up while waiting to deal the next hand.
            RenderCardList("", m_PlayerCards, 0);
            if (m_PlayerHand) m_PlayerHand.SetText("YOUR HAND: FOLDED");
            if (m_Result) m_Result.SetText("FOLD -" + state.playerContribution.ToString());
            if (m_StatusPrompt) m_StatusPrompt.SetText("YOU FOLDED - DEALER TAKES THE POT");
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_DEALER_FOLD)
        {
            if (m_Result) m_Result.SetText("DEALER FOLD +" + state.netResult.ToString());
            if (m_StatusPrompt) m_StatusPrompt.SetText("BLUFF OR VALUE BET WORKED - DEALER FOLDS");
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_PLAYER_WIN)
        {
            if (m_Result) m_Result.SetText("WIN +" + state.netResult.ToString());
            if (m_StatusPrompt) m_StatusPrompt.SetText("SHOWDOWN - YOU WIN THE POT");
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_DEALER_WIN)
        {
            if (m_Result) m_Result.SetText("LOSS -" + state.playerContribution.ToString());
            if (m_StatusPrompt) m_StatusPrompt.SetText("SHOWDOWN - DEALER WINS");
            return;
        }

        if (state.status == OE_HOLDEM_STATUS_PUSH)
        {
            if (m_Result) m_Result.SetText("PUSH 0");
            if (m_StatusPrompt) m_StatusPrompt.SetText("SHOWDOWN PUSH - YOUR CHIPS RETURNED");
        }
    }

    protected void BeginCardReveal(OECasinoHoldemNetState state, int revealStage)
    {
        CancelRevealCallbacks();
        m_PendingRevealState = state;
        m_RevealInProgress = true;
        m_RevealStage = revealStage;
        m_WaitingForResponse = true;
        HideFlipBars();

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Pot) m_Pot.SetText(state.pot.ToString());
        if (m_PlayerTotal) m_PlayerTotal.SetText(state.playerContribution.ToString());
        if (m_DealerTotal) m_DealerTotal.SetText(state.dealerContribution.ToString());
        if (m_Street) m_Street.SetText(GetStreetName(state.street));
        if (m_Result) m_Result.SetText("DEALING");
        if (m_DealerAction)
        {
            if (state.dealerLastAction == "") m_DealerAction.SetText("DEALING");
            else m_DealerAction.SetText(state.dealerLastAction);
        }
        if (m_StartPanel) m_StartPanel.Show(false);
        if (m_ActionPanel) m_ActionPanel.Show(false);

        string revealText = "DEALING CARDS...";
        if (revealStage == 1) revealText = "DEALING YOUR PRIVATE CARDS...";
        else if (revealStage == 2) revealText = "DEALING THE FLOP...";
        else if (revealStage == 3) revealText = "DEALING THE TURN...";
        else if (revealStage == 4) revealText = "DEALING THE RIVER...";
        else if (revealStage == 5) revealText = "SHOWDOWN - DEALER REVEALS...";
        if (m_StatusPrompt) m_StatusPrompt.SetText(revealText);
        if (m_Message) m_Message.SetText(state.message);
        OECasinoAudio.PlayCardFlip();

        if (revealStage == 1)
            RenderCardListMasked(state.playerCards, m_PlayerCards, 0, 0, 2);
        else
            RenderCardList(state.playerCards, m_PlayerCards, 0);

        if (revealStage == 2)
            RenderCardListMasked(state.communityCards, m_CommunityCards, state.communityHiddenCards, 0, 3);
        else if (revealStage == 3)
            RenderCardListMasked(state.communityCards, m_CommunityCards, state.communityHiddenCards, 3, 1);
        else if (revealStage == 4)
            RenderCardListMasked(state.communityCards, m_CommunityCards, state.communityHiddenCards, 4, 1);
        else
            RenderCardList(state.communityCards, m_CommunityCards, state.communityHiddenCards);

        if (revealStage == 5)
            RenderCardListMasked(state.dealerCards, m_DealerCards, state.dealerHiddenCards, 0, 2);
        else
            RenderCardList(state.dealerCards, m_DealerCards, state.dealerHiddenCards);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RevealFlipMid, 320, false);
    }

    protected void RevealFlipMid()
    {
        if (!m_RevealInProgress || !m_PendingRevealState) return;

        HideRevealCards(m_RevealStage);
        ShowFlipBarsForStage(m_RevealStage);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishCardReveal, 460, false);
    }

    protected void FinishCardReveal()
    {
        if (!m_PendingRevealState) return;

        OECasinoHoldemNetState state = m_PendingRevealState;
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        m_RevealStage = 0;
        m_WaitingForResponse = false;
        HideFlipBars();
        RenderStateImmediate(state);
    }

    protected void HideRevealCards(int revealStage)
    {
        if (revealStage == 1)
        {
            HideImageRange(m_PlayerCards, 0, 2);
            return;
        }
        if (revealStage == 2)
        {
            HideImageRange(m_CommunityCards, 0, 3);
            return;
        }
        if (revealStage == 3)
        {
            HideImageRange(m_CommunityCards, 3, 1);
            return;
        }
        if (revealStage == 4)
        {
            HideImageRange(m_CommunityCards, 4, 1);
            return;
        }
        if (revealStage == 5)
            HideImageRange(m_DealerCards, 0, 2);
    }

    protected void HideImageRange(array<ImageWidget> widgets, int startIndex, int count)
    {
        if (!widgets) return;
        int endIndex = startIndex + count;
        for (int i = startIndex; i < endIndex && i < widgets.Count(); i++)
        {
            ImageWidget widget = widgets.Get(i);
            if (widget) widget.Show(false);
        }
    }

    protected void ShowFlipBarsForStage(int revealStage)
    {
        HideFlipBars();
        if (revealStage == 1) ShowWidgetRange(m_PlayerFlipBars, 0, 2);
        else if (revealStage == 2) ShowWidgetRange(m_CommunityFlipBars, 0, 3);
        else if (revealStage == 3) ShowWidgetRange(m_CommunityFlipBars, 3, 1);
        else if (revealStage == 4) ShowWidgetRange(m_CommunityFlipBars, 4, 1);
        else if (revealStage == 5) ShowWidgetRange(m_DealerFlipBars, 0, 2);
    }

    protected void ShowWidgetRange(array<Widget> widgets, int startIndex, int count)
    {
        if (!widgets) return;
        int endIndex = startIndex + count;
        for (int i = startIndex; i < endIndex && i < widgets.Count(); i++)
        {
            Widget widget = widgets.Get(i);
            if (widget) widget.Show(true);
        }
    }

    protected void HideFlipBars()
    {
        if (m_DealerFlipBars)
        {
            foreach (Widget dealerBar : m_DealerFlipBars)
            {
                if (dealerBar) dealerBar.Show(false);
            }
        }
        if (m_CommunityFlipBars)
        {
            foreach (Widget communityBar : m_CommunityFlipBars)
            {
                if (communityBar) communityBar.Show(false);
            }
        }
        if (m_PlayerFlipBars)
        {
            foreach (Widget playerBar : m_PlayerFlipBars)
            {
                if (playerBar) playerBar.Show(false);
            }
        }
    }

    protected void CancelRevealCallbacks()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.RevealFlipMid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishCardReveal);
    }

    protected void RememberState(OECasinoHoldemNetState state)
    {
        if (!state) return;
        if (state.status == OE_HOLDEM_STATUS_IDLE || state.status == OE_HOLDEM_STATUS_REJECTED)
        {
            m_LastCommunityCount = 0;
            m_LastDealerHiddenCards = 2;
            return;
        }
        m_LastCommunityCount = CountEncodedCards(state.communityCards);
        m_LastDealerHiddenCards = state.dealerHiddenCards;
    }

    protected int CountEncodedCards(string encoded)
    {
        if (encoded == "") return 0;
        TStringArray entries = new TStringArray;
        encoded.Split(";", entries);
        return entries.Count();
    }

    protected void ShowIdleCards()
    {
        RenderCardList("", m_DealerCards, 2);
        RenderCardList("", m_CommunityCards, 5);
        RenderCardList("", m_PlayerCards, 2);
        if (m_Pot) m_Pot.SetText("0");
        if (m_PlayerTotal) m_PlayerTotal.SetText("0");
        if (m_DealerTotal) m_DealerTotal.SetText("0");
        if (m_DealerAction) m_DealerAction.SetText("WAITING FOR HAND");
        if (m_Deal) m_Deal.SetText("DEAL HAND");
    }

    protected void ShowStartControls()
    {
        if (m_StartPanel) m_StartPanel.Show(true);
        if (m_ActionPanel) m_ActionPanel.Show(false);
        ClampSelectedBet();
        UpdateSelectedBetText();
    }

    protected void ShowActionControls(OECasinoHoldemNetState state)
    {
        if (m_StartPanel) m_StartPanel.Show(false);
        if (m_ActionPanel) m_ActionPanel.Show(true);

        m_MinRaise = state.minRaiseAmount;
        m_MaxRaise = state.maxRaiseAmount;
        if (m_MinRaise < 1) m_MinRaise = 1;
        if (m_MaxRaise < m_MinRaise) m_MaxRaise = m_MinRaise;
        ClampSelectedRaise();
        UpdateSelectedRaiseText();

        if (m_Fold) m_Fold.Enable(state.canFold);

        if (state.canCall)
        {
            m_PrimaryAction = OE_HOLDEM_ACTION_CALL;
            if (m_Primary) m_Primary.SetText("CALL " + state.amountToCall.ToString());
            if (m_Primary) m_Primary.Enable(true);
        }
        else
        {
            m_PrimaryAction = OE_HOLDEM_ACTION_CHECK;
            if (m_Primary) m_Primary.SetText("CHECK");
            if (m_Primary) m_Primary.Enable(state.canCheck);
        }

        if (m_Raise)
        {
            if (state.amountToCall > 0) m_Raise.SetText("RAISE +" + m_SelectedRaise.ToString());
            else m_Raise.SetText("BET " + m_SelectedRaise.ToString());
            m_Raise.Enable(state.canRaise);
        }

        if (m_RaiseMinus1000) m_RaiseMinus1000.Enable(state.canRaise);
        if (m_RaiseMinus100) m_RaiseMinus100.Enable(state.canRaise);
        if (m_RaiseMinus10) m_RaiseMinus10.Enable(state.canRaise);
        if (m_RaisePlus10) m_RaisePlus10.Enable(state.canRaise);
        if (m_RaisePlus100) m_RaisePlus100.Enable(state.canRaise);
        if (m_RaisePlus1000) m_RaisePlus1000.Enable(state.canRaise);

        if (m_ActionInfo)
        {
            string info;
            if (state.maxRaisesPerStreet <= 0)
                info = "NO-LIMIT  |  Raise " + m_MinRaise.ToString() + "-" + m_MaxRaise.ToString();
            else
                info = "Raise " + m_MinRaise.ToString() + "-" + m_MaxRaise.ToString() + "  |  Raises " + state.raisesThisStreet.ToString() + "/" + state.maxRaisesPerStreet.ToString();
            if (state.amountToCall > 0) info = info + "  |  To call " + state.amountToCall.ToString();
            m_ActionInfo.SetText(info);
        }
    }

    protected string GetStreetName(int street)
    {
        if (street == OE_HOLDEM_STREET_PREFLOP) return "PRE-FLOP";
        if (street == OE_HOLDEM_STREET_FLOP) return "FLOP";
        if (street == OE_HOLDEM_STREET_TURN) return "TURN";
        if (street == OE_HOLDEM_STREET_RIVER) return "RIVER";
        if (street == OE_HOLDEM_STREET_SHOWDOWN) return "SHOWDOWN";
        return "-";
    }

    protected void AdjustBet(int delta)
    {
        m_SelectedBet += delta;
        ClampSelectedBet();
        UpdateSelectedBetText();
    }

    protected void ClampSelectedBet()
    {
        int minBet = 10;
        int maxBet = 5000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.casinoHoldem)
        {
            minBet = OECasinoConfig.Instance.casinoHoldem.minBet;
            maxBet = OECasinoConfig.Instance.casinoHoldem.maxBet;
        }

        if (minBet < 1) minBet = 1;
        if (maxBet < minBet) maxBet = minBet;
        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
    }

    protected void UpdateSelectedBetText()
    {
        if (m_SelectedBetValue) m_SelectedBetValue.SetText(m_SelectedBet.ToString());
    }


    protected void AdjustRaise(int delta)
    {
        m_SelectedRaise += delta;
        ClampSelectedRaise();
        UpdateSelectedRaiseText();
    }

    protected void ClampSelectedRaise()
    {
        if (m_MinRaise < 1) m_MinRaise = 1;
        if (m_MaxRaise < m_MinRaise) m_MaxRaise = m_MinRaise;
        if (m_SelectedRaise < m_MinRaise) m_SelectedRaise = m_MinRaise;
        if (m_SelectedRaise > m_MaxRaise) m_SelectedRaise = m_MaxRaise;
    }

    protected void UpdateSelectedRaiseText()
    {
        if (m_SelectedRaiseValue) m_SelectedRaiseValue.SetText(m_SelectedRaise.ToString());
        if (m_Raise)
        {
            if (m_PrimaryAction == OE_HOLDEM_ACTION_CALL) m_Raise.SetText("RAISE +" + m_SelectedRaise.ToString());
            else m_Raise.SetText("BET " + m_SelectedRaise.ToString());
        }
    }

    protected void RenderCardList(string encoded, array<ImageWidget> widgets, int hiddenCards)
    {
        if (!widgets) return;
        foreach (ImageWidget existing : widgets)
        {
            if (existing) existing.Show(false);
        }

        TStringArray entries = new TStringArray;
        if (encoded != "") encoded.Split(";", entries);

        int slot = 0;
        foreach (string entry : entries)
        {
            if (slot >= widgets.Count()) break;
            TStringArray parts = new TStringArray;
            entry.Split(",", parts);
            if (parts.Count() < 2) continue;

            int rank = parts.Get(0).ToInt();
            int suit = parts.Get(1).ToInt();
            string texturePath = GetCardFaceTexturePath(rank, suit);
            if (texturePath == "") continue;

            ImageWidget widget = widgets.Get(slot);
            if (!widget) continue;
            widget.LoadImageFile(0, texturePath);
            widget.SetImage(0);
            widget.Show(true);
            slot++;
        }

        for (int i = 0; i < hiddenCards && slot < widgets.Count(); i++)
        {
            ImageWidget hiddenWidget = widgets.Get(slot);
            if (!hiddenWidget) continue;
            hiddenWidget.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
            hiddenWidget.SetImage(0);
            hiddenWidget.Show(true);
            slot++;
        }
    }

    protected void RenderCardListMasked(string encoded, array<ImageWidget> widgets, int hiddenCards, int maskStart, int maskCount)
    {
        if (!widgets) return;
        foreach (ImageWidget existing : widgets)
        {
            if (existing) existing.Show(false);
        }

        TStringArray entries = new TStringArray;
        if (encoded != "") encoded.Split(";", entries);

        int slot = 0;
        int maskEnd = maskStart + maskCount;
        foreach (string entry : entries)
        {
            if (slot >= widgets.Count()) break;
            TStringArray parts = new TStringArray;
            entry.Split(",", parts);
            if (parts.Count() < 2) continue;

            ImageWidget widget = widgets.Get(slot);
            if (!widget) continue;

            bool masked = slot >= maskStart && slot < maskEnd;
            string texturePath = "OperationExileCasino/data/cards/oe_card_back.edds";
            if (!masked)
            {
                int rank = parts.Get(0).ToInt();
                int suit = parts.Get(1).ToInt();
                texturePath = GetCardFaceTexturePath(rank, suit);
                if (texturePath == "") texturePath = "OperationExileCasino/data/cards/oe_card_back.edds";
            }

            widget.LoadImageFile(0, texturePath);
            widget.SetImage(0);
            widget.Show(true);
            slot++;
        }

        for (int i = 0; i < hiddenCards && slot < widgets.Count(); i++)
        {
            ImageWidget hiddenWidget = widgets.Get(slot);
            if (!hiddenWidget) continue;
            hiddenWidget.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
            hiddenWidget.SetImage(0);
            hiddenWidget.Show(true);
            slot++;
        }
    }

    protected string GetCardFaceTexturePath(int rank, int suit)
    {
        string rankToken = "";
        string suitToken = "";

        if (rank == 14) rankToken = "ace";
        else if (rank == 13) rankToken = "king";
        else if (rank == 12) rankToken = "queen";
        else if (rank == 11) rankToken = "jack";
        else if (rank >= 2 && rank <= 10) rankToken = rank.ToString();

        if (suit == OE_CARD_SUIT_HEARTS) suitToken = "hearts";
        else if (suit == OE_CARD_SUIT_DIAMONDS) suitToken = "diamonds";
        else if (suit == OE_CARD_SUIT_CLUBS) suitToken = "clubs";
        else if (suit == OE_CARD_SUIT_SPADES) suitToken = "spades";

        if (rankToken == "" || suitToken == "") return "";
        return "OperationExileCasino/data/cards/faces/" + rankToken + "_of_" + suitToken + ".edds";
    }

    protected void SendStart()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_HOLDEM_START, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void SendAction(int action, int raiseAmount)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_HOLDEM_ACTION, new Param2<int, int>(action, raiseAmount), true);
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_HOLDEM_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
