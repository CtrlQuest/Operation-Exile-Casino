class OEBlackjackMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Result;
    protected TextWidget m_Message;
    protected TextWidget m_DealerTotal;
    protected TextWidget m_PlayerLabel;
    protected TextWidget m_PlayerTotal;
    protected TextWidget m_SelectedBetText;
    protected TextWidget m_SplitHand1Label;
    protected TextWidget m_SplitHand1Meta;
    protected TextWidget m_SplitHand2Label;
    protected TextWidget m_SplitHand2Meta;

    protected Widget m_StartPanel;
    protected Widget m_ActionPanel;

    protected ButtonWidget m_Start;
    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Hit;
    protected ButtonWidget m_Stand;
    protected ButtonWidget m_Double;
    protected ButtonWidget m_Split;

    protected ref array<ImageWidget> m_DealerCardWidgets;
    protected ref array<ImageWidget> m_PlayerCardWidgets;
    protected ref array<ImageWidget> m_SplitHand1CardWidgets;
    protected ref array<ImageWidget> m_SplitHand2CardWidgets;
    protected ref array<Widget> m_DealerFlipBars;
    protected ref array<Widget> m_PlayerFlipBars;
    protected ref array<Widget> m_SplitHand1FlipBars;
    protected ref array<Widget> m_SplitHand2FlipBars;
    protected ref array<int> m_DealerFlipIndexes;
    protected ref array<int> m_PlayerFlipIndexes;
    protected ref array<int> m_SplitHand1FlipIndexes;
    protected ref array<int> m_SplitHand2FlipIndexes;
    protected ref OEBlackjackNetState m_PendingFlipState;
    protected int m_LastDealerVisibleCount;
    protected int m_LastDealerTotalCount;
    protected int m_LastPlayerCount;
    protected int m_LastSplitHand1Count;
    protected int m_LastSplitHand2Count;

    protected string m_StationId;
    protected bool m_ActiveGame;
    protected bool m_WaitingForResponse;
    protected bool m_PlayFlipOnNextState;
    protected int m_SelectedBet = 100;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/Blackjack.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_DealerTotal = TextWidget.Cast(m_Root.FindAnyWidget("dealerTotalValue"));
        m_PlayerLabel = TextWidget.Cast(m_Root.FindAnyWidget("playerLabel"));
        m_PlayerTotal = TextWidget.Cast(m_Root.FindAnyWidget("playerTotalValue"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_SplitHand1Label = TextWidget.Cast(m_Root.FindAnyWidget("splitHand1Label"));
        m_SplitHand1Meta = TextWidget.Cast(m_Root.FindAnyWidget("splitHand1Meta"));
        m_SplitHand2Label = TextWidget.Cast(m_Root.FindAnyWidget("splitHand2Label"));
        m_SplitHand2Meta = TextWidget.Cast(m_Root.FindAnyWidget("splitHand2Meta"));

        m_StartPanel = m_Root.FindAnyWidget("startPanel");
        m_ActionPanel = m_Root.FindAnyWidget("actionPanel");

        m_Start = ButtonWidget.Cast(m_Root.FindAnyWidget("startButton"));
        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Hit = ButtonWidget.Cast(m_Root.FindAnyWidget("hitButton"));
        m_Stand = ButtonWidget.Cast(m_Root.FindAnyWidget("standButton"));
        m_Double = ButtonWidget.Cast(m_Root.FindAnyWidget("doubleButton"));
        m_Split = ButtonWidget.Cast(m_Root.FindAnyWidget("splitButton"));

        m_DealerCardWidgets = new array<ImageWidget>;
        m_PlayerCardWidgets = new array<ImageWidget>;
        m_SplitHand1CardWidgets = new array<ImageWidget>;
        m_SplitHand2CardWidgets = new array<ImageWidget>;
        m_DealerFlipBars = new array<Widget>;
        m_PlayerFlipBars = new array<Widget>;
        m_SplitHand1FlipBars = new array<Widget>;
        m_SplitHand2FlipBars = new array<Widget>;
        m_DealerFlipIndexes = new array<int>;
        m_PlayerFlipIndexes = new array<int>;
        m_SplitHand1FlipIndexes = new array<int>;
        m_SplitHand2FlipIndexes = new array<int>;
        for (int i = 1; i <= 6; i++)
        {
            ImageWidget dealerWidget = ImageWidget.Cast(m_Root.FindAnyWidget("dealerCard" + i.ToString()));
            ImageWidget playerWidget = ImageWidget.Cast(m_Root.FindAnyWidget("playerCard" + i.ToString()));
            ImageWidget split1Widget = ImageWidget.Cast(m_Root.FindAnyWidget("splitHand1Card" + i.ToString()));
            ImageWidget split2Widget = ImageWidget.Cast(m_Root.FindAnyWidget("splitHand2Card" + i.ToString()));
            Widget dealerFlipBar = m_Root.FindAnyWidget("dealerFlipBar" + i.ToString());
            Widget playerFlipBar = m_Root.FindAnyWidget("playerFlipBar" + i.ToString());
            Widget split1FlipBar = m_Root.FindAnyWidget("splitHand1FlipBar" + i.ToString());
            Widget split2FlipBar = m_Root.FindAnyWidget("splitHand2FlipBar" + i.ToString());
            if (dealerWidget) m_DealerCardWidgets.Insert(dealerWidget);
            if (playerWidget) m_PlayerCardWidgets.Insert(playerWidget);
            if (split1Widget) m_SplitHand1CardWidgets.Insert(split1Widget);
            if (split2Widget) m_SplitHand2CardWidgets.Insert(split2Widget);
            if (dealerFlipBar) m_DealerFlipBars.Insert(dealerFlipBar);
            if (playerFlipBar) m_PlayerFlipBars.Insert(playerFlipBar);
            if (split1FlipBar) m_SplitHand1FlipBars.Insert(split1FlipBar);
            if (split2FlipBar) m_SplitHand2FlipBars.Insert(split2FlipBar);
        }

        HideNormalFlipBars();
        ClampSelectedBet();
        ClearCards();
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
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.BlackjackFlipMid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishBlackjackFlip);
        m_PendingFlipState = null;
        HideNormalFlipBars();
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

        if (w == m_Start)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Dealing...");
            m_PlayFlipOnNextState = true;
            SendStart(m_SelectedBet);
            return true;
        }

        if (w == m_Hit)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Dealing another card...");
            m_PlayFlipOnNextState = true;
            SendAction(OE_BJ_ACTION_HIT);
            return true;
        }

        if (w == m_Stand)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Dealer is playing...");
            m_PlayFlipOnNextState = true;
            SendAction(OE_BJ_ACTION_STAND);
            return true;
        }

        if (w == m_Double)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Doubling the stake and drawing one card...");
            m_PlayFlipOnNextState = true;
            SendAction(OE_BJ_ACTION_DOUBLE);
            return true;
        }

        if (w == m_Split)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Splitting into two hands...");
            m_PlayFlipOnNextState = true;
            SendAction(OE_BJ_ACTION_SPLIT);
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OEBlackjackNetState state)
    {
        if (!state) return;

        if (m_PlayFlipOnNextState && state.status != OE_BJ_STATUS_ERROR)
        {
            m_PlayFlipOnNextState = false;

            BeginBlackjackFlip(state);
            return;
        }

        m_PlayFlipOnNextState = false;
        RenderStateImmediate(state);
    }

    protected void RenderStateImmediate(OEBlackjackNetState state)
    {
        if (!state) return;
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        UpdateResultDisplay(state);
        if (m_Message) m_Message.SetText(state.message);
        if (m_DealerTotal) m_DealerTotal.SetText("TOTAL: " + state.dealerTotal.ToString());
        RenderCardList(state.dealerCards, m_DealerCardWidgets, state.dealerHiddenCards);

        if (state.splitActive)
            RenderSplitState(state);
        else
        {
            ShowNormalPlayerArea(true);
            HideSplitArea();
            if (m_PlayerTotal) m_PlayerTotal.SetText("TOTAL: " + state.playerTotal.ToString());
            RenderCardList(state.playerCards, m_PlayerCardWidgets, 0);
        }

        m_LastDealerVisibleCount = CountEncodedCards(state.dealerCards);
        m_LastDealerTotalCount = m_LastDealerVisibleCount + state.dealerHiddenCards;
        m_LastPlayerCount = CountEncodedCards(state.playerCards);
        if (state.splitActive)
        {
            m_LastSplitHand1Count = CountEncodedCards(state.splitHand1Cards);
            m_LastSplitHand2Count = CountEncodedCards(state.splitHand2Cards);
        }
        else
        {
            m_LastSplitHand1Count = 0;
            m_LastSplitHand2Count = 0;
        }

        if (state.status == OE_BJ_STATUS_PLAYING)
        {
            m_ActiveGame = true;
            if (m_StartPanel) m_StartPanel.Show(false);
            if (m_ActionPanel) m_ActionPanel.Show(true);

            if (state.splitActive)
            {
                if (m_Hit) { m_Hit.Show(true); m_Hit.Enable(true); }
                if (m_Stand) { m_Stand.Show(true); m_Stand.Enable(true); }
                if (m_Double) { m_Double.Show(false); m_Double.Enable(false); }
                if (m_Split) { m_Split.Show(false); m_Split.Enable(false); }
                return;
            }

            if (m_Hit) { m_Hit.Show(true); m_Hit.Enable(state.canHit); }
            if (m_Stand) { m_Stand.Show(true); m_Stand.Enable(state.canStand); }
            if (m_Double) { m_Double.Show(true); m_Double.Enable(state.canDouble); }
            if (m_Split) { m_Split.Show(state.canSplit); m_Split.Enable(state.canSplit); }
            return;
        }

        m_ActiveGame = false;
        if (state.status == OE_BJ_STATUS_IDLE || state.status == OE_BJ_STATUS_ERROR)
        {
            ShowIdleCardBacks();
            m_LastDealerVisibleCount = 0;
            m_LastDealerTotalCount = 0;
            m_LastPlayerCount = 0;
            m_LastSplitHand1Count = 0;
            m_LastSplitHand2Count = 0;
        }
        ShowIdleControls();
    }

    protected int CountEncodedCards(string encoded)
    {
        if (encoded == "") return 0;
        TStringArray entries = new TStringArray;
        encoded.Split(";", entries);
        int count = 0;
        foreach (string entry : entries)
        {
            if (entry != "") count++;
        }
        return count;
    }

    protected void BeginBlackjackFlip(OEBlackjackNetState state)
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.BlackjackFlipMid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.FinishBlackjackFlip);
        HideNormalFlipBars();
        m_DealerFlipIndexes.Clear();
        m_PlayerFlipIndexes.Clear();
        m_SplitHand1FlipIndexes.Clear();
        m_SplitHand2FlipIndexes.Clear();
        m_PendingFlipState = state;
        m_WaitingForResponse = true;

        int dealerVisible = CountEncodedCards(state.dealerCards);
        int playerCount = CountEncodedCards(state.playerCards);
        int split1Count = CountEncodedCards(state.splitHand1Cards);
        int split2Count = CountEncodedCards(state.splitHand2Cards);
        int i;

        // Only face-up dealer cards flip. The hole card stays face-down until
        // the server reveals it later, at which point it becomes a new visible card.
        for (i = 0; i < dealerVisible && i < m_DealerCardWidgets.Count(); i++)
        {
            if (i >= m_LastDealerVisibleCount) m_DealerFlipIndexes.Insert(i);
        }
        if (state.splitActive)
        {
            // Render the existing split hands, then turn only newly dealt cards
            // back over for the synchronized flip animation.
            ShowNormalPlayerArea(false);
            RenderSplitState(state);
            for (i = 0; i < split1Count && i < m_SplitHand1CardWidgets.Count(); i++)
            {
                if (i >= m_LastSplitHand1Count) m_SplitHand1FlipIndexes.Insert(i);
            }
            for (i = 0; i < split2Count && i < m_SplitHand2CardWidgets.Count(); i++)
            {
                if (i >= m_LastSplitHand2Count) m_SplitHand2FlipIndexes.Insert(i);
            }
        }
        else
        {
            for (i = 0; i < playerCount && i < m_PlayerCardWidgets.Count(); i++)
            {
                if (i >= m_LastPlayerCount) m_PlayerFlipIndexes.Insert(i);
            }

            // If the first response arrives while the idle backs are displayed,
            // explicitly flip the opening player cards. The dealer hole card stays hidden.
            if (m_LastPlayerCount == 0 && playerCount > 0 && m_PlayerFlipIndexes.Count() == 0)
                for (i = 0; i < playerCount; i++) m_PlayerFlipIndexes.Insert(i);
        }

        string back = "OperationExileCasino/data/cards/oe_card_back.edds";
        foreach (int dealerIndex : m_DealerFlipIndexes)
        {
            ImageWidget dw = m_DealerCardWidgets.Get(dealerIndex);
            if (dw) { dw.LoadImageFile(0, back); dw.SetImage(0); dw.Show(true); }
        }
        foreach (int playerIndex : m_PlayerFlipIndexes)
        {
            ImageWidget pw = m_PlayerCardWidgets.Get(playerIndex);
            if (pw) { pw.LoadImageFile(0, back); pw.SetImage(0); pw.Show(true); }
        }
        foreach (int split1Index : m_SplitHand1FlipIndexes)
        {
            ImageWidget s1w = m_SplitHand1CardWidgets.Get(split1Index);
            if (s1w) { s1w.LoadImageFile(0, back); s1w.SetImage(0); s1w.Show(true); }
        }
        foreach (int split2Index : m_SplitHand2FlipIndexes)
        {
            ImageWidget s2w = m_SplitHand2CardWidgets.Get(split2Index);
            if (s2w) { s2w.LoadImageFile(0, back); s2w.SetImage(0); s2w.Show(true); }
        }

        // A stand between split hands may not deal or reveal a card. In that case
        // there is nothing to animate, so apply the state immediately.
        if (m_DealerFlipIndexes.Count() == 0 && m_PlayerFlipIndexes.Count() == 0 && m_SplitHand1FlipIndexes.Count() == 0 && m_SplitHand2FlipIndexes.Count() == 0)
        {
            m_PendingFlipState = null;
            RenderStateImmediate(state);
            return;
        }

        if (m_Result) m_Result.SetText("DEALING");
        if (m_Message) m_Message.SetText("Cards dealing...");
        OECasinoAudio.PlayCardFlip();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.BlackjackFlipMid, 320, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.FinishBlackjackFlip, 780, false);
    }

    protected void BlackjackFlipMid()
    {
        if (!m_PendingFlipState) return;
        foreach (int dealerIndex : m_DealerFlipIndexes)
        {
            if (dealerIndex < m_DealerCardWidgets.Count()) m_DealerCardWidgets.Get(dealerIndex).Show(false);
            if (dealerIndex < m_DealerFlipBars.Count()) m_DealerFlipBars.Get(dealerIndex).Show(true);
        }
        foreach (int playerIndex : m_PlayerFlipIndexes)
        {
            if (playerIndex < m_PlayerCardWidgets.Count()) m_PlayerCardWidgets.Get(playerIndex).Show(false);
            if (playerIndex < m_PlayerFlipBars.Count()) m_PlayerFlipBars.Get(playerIndex).Show(true);
        }
        foreach (int split1Index : m_SplitHand1FlipIndexes)
        {
            if (split1Index < m_SplitHand1CardWidgets.Count()) m_SplitHand1CardWidgets.Get(split1Index).Show(false);
            if (split1Index < m_SplitHand1FlipBars.Count()) m_SplitHand1FlipBars.Get(split1Index).Show(true);
        }
        foreach (int split2Index : m_SplitHand2FlipIndexes)
        {
            if (split2Index < m_SplitHand2CardWidgets.Count()) m_SplitHand2CardWidgets.Get(split2Index).Show(false);
            if (split2Index < m_SplitHand2FlipBars.Count()) m_SplitHand2FlipBars.Get(split2Index).Show(true);
        }
    }

    protected void FinishBlackjackFlip()
    {
        if (!m_PendingFlipState) return;
        OEBlackjackNetState state = m_PendingFlipState;
        m_PendingFlipState = null;
        HideNormalFlipBars();
        RenderStateImmediate(state);
    }

    protected void HideNormalFlipBars()
    {
        if (m_DealerFlipBars)
        {
            foreach (Widget dealerBar : m_DealerFlipBars)
            {
                if (dealerBar) dealerBar.Show(false);
            }
        }
        if (m_PlayerFlipBars)
        {
            foreach (Widget playerBar : m_PlayerFlipBars)
            {
                if (playerBar) playerBar.Show(false);
            }
        }
        if (m_SplitHand1FlipBars)
        {
            foreach (Widget split1Bar : m_SplitHand1FlipBars)
            {
                if (split1Bar) split1Bar.Show(false);
            }
        }
        if (m_SplitHand2FlipBars)
        {
            foreach (Widget split2Bar : m_SplitHand2FlipBars)
            {
                if (split2Bar) split2Bar.Show(false);
            }
        }
    }

    protected void UpdateResultDisplay(OEBlackjackNetState state)
    {
        if (!m_Result || !state) return;

        if (state.splitActive)
        {
            if (state.status == OE_BJ_STATUS_PLAYING)
            {
                m_Result.SetText("HAND " + state.activeSplitHand.ToString() + " IN PLAY");
                return;
            }

            if (state.splitNetResult > 0)
            {
                m_Result.SetText("WIN +" + state.splitNetResult.ToString());
                return;
            }
            if (state.splitNetResult < 0)
            {
                int splitLoss = 0 - state.splitNetResult;
                m_Result.SetText("LOSS -" + splitLoss.ToString());
                return;
            }

            m_Result.SetText("PUSH 0");
            return;
        }

        if (state.status == OE_BJ_STATUS_PLAYING)
        {
            m_Result.SetText("IN PLAY");
            return;
        }

        if (state.status == OE_BJ_STATUS_PLAYER_WIN)
        {
            float normalMultiplier = 2.0;
            if (OECasinoConfig.Instance && OECasinoConfig.Instance.blackjack)
                normalMultiplier = OECasinoConfig.Instance.blackjack.normalWinPayout;

            int normalReturn = Math.Floor(state.bet * normalMultiplier);
            int normalNet = normalReturn - state.bet;
            m_Result.SetText("WIN +" + normalNet.ToString());
            return;
        }

        if (state.status == OE_BJ_STATUS_BLACKJACK)
        {
            float blackjackMultiplier = 2.5;
            if (OECasinoConfig.Instance && OECasinoConfig.Instance.blackjack)
                blackjackMultiplier = OECasinoConfig.Instance.blackjack.blackjackPayout;

            int blackjackReturn = Math.Floor(state.bet * blackjackMultiplier);
            int blackjackNet = blackjackReturn - state.bet;
            m_Result.SetText("BLACKJACK +" + blackjackNet.ToString());
            return;
        }

        if (state.status == OE_BJ_STATUS_DEALER_WIN)
        {
            m_Result.SetText("LOSS -" + state.bet.ToString());
            return;
        }

        if (state.status == OE_BJ_STATUS_PUSH)
        {
            m_Result.SetText("PUSH 0");
            return;
        }

        m_Result.SetText("-");
    }

    protected void RenderCardList(string encoded, array<ImageWidget> widgets, int hiddenCards)
    {
        if (!widgets) return;
        for (int h = 0; h < widgets.Count(); h++)
            widgets.Get(h).Show(false);

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
            widget.LoadImageFile(0, texturePath);
            widget.SetImage(0);
            widget.Show(true);
            slot++;
        }

        for (int i = 0; i < hiddenCards && slot < widgets.Count(); i++)
        {
            ImageWidget hiddenWidget = widgets.Get(slot);
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

    protected void ShowIdleCardBacks()
    {
        string back = "OperationExileCasino/data/cards/oe_card_back.edds";
        if (m_DealerCardWidgets)
        {
            for (int i = 0; i < m_DealerCardWidgets.Count(); i++)
            {
                ImageWidget w = m_DealerCardWidgets.Get(i);
                if (!w) continue;
                if (i < 2)
                {
                    w.LoadImageFile(0, back);
                    w.SetImage(0);
                    w.Show(true);
                }
                else w.Show(false);
            }
        }
        if (m_PlayerCardWidgets)
        {
            for (int j = 0; j < m_PlayerCardWidgets.Count(); j++)
            {
                ImageWidget pw = m_PlayerCardWidgets.Get(j);
                if (!pw) continue;
                if (j < 2)
                {
                    pw.LoadImageFile(0, back);
                    pw.SetImage(0);
                    pw.Show(true);
                }
                else pw.Show(false);
            }
        }
    }

    protected void ClearCards()
    {
        if (m_DealerCardWidgets)
        {
            foreach (ImageWidget d : m_DealerCardWidgets)
            {
                d.Show(false);
            }
        }
        if (m_PlayerCardWidgets)
        {
            foreach (ImageWidget p : m_PlayerCardWidgets)
            {
                p.Show(false);
            }
        }
        if (m_SplitHand1CardWidgets)
        {
            foreach (ImageWidget s1 : m_SplitHand1CardWidgets)
            {
                s1.Show(false);
            }
        }
        if (m_SplitHand2CardWidgets)
        {
            foreach (ImageWidget s2 : m_SplitHand2CardWidgets)
            {
                s2.Show(false);
            }
        }
        HideSplitArea();
        ShowNormalPlayerArea(true);
    }

    protected void RenderSplitState(OEBlackjackNetState state)
    {
        ShowNormalPlayerArea(false);

        if (m_SplitHand1Label)
        {
            string hand1Label = "HAND 1";
            if (state.status == OE_BJ_STATUS_PLAYING && state.activeSplitHand == 1) hand1Label += " - ACTIVE";
            m_SplitHand1Label.SetText(hand1Label);
            m_SplitHand1Label.Show(true);
        }
        if (m_SplitHand2Label)
        {
            string hand2Label = "HAND 2";
            if (state.status == OE_BJ_STATUS_PLAYING && state.activeSplitHand == 2) hand2Label += " - ACTIVE";
            m_SplitHand2Label.SetText(hand2Label);
            m_SplitHand2Label.Show(true);
        }

        string hand1Meta = "TOTAL: " + state.splitHand1Total.ToString() + "   BET: " + state.splitHandBet.ToString();
        string hand2Meta = "TOTAL: " + state.splitHand2Total.ToString() + "   BET: " + state.splitHandBet.ToString();
        if (state.splitHand1Result != "") hand1Meta += "   " + state.splitHand1Result;
        if (state.splitHand2Result != "") hand2Meta += "   " + state.splitHand2Result;

        if (m_SplitHand1Meta) { m_SplitHand1Meta.SetText(hand1Meta); m_SplitHand1Meta.Show(true); }
        if (m_SplitHand2Meta) { m_SplitHand2Meta.SetText(hand2Meta); m_SplitHand2Meta.Show(true); }

        RenderCardList(state.splitHand1Cards, m_SplitHand1CardWidgets, 0);
        RenderCardList(state.splitHand2Cards, m_SplitHand2CardWidgets, 0);
    }

    protected void ShowNormalPlayerArea(bool show)
    {
        if (m_PlayerLabel) m_PlayerLabel.Show(show);
        if (m_PlayerTotal) m_PlayerTotal.Show(show);
        if (!show && m_PlayerCardWidgets)
        {
            foreach (ImageWidget p : m_PlayerCardWidgets)
            {
                p.Show(false);
            }
        }
    }

    protected void HideSplitArea()
    {
        if (m_SplitHand1Label) m_SplitHand1Label.Show(false);
        if (m_SplitHand1Meta) m_SplitHand1Meta.Show(false);
        if (m_SplitHand2Label) m_SplitHand2Label.Show(false);
        if (m_SplitHand2Meta) m_SplitHand2Meta.Show(false);
        if (m_SplitHand1CardWidgets)
        {
            foreach (ImageWidget s1 : m_SplitHand1CardWidgets)
            {
                s1.Show(false);
            }
        }
        if (m_SplitHand2CardWidgets)
        {
            foreach (ImageWidget s2 : m_SplitHand2CardWidgets)
            {
                s2.Show(false);
            }
        }
    }

    protected void ShowIdleControls()
    {
        if (m_StartPanel) m_StartPanel.Show(true);
        if (m_ActionPanel) m_ActionPanel.Show(false);
        if (!m_ActiveGame && m_Bet) m_Bet.SetText(m_SelectedBet.ToString());
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
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.blackjack)
        {
            minBet = OECasinoConfig.Instance.blackjack.minBet;
            maxBet = OECasinoConfig.Instance.blackjack.maxBet;
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
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart(int bet)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || m_StationId == "") return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_START, new Param2<string, int>(m_StationId, bet), true);
    }

    protected void SendAction(int action)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_BJ_ACTION, new Param1<int>(action), true);
    }
}
