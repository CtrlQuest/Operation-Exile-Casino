class OERideBusMenu : UIScriptedMenu
{
    protected Widget m_Root;

    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Cashout;
    protected TextWidget m_RoundPrompt;
    protected TextWidget m_History;
    protected TextWidget m_Message;
    protected TextWidget m_Timer;
    protected TextWidget m_TimerBar;
    protected TextWidget m_SelectedBetText;

    protected Widget m_StartPanel;
    protected Widget m_ChoicePanel;
    protected Widget m_SuitPanel;

    protected Widget m_ActiveCardSlot;
    protected Widget m_ActiveCardFace;
    protected ImageWidget m_ActiveCardFaceImage;
    protected ImageWidget m_ActiveCardBack;
    protected Widget m_ActiveCardFlipBar;
    protected TextWidget m_ActiveCardText;

    protected ButtonWidget m_Start;
    protected ButtonWidget m_Close;
    protected ButtonWidget m_Cash;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_ChoiceLeft;
    protected ButtonWidget m_ChoiceRight;
    protected ButtonWidget m_Hearts;
    protected ButtonWidget m_Clubs;
    protected ButtonWidget m_Diamonds;
    protected ButtonWidget m_Spades;

    protected string m_StationId;
    protected bool m_ActiveGame;
    protected bool m_WaitingForResponse;
    protected int m_CurrentRound;
    protected int m_SelectedBet = 100;
    protected int m_LastRevealedCount;
    protected float m_CountdownRemaining;
    protected int m_CountdownTotal = 8;
    protected bool m_CountdownActive;
    protected ref OERideBusNetState m_PendingState;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/RideBus.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Cashout = TextWidget.Cast(m_Root.FindAnyWidget("cashoutValue"));
        m_RoundPrompt = TextWidget.Cast(m_Root.FindAnyWidget("roundPrompt"));
        m_History = TextWidget.Cast(m_Root.FindAnyWidget("historyText"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_Timer = TextWidget.Cast(m_Root.FindAnyWidget("timerText"));
        m_TimerBar = TextWidget.Cast(m_Root.FindAnyWidget("timerBarText"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));

        m_StartPanel = m_Root.FindAnyWidget("startPanel");
        m_ChoicePanel = m_Root.FindAnyWidget("choicePanel");
        m_SuitPanel = m_Root.FindAnyWidget("suitPanel");

        m_ActiveCardSlot = m_Root.FindAnyWidget("activeCardSlot");
        m_ActiveCardFace = m_Root.FindAnyWidget("activeCardFace");
        m_ActiveCardFaceImage = ImageWidget.Cast(m_Root.FindAnyWidget("activeCardImage"));
        m_ActiveCardBack = ImageWidget.Cast(m_Root.FindAnyWidget("activeCardBack"));
        m_ActiveCardFlipBar = m_Root.FindAnyWidget("activeCardFlipBar");
        m_ActiveCardText = TextWidget.Cast(m_Root.FindAnyWidget("activeCardText"));

        m_Start = ButtonWidget.Cast(m_Root.FindAnyWidget("startButton"));
        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_Cash = ButtonWidget.Cast(m_Root.FindAnyWidget("cashButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_ChoiceLeft = ButtonWidget.Cast(m_Root.FindAnyWidget("choiceLeft"));
        m_ChoiceRight = ButtonWidget.Cast(m_Root.FindAnyWidget("choiceRight"));
        m_Hearts = ButtonWidget.Cast(m_Root.FindAnyWidget("heartsButton"));
        m_Clubs = ButtonWidget.Cast(m_Root.FindAnyWidget("clubsButton"));
        m_Diamonds = ButtonWidget.Cast(m_Root.FindAnyWidget("diamondsButton"));
        m_Spades = ButtonWidget.Cast(m_Root.FindAnyWidget("spadesButton"));

        LoadCardBack();
        ClampSelectedBet();
        ShowHiddenCard();
        ShowIdleControls();
        return m_Root;
    }

    protected void LoadCardBack()
    {
        if (!m_ActiveCardBack) return;
        m_ActiveCardBack.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
        m_ActiveCardBack.SetImage(0);
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
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ShowFlipBar);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.RevealPendingCard);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishReveal);
        m_CountdownActive = false;

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
        {
            CloseMenu();
            return;
        }

        if (m_CountdownActive)
        {
            m_CountdownRemaining -= timeslice;
            if (m_CountdownRemaining <= 0)
            {
                m_CountdownRemaining = 0;
                m_CountdownActive = false;
                if (m_Timer) m_Timer.SetText("0 SECONDS");
                UpdateTimerBar(0);
                HideDecisionControls();
                if (m_Message) m_Message.SetText("Time expired - waiting for server result...");
            }
            else
            {
                int displaySeconds = Math.Ceil(m_CountdownRemaining);
                if (m_Timer) m_Timer.SetText(displaySeconds.ToString() + " SECONDS");
                UpdateTimerBar(m_CountdownRemaining);
            }
        }
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

        if (w == m_Start)
        {
            m_WaitingForResponse = true;
            if (m_StartPanel) m_StartPanel.Show(false);
            if (m_RoundPrompt) m_RoundPrompt.SetText("Dealing...");
            if (m_Message) m_Message.SetText("Your card is being dealt.");
            SendStart(m_SelectedBet);
            return true;
        }

        if (w == m_Cash)
        {
            m_WaitingForResponse = true;
            HideDecisionControls();
            if (m_Message) m_Message.SetText("Collecting chips...");
            SendCashOut();
            return true;
        }

        if (w == m_ChoiceLeft)
        {
            if (m_CurrentRound == OE_RTB_ROUND_COLOUR) SendRoundChoice(OE_RTB_CHOICE_RED);
            else if (m_CurrentRound == OE_RTB_ROUND_HIGH_LOW) SendRoundChoice(OE_RTB_CHOICE_HIGHER);
            else if (m_CurrentRound == OE_RTB_ROUND_IN_OUT) SendRoundChoice(OE_RTB_CHOICE_INSIDE);
            return true;
        }

        if (w == m_ChoiceRight)
        {
            if (m_CurrentRound == OE_RTB_ROUND_COLOUR) SendRoundChoice(OE_RTB_CHOICE_BLACK);
            else if (m_CurrentRound == OE_RTB_ROUND_HIGH_LOW) SendRoundChoice(OE_RTB_CHOICE_LOWER);
            else if (m_CurrentRound == OE_RTB_ROUND_IN_OUT) SendRoundChoice(OE_RTB_CHOICE_OUTSIDE);
            return true;
        }

        if (w == m_Hearts) { SendRoundChoice(OE_RTB_CHOICE_HEARTS); return true; }
        if (w == m_Clubs) { SendRoundChoice(OE_RTB_CHOICE_CLUBS); return true; }
        if (w == m_Diamonds) { SendRoundChoice(OE_RTB_CHOICE_DIAMONDS); return true; }
        if (w == m_Spades) { SendRoundChoice(OE_RTB_CHOICE_SPADES); return true; }

        return super.OnClick(w, x, y, button);
    }

    protected void SendRoundChoice(int choice)
    {
        if (!m_ActiveGame) return;
        m_WaitingForResponse = true;
        m_CountdownActive = false;
        HideDecisionControls();
        if (m_Message) m_Message.SetText("Revealing card...");
        if (m_RoundPrompt) m_RoundPrompt.SetText("Revealing...");
        SendChoice(choice);
    }

    void ApplyState(OERideBusNetState state)
    {
        if (!state) return;

        m_WaitingForResponse = false;
        int revealed = CountRevealedCards(state);

        if (revealed > m_LastRevealedCount)
        {
            m_PendingState = state;
            m_CountdownActive = false;
            HideDecisionControls();
            ShowHiddenCard();
            if (m_RoundPrompt) m_RoundPrompt.SetText("REVEALING CARD...");
            if (m_Message) m_Message.SetText(state.message);
            SetHistoryText(state, revealed - 1);
            OECasinoAudio.PlayCardFlip();
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ShowFlipBar);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.RevealPendingCard);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishReveal);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ShowFlipBar, 320, false);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RevealPendingCard, 780, false);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishReveal, 1050, false);
            return;
        }

        RenderState(state);
    }


    protected void ShowFlipBar()
    {
        if (!m_PendingState) return;
        if (m_ActiveCardBack) m_ActiveCardBack.Show(false);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(false);
        if (m_ActiveCardFlipBar) m_ActiveCardFlipBar.Show(true);
    }

    protected void RevealPendingCard()
    {
        if (m_ActiveCardFlipBar) m_ActiveCardFlipBar.Show(false);
        if (!m_PendingState) return;
        int revealed = CountRevealedCards(m_PendingState);
        if (revealed <= 0) return;
        ShowRevealedCard(GetCardByIndex(m_PendingState, revealed), GetCardRankByIndex(m_PendingState, revealed), GetCardSuitByIndex(m_PendingState, revealed));
        if (m_RoundPrompt) m_RoundPrompt.SetText("CARD REVEALED");
    }

    protected void FinishReveal()
    {
        if (!m_PendingState) return;
        OERideBusNetState state = m_PendingState;
        m_PendingState = null;
        RenderState(state);
    }

    protected void RenderState(OERideBusNetState state)
    {
        if (!state) return;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Cashout) m_Cashout.SetText(state.cashoutValue.ToString());
        if (m_Message) m_Message.SetText(state.message);

        m_CurrentRound = state.round;
        m_LastRevealedCount = CountRevealedCards(state);
        m_CountdownActive = false;
        if (m_Timer) m_Timer.SetText("");
        if (m_TimerBar) m_TimerBar.SetText("");

        if (state.status == OE_RTB_STATUS_PLAYING)
        {
            m_ActiveGame = true;
            ShowHiddenCard();
            SetHistoryText(state, m_LastRevealedCount);
            ShowPlayingControls(state);
            return;
        }

        m_ActiveGame = false;
        ShowFinishedOrIdleCard(state);
        ShowIdleControls();
    }

    protected void ShowPlayingControls(OERideBusNetState state)
    {
        if (m_StartPanel) m_StartPanel.Show(false);
        if (m_ChoicePanel) m_ChoicePanel.Show(false);
        if (m_SuitPanel) m_SuitPanel.Show(false);

        if (m_RoundPrompt) m_RoundPrompt.SetText(GetClientRoundPrompt(state.round));
        if (m_Message) m_Message.SetText("");

        if (state.round == OE_RTB_ROUND_SUIT)
        {
            if (m_SuitPanel) m_SuitPanel.Show(true);
        }
        else
        {
            ConfigureChoiceButtons(state.round);
            if (m_ChoicePanel) m_ChoicePanel.Show(true);
        }

        bool allowCollect = state.cashoutValue > 0;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.rideBus)
            allowCollect = allowCollect && OECasinoConfig.Instance.rideBus.allowCashOut;

        if (m_Cash)
        {
            m_Cash.Show(allowCollect);
            if (allowCollect) m_Cash.SetText("FORFEIT & COLLECT " + state.cashoutValue.ToString());
        }

        if (state.decisionSeconds > 0)
        {
            m_CountdownRemaining = state.decisionSeconds;
            m_CountdownTotal = state.decisionSeconds;
            if (OECasinoConfig.Instance && OECasinoConfig.Instance.rideBus && OECasinoConfig.Instance.rideBus.choiceSeconds > 0)
                m_CountdownTotal = OECasinoConfig.Instance.rideBus.choiceSeconds;
            if (m_CountdownTotal < state.decisionSeconds) m_CountdownTotal = state.decisionSeconds;
            m_CountdownActive = true;
            if (m_Timer) m_Timer.SetText(state.decisionSeconds.ToString() + " SECONDS");
            UpdateTimerBar(m_CountdownRemaining);
        }
    }

    protected string GetClientRoundPrompt(int round)
    {
        if (round == OE_RTB_ROUND_COLOUR) return "ROUND 1 - RED OR BLACK?";
        if (round == OE_RTB_ROUND_HIGH_LOW) return "ROUND 2 - HIGHER OR LOWER?";
        if (round == OE_RTB_ROUND_IN_OUT) return "ROUND 3 - INSIDE OR OUTSIDE?";
        if (round == OE_RTB_ROUND_SUIT) return "ROUND 4 - PICK THE SUIT";
        return "RIDE THE BUS";
    }

    protected void ConfigureChoiceButtons(int round)
    {
        if (!m_ChoiceLeft || !m_ChoiceRight) return;

        if (round == OE_RTB_ROUND_COLOUR)
        {
            m_ChoiceLeft.SetText("RED");
            m_ChoiceRight.SetText("BLACK");
        }
        else if (round == OE_RTB_ROUND_HIGH_LOW)
        {
            m_ChoiceLeft.SetText("HIGHER");
            m_ChoiceRight.SetText("LOWER");
        }
        else if (round == OE_RTB_ROUND_IN_OUT)
        {
            m_ChoiceLeft.SetText("INSIDE");
            m_ChoiceRight.SetText("OUTSIDE");
        }
    }

    protected void ShowIdleControls()
    {
        m_CountdownActive = false;
        if (m_Timer) m_Timer.SetText("");
        if (m_TimerBar) m_TimerBar.SetText("");
        if (m_StartPanel) m_StartPanel.Show(true);
        if (m_ChoicePanel) m_ChoicePanel.Show(false);
        if (m_SuitPanel) m_SuitPanel.Show(false);
        if (m_Cash) m_Cash.Show(false);
        if (m_Bet) m_Bet.SetText(m_SelectedBet.ToString());
        if (m_Cashout) m_Cashout.SetText("0");
        if (m_RoundPrompt) m_RoundPrompt.SetText("Set your wager and deal when ready.");
    }

    protected void HideDecisionControls()
    {
        if (m_ChoicePanel) m_ChoicePanel.Show(false);
        if (m_SuitPanel) m_SuitPanel.Show(false);
        if (m_Cash) m_Cash.Show(false);
    }

    protected void HideActiveCard()
    {
        if (m_ActiveCardSlot) m_ActiveCardSlot.Show(false);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(false);
        if (m_ActiveCardBack) m_ActiveCardBack.Show(false);
        if (m_ActiveCardFlipBar) m_ActiveCardFlipBar.Show(false);
        if (m_ActiveCardFaceImage) m_ActiveCardFaceImage.Show(false);
        if (m_ActiveCardText)
        {
            m_ActiveCardText.SetText("");
            m_ActiveCardText.Show(false);
        }
    }

    protected void ShowHiddenCard()
    {
        if (m_ActiveCardSlot) m_ActiveCardSlot.Show(true);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(false);
        if (m_ActiveCardBack) m_ActiveCardBack.Show(true);
        if (m_ActiveCardFlipBar) m_ActiveCardFlipBar.Show(false);
        if (m_ActiveCardFaceImage) m_ActiveCardFaceImage.Show(false);
        if (m_ActiveCardText) m_ActiveCardText.Show(false);
    }

    protected void ShowRevealedCard(string value, int rank, int suit)
    {
        if (value == "")
        {
            HideActiveCard();
            return;
        }

        if (m_ActiveCardSlot) m_ActiveCardSlot.Show(true);
        if (m_ActiveCardBack) m_ActiveCardBack.Show(false);
        if (m_ActiveCardFlipBar) m_ActiveCardFlipBar.Show(false);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(true);

        string texturePath = GetCardFaceTexturePath(rank, suit);
        if (texturePath != "" && m_ActiveCardFaceImage)
        {
            m_ActiveCardFaceImage.LoadImageFile(0, texturePath);
            m_ActiveCardFaceImage.SetImage(0);
            m_ActiveCardFaceImage.Show(true);
            if (m_ActiveCardText)
            {
                m_ActiveCardText.SetText("");
                m_ActiveCardText.Show(false);
            }
        }
        else if (m_ActiveCardText)
        {
            if (m_ActiveCardFaceImage) m_ActiveCardFaceImage.Show(false);
            m_ActiveCardText.SetText(value);
            m_ActiveCardText.Show(true);
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

    protected int GetCardRankByIndex(OERideBusNetState state, int index)
    {
        if (!state) return 0;
        if (index == 1) return state.card1Rank;
        if (index == 2) return state.card2Rank;
        if (index == 3) return state.card3Rank;
        if (index == 4) return state.card4Rank;
        return 0;
    }

    protected int GetCardSuitByIndex(OERideBusNetState state, int index)
    {
        if (!state) return -1;
        if (index == 1) return state.card1Suit;
        if (index == 2) return state.card2Suit;
        if (index == 3) return state.card3Suit;
        if (index == 4) return state.card4Suit;
        return -1;
    }

    protected void ShowFinishedOrIdleCard(OERideBusNetState state)
    {
        int revealed = CountRevealedCards(state);
        if (revealed > 0)
        {
            ShowRevealedCard(GetCardByIndex(state, revealed), GetCardRankByIndex(state, revealed), GetCardSuitByIndex(state, revealed));
            SetHistoryText(state, revealed - 1);
        }
        else
        {
            ShowHiddenCard();
            if (m_History) m_History.SetText("");
        }
    }

    protected void SetHistoryText(OERideBusNetState state, int count)
    {
        if (!m_History) return;
        if (!state || count <= 0)
        {
            m_History.SetText("");
            return;
        }

        string history = "PREVIOUS CARDS: ";
        for (int i = 1; i <= count; i++)
        {
            if (i > 1) history += "  |  ";
            history += GetCardByIndex(state, i);
        }
        m_History.SetText(history);
    }

    protected string GetCardByIndex(OERideBusNetState state, int index)
    {
        if (!state) return "";
        if (index == 1) return state.card1;
        if (index == 2) return state.card2;
        if (index == 3) return state.card3;
        if (index == 4) return state.card4;
        return "";
    }

    protected int CountRevealedCards(OERideBusNetState state)
    {
        int count = 0;
        if (state.card1 != "") count++;
        if (state.card2 != "") count++;
        if (state.card3 != "") count++;
        if (state.card4 != "") count++;
        return count;
    }

    protected void UpdateTimerBar(float remaining)
    {
        if (!m_TimerBar) return;
        if (m_CountdownTotal <= 0)
        {
            m_TimerBar.SetText("");
            return;
        }

        int segments = 10;
        int filled = Math.Ceil((remaining / m_CountdownTotal) * segments);
        if (filled < 0) filled = 0;
        if (filled > segments) filled = segments;

        string bar = "[";
        for (int i = 0; i < segments; i++)
        {
            if (i < filled) bar += "#";
            else bar += "-";
        }
        bar += "]";
        m_TimerBar.SetText(bar);
    }

    protected void AdjustBet(int delta)
    {
        if (m_ActiveGame || m_WaitingForResponse) return;
        m_SelectedBet += delta;
        ClampSelectedBet();
    }

    protected void ClampSelectedBet()
    {
        int minBet = 10;
        int maxBet = 5000;

        if (OECasinoConfig.Instance && OECasinoConfig.Instance.rideBus)
        {
            minBet = OECasinoConfig.Instance.rideBus.minBet;
            maxBet = OECasinoConfig.Instance.rideBus.maxBet;
        }

        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
        if (m_SelectedBetText) m_SelectedBetText.SetText(m_SelectedBet.ToString());
        if (!m_ActiveGame && m_Bet) m_Bet.SetText(m_SelectedBet.ToString());
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().HideScriptedMenu(this);
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || m_StationId == "") return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart(int bet)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || m_StationId == "") return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_START, new Param2<string, int>(m_StationId, bet), true);
    }

    protected void SendChoice(int choice)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_CHOICE, new Param1<int>(choice), true);
    }

    protected void SendCashOut()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_RTB_CASHOUT, new Param1<int>(0), true);
    }
}
