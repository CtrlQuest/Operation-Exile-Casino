class OEHighCardWarMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Stake;
    protected TextWidget m_Result;
    protected TextWidget m_Message;
    protected TextWidget m_SelectedBetValue;
    protected TextWidget m_DealerRoundLabel;
    protected TextWidget m_PlayerRoundLabel;

    protected ImageWidget m_DealerCard;
    protected ImageWidget m_PlayerCard;
    protected ImageWidget m_DealerWarCard;
    protected ImageWidget m_PlayerWarCard;

    protected Widget m_DealerFlipBar;
    protected Widget m_PlayerFlipBar;
    protected Widget m_DealerWarFlipBar;
    protected Widget m_PlayerWarFlipBar;

    protected ref OEHighCardWarNetState m_PendingRevealState;
    protected bool m_RevealInProgress;
    protected int m_PendingAction;

    protected Widget m_StartPanel;
    protected Widget m_ActionPanel;
    protected Widget m_WarCardLabels;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Deal;
    protected ButtonWidget m_Surrender;
    protected ButtonWidget m_GoToWar;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected int m_SelectedBet = 100;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/HighCardWar.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Stake = TextWidget.Cast(m_Root.FindAnyWidget("stakeValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_SelectedBetValue = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_DealerRoundLabel = TextWidget.Cast(m_Root.FindAnyWidget("dealerRoundLabel"));
        m_PlayerRoundLabel = TextWidget.Cast(m_Root.FindAnyWidget("playerRoundLabel"));

        m_DealerCard = ImageWidget.Cast(m_Root.FindAnyWidget("dealerCard"));
        m_PlayerCard = ImageWidget.Cast(m_Root.FindAnyWidget("playerCard"));
        m_DealerWarCard = ImageWidget.Cast(m_Root.FindAnyWidget("dealerWarCard"));
        m_PlayerWarCard = ImageWidget.Cast(m_Root.FindAnyWidget("playerWarCard"));

        m_DealerFlipBar = m_Root.FindAnyWidget("dealerFlipBar");
        m_PlayerFlipBar = m_Root.FindAnyWidget("playerFlipBar");
        m_DealerWarFlipBar = m_Root.FindAnyWidget("dealerWarFlipBar");
        m_PlayerWarFlipBar = m_Root.FindAnyWidget("playerWarFlipBar");

        m_StartPanel = m_Root.FindAnyWidget("startPanel");
        m_ActionPanel = m_Root.FindAnyWidget("actionPanel");
        m_WarCardLabels = m_Root.FindAnyWidget("warCardLabels");

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Deal = ButtonWidget.Cast(m_Root.FindAnyWidget("dealButton"));
        m_Surrender = ButtonWidget.Cast(m_Root.FindAnyWidget("surrenderButton"));
        m_GoToWar = ButtonWidget.Cast(m_Root.FindAnyWidget("goToWarButton"));

        ClampSelectedBet();
        UpdateSelectedBetText();
        m_RevealInProgress = false;
        m_PendingAction = 0;
        HideFlipBars();
        ShowIdleCardBacks();
        ShowIdleControls();
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

        if (w == m_Deal)
        {
            m_WaitingForResponse = true;
            m_PendingAction = 1;
            SetControlsEnabled(false);
            PrepareInitialDealVisuals();
            SendStart();
            return true;
        }

        if (w == m_Surrender)
        {
            m_WaitingForResponse = true;
            m_PendingAction = 2;
            SetControlsEnabled(false);
            if (m_Message) m_Message.SetText("Surrendering half the wager...");
            SendAction(OE_WAR_ACTION_SURRENDER);
            return true;
        }

        if (w == m_GoToWar)
        {
            m_WaitingForResponse = true;
            m_PendingAction = 3;
            SetControlsEnabled(false);
            PrepareWarDealVisuals();
            SendAction(OE_WAR_ACTION_GO_TO_WAR);
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OEHighCardWarNetState state)
    {
        if (!state) return;

        int requestedAction = m_PendingAction;
        m_PendingAction = 0;

        // Idle syncs, errors, recovery syncs and surrender responses render
        // immediately. Fresh DEAL / GO TO WAR responses get the flip reveal.
        if (state.status == OE_WAR_STATUS_IDLE || state.status == OE_WAR_STATUS_ERROR)
        {
            m_WaitingForResponse = false;
            RenderStateImmediate(state);
            return;
        }

        if (requestedAction == 1 && state.playerRank >= 2 && state.dealerRank >= 2)
        {
            BeginInitialReveal(state);
            return;
        }

        if (requestedAction == 3 && state.warRound && state.warPlayerRank >= 2 && state.warDealerRank >= 2)
        {
            BeginWarReveal(state);
            return;
        }

        m_WaitingForResponse = false;
        RenderStateImmediate(state);
    }

    protected void RenderStateImmediate(OEHighCardWarNetState state)
    {
        if (!state) return;

        CancelRevealCallbacks();
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        HideFlipBars();

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Stake) m_Stake.SetText(state.totalStake.ToString());
        if (m_Message) m_Message.SetText(state.message);

        if (state.status == OE_WAR_STATUS_IDLE)
        {
            ShowIdleCardBacks();
            UpdateResultText(state);
            ShowIdleControls();
            SetControlsEnabled(true);
            return;
        }

        if (state.status == OE_WAR_STATUS_ERROR)
        {
            // Keep the table looking ready after a rejected action.
            if (!m_DealerCard || !m_DealerCard.IsVisible())
                ShowIdleCardBacks();
            UpdateResultText(state);
            ShowIdleControls();
            SetControlsEnabled(true);
            return;
        }

        RenderCard(state.playerRank, state.playerSuit, m_PlayerCard);
        RenderCard(state.dealerRank, state.dealerSuit, m_DealerCard);

        if (state.warRound)
        {
            RenderCard(state.warPlayerRank, state.warPlayerSuit, m_PlayerWarCard);
            RenderCard(state.warDealerRank, state.warDealerSuit, m_DealerWarCard);
            if (m_WarCardLabels) m_WarCardLabels.Show(true);
            if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER - WAR CARD");
            if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU - WAR CARD");
        }
        else
        {
            HideWarCards();
            if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER");
            if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU");
        }

        UpdateResultText(state);

        if (state.status == OE_WAR_STATUS_TIE)
        {
            if (m_StartPanel) m_StartPanel.Show(false);
            if (m_ActionPanel) m_ActionPanel.Show(true);
            if (m_Surrender) m_Surrender.Enable(state.canSurrender);
            if (m_GoToWar) m_GoToWar.Enable(state.canGoToWar);
            SetBetControlsEnabled(false);
            return;
        }

        ShowIdleControls();
        SetControlsEnabled(true);
    }

    protected void BeginInitialReveal(OEHighCardWarNetState state)
    {
        CancelRevealCallbacks();
        m_PendingRevealState = state;
        m_RevealInProgress = true;
        m_WaitingForResponse = true;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Stake) m_Stake.SetText(state.totalStake.ToString());
        if (m_Result) m_Result.SetText("DEALING");
        if (m_Message) m_Message.SetText("Cards dealt... revealing.");
        if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER");
        if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU");

        HideWarCards();
        HideFlipBars();
        ShowCardBack(m_DealerCard);
        ShowCardBack(m_PlayerCard);
        OECasinoAudio.PlayCardFlip();

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.InitialFlipMid, 320, false);
    }

    protected void InitialFlipMid()
    {
        if (!m_RevealInProgress || !m_PendingRevealState) return;

        if (m_DealerCard) m_DealerCard.Show(false);
        if (m_PlayerCard) m_PlayerCard.Show(false);
        if (m_DealerFlipBar) m_DealerFlipBar.Show(true);
        if (m_PlayerFlipBar) m_PlayerFlipBar.Show(true);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishInitialReveal, 460, false);
    }

    protected void FinishInitialReveal()
    {
        if (!m_PendingRevealState) return;

        OEHighCardWarNetState state = m_PendingRevealState;
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        m_WaitingForResponse = false;

        HideFlipBars();
        RenderStateImmediate(state);
    }

    protected void BeginWarReveal(OEHighCardWarNetState state)
    {
        CancelRevealCallbacks();
        m_PendingRevealState = state;
        m_RevealInProgress = true;
        m_WaitingForResponse = true;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Stake) m_Stake.SetText(state.totalStake.ToString());
        if (m_Result) m_Result.SetText("WAR");
        if (m_Message) m_Message.SetText("WAR cards dealt... revealing.");
        if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER - WAR CARD");
        if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU - WAR CARD");
        if (m_WarCardLabels) m_WarCardLabels.Show(true);

        HideFlipBars();
        ShowCardBack(m_DealerWarCard);
        ShowCardBack(m_PlayerWarCard);
        OECasinoAudio.PlayCardFlip();

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.WarFlipMid, 320, false);
    }

    protected void WarFlipMid()
    {
        if (!m_RevealInProgress || !m_PendingRevealState) return;

        if (m_DealerWarCard) m_DealerWarCard.Show(false);
        if (m_PlayerWarCard) m_PlayerWarCard.Show(false);
        if (m_DealerWarFlipBar) m_DealerWarFlipBar.Show(true);
        if (m_PlayerWarFlipBar) m_PlayerWarFlipBar.Show(true);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishWarReveal, 460, false);
    }

    protected void FinishWarReveal()
    {
        if (!m_PendingRevealState) return;

        OEHighCardWarNetState state = m_PendingRevealState;
        m_PendingRevealState = null;
        m_RevealInProgress = false;
        m_WaitingForResponse = false;

        HideFlipBars();
        RenderStateImmediate(state);
    }

    protected void UpdateResultText(OEHighCardWarNetState state)
    {
        if (!m_Result || !state) return;

        if (state.status == OE_WAR_STATUS_IDLE)
        {
            m_Result.SetText("-");
            return;
        }
        if (state.status == OE_WAR_STATUS_TIE)
        {
            m_Result.SetText("TIE - DECIDE");
            return;
        }
        if (state.status == OE_WAR_STATUS_PLAYER_WIN)
        {
            m_Result.SetText("WIN +" + state.netResult.ToString());
            return;
        }
        if (state.status == OE_WAR_STATUS_DEALER_WIN)
        {
            int loss = 0 - state.netResult;
            m_Result.SetText("LOSS -" + loss.ToString());
            return;
        }
        if (state.status == OE_WAR_STATUS_SURRENDER)
        {
            int surrenderLoss = 0 - state.netResult;
            m_Result.SetText("SURRENDER -" + surrenderLoss.ToString());
            return;
        }
        if (state.status == OE_WAR_STATUS_WAR_WIN)
        {
            if (state.netResult > 0)
                m_Result.SetText("WAR WIN +" + state.netResult.ToString());
            else
                m_Result.SetText("WAR PUSH 0");
            return;
        }
        if (state.status == OE_WAR_STATUS_WAR_LOSS)
        {
            int warLoss = 0 - state.netResult;
            m_Result.SetText("WAR LOSS -" + warLoss.ToString());
            return;
        }
        if (state.status == OE_WAR_STATUS_ERROR)
        {
            m_Result.SetText("ERROR");
            return;
        }
        m_Result.SetText("-");
    }

    protected void ShowIdleControls()
    {
        if (m_StartPanel) m_StartPanel.Show(true);
        if (m_ActionPanel) m_ActionPanel.Show(false);
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
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.highCardWar)
        {
            minBet = OECasinoConfig.Instance.highCardWar.minBet;
            maxBet = OECasinoConfig.Instance.highCardWar.maxBet;
        }
        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
    }

    protected void UpdateSelectedBetText()
    {
        if (m_SelectedBetValue) m_SelectedBetValue.SetText(m_SelectedBet.ToString());
    }

    protected void PrepareInitialDealVisuals()
    {
        HideFlipBars();
        HideWarCards();
        ShowCardBack(m_DealerCard);
        ShowCardBack(m_PlayerCard);

        if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER");
        if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU");
        if (m_Result) m_Result.SetText("DEALING");
        if (m_Message) m_Message.SetText("Dealing cards...");
    }

    protected void PrepareWarDealVisuals()
    {
        HideFlipBars();

        if (m_WarCardLabels) m_WarCardLabels.Show(true);
        if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER - WAR CARD");
        if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU - WAR CARD");

        ShowCardBack(m_DealerWarCard);
        ShowCardBack(m_PlayerWarCard);

        if (m_Result) m_Result.SetText("WAR");
        if (m_Message) m_Message.SetText("Matching the wager and dealing WAR cards...");
    }

    protected void ShowIdleCardBacks()
    {
        HideFlipBars();
        HideWarCards();

        ShowCardBack(m_DealerCard);
        ShowCardBack(m_PlayerCard);

        if (m_DealerRoundLabel) m_DealerRoundLabel.SetText("DEALER");
        if (m_PlayerRoundLabel) m_PlayerRoundLabel.SetText("YOU");
    }

    protected void ShowCardBack(ImageWidget widget)
    {
        if (!widget) return;
        widget.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
        widget.SetImage(0);
        widget.Show(true);
    }

    protected void HideWarCards()
    {
        if (m_DealerWarCard) m_DealerWarCard.Show(false);
        if (m_PlayerWarCard) m_PlayerWarCard.Show(false);
        if (m_WarCardLabels) m_WarCardLabels.Show(false);
    }

    protected void HideFlipBars()
    {
        if (m_DealerFlipBar) m_DealerFlipBar.Show(false);
        if (m_PlayerFlipBar) m_PlayerFlipBar.Show(false);
        if (m_DealerWarFlipBar) m_DealerWarFlipBar.Show(false);
        if (m_PlayerWarFlipBar) m_PlayerWarFlipBar.Show(false);
    }

    protected void CancelRevealCallbacks()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.InitialFlipMid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishInitialReveal);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.WarFlipMid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishWarReveal);
    }

    protected void SetBetControlsEnabled(bool enabled)
    {
        if (m_BetMinus100) m_BetMinus100.Enable(enabled);
        if (m_BetMinus10) m_BetMinus10.Enable(enabled);
        if (m_BetPlus10) m_BetPlus10.Enable(enabled);
        if (m_BetPlus100) m_BetPlus100.Enable(enabled);
        if (m_Deal) m_Deal.Enable(enabled);
    }

    protected void SetControlsEnabled(bool enabled)
    {
        SetBetControlsEnabled(enabled);
        if (m_Surrender) m_Surrender.Enable(enabled);
        if (m_GoToWar) m_GoToWar.Enable(enabled);
    }

    protected void RenderCard(int rank, int suit, ImageWidget widget)
    {
        if (!widget) return;
        if (rank < 2 || rank > 14)
        {
            widget.Show(false);
            return;
        }

        string texturePath = GetCardFaceTexturePath(rank, suit);
        if (texturePath == "")
        {
            widget.Show(false);
            return;
        }

        widget.LoadImageFile(0, texturePath);
        widget.SetImage(0);
        widget.Show(true);
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

    protected void ClearCards()
    {
        if (m_DealerCard) m_DealerCard.Show(false);
        if (m_PlayerCard) m_PlayerCard.Show(false);
        HideWarCards();
        HideFlipBars();
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_WAR_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_WAR_START, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void SendAction(int action)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_WAR_ACTION, new Param1<int>(action), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
