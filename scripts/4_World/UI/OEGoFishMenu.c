class OEGoFishMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Bet;
    protected TextWidget m_Result;
    protected TextWidget m_Books;
    protected TextWidget m_Message;
    protected TextWidget m_DealerMeta;
    protected TextWidget m_PlayerMeta;
    protected TextWidget m_PondText;
    protected TextWidget m_SelectedBetText;
    protected Widget m_WagerPanel;
    protected TextWidget m_AskLabel;
    protected ImageWidget m_PondCard;
    protected ref array<ImageWidget> m_DealerCards;
    protected ref array<ImageWidget> m_PlayerCards;
    protected ref array<ButtonWidget> m_RankButtons;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinusLarge;
    protected ButtonWidget m_BetMinusSmall;
    protected ButtonWidget m_BetPlusSmall;
    protected ButtonWidget m_BetPlusLarge;
    protected ButtonWidget m_MaxBet;
    protected ButtonWidget m_Start;

    protected string m_StationId;
    protected int m_SelectedBet = 100;
    protected bool m_WaitingForResponse;
    protected int m_LastAskedRank = -1;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;
        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/GoFish.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Bet = TextWidget.Cast(m_Root.FindAnyWidget("betValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_Books = TextWidget.Cast(m_Root.FindAnyWidget("booksValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_DealerMeta = TextWidget.Cast(m_Root.FindAnyWidget("dealerMeta"));
        m_PlayerMeta = TextWidget.Cast(m_Root.FindAnyWidget("playerMeta"));
        m_PondText = TextWidget.Cast(m_Root.FindAnyWidget("pondText"));
        m_SelectedBetText = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_WagerPanel = m_Root.FindAnyWidget("wagerPanel");
        m_AskLabel = TextWidget.Cast(m_Root.FindAnyWidget("askLabel"));
        m_PondCard = ImageWidget.Cast(m_Root.FindAnyWidget("pondCard"));

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinusLarge = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinusLarge"));
        m_BetMinusSmall = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinusSmall"));
        m_BetPlusSmall = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlusSmall"));
        m_BetPlusLarge = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlusLarge"));
        m_MaxBet = ButtonWidget.Cast(m_Root.FindAnyWidget("maxBetButton"));
        m_Start = ButtonWidget.Cast(m_Root.FindAnyWidget("startButton"));

        m_DealerCards = new array<ImageWidget>;
        for (int d = 1; d <= 8; d++)
        {
            ImageWidget dealer = ImageWidget.Cast(m_Root.FindAnyWidget("dealerCard" + d.ToString()));
            if (dealer) m_DealerCards.Insert(dealer);
        }

        m_PlayerCards = new array<ImageWidget>;
        for (int p = 1; p <= 24; p++)
        {
            ImageWidget playerCard = ImageWidget.Cast(m_Root.FindAnyWidget("playerCard" + p.ToString()));
            if (playerCard) m_PlayerCards.Insert(playerCard);
        }

        m_RankButtons = new array<ButtonWidget>;
        for (int rank = 2; rank <= 14; rank++)
        {
            ButtonWidget rankButton = ButtonWidget.Cast(m_Root.FindAnyWidget("rankButton" + rank.ToString()));
            if (rankButton) m_RankButtons.Insert(rankButton);
        }

        if (m_PondCard)
        {
            m_PondCard.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
            m_PondCard.SetImage(0);
        }

        LoadDefaultBet();
        ClampSelectedBet();
        UpdateBetControls();
        RenderIdleCards();
        SetRankButtons("0,0,0,0,0,0,0,0,0,0,0,0,0", false);
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
        m_LastAskedRank = -1;
        RequestState();
    }

    override void OnHide()
    {
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
        if (GetUApi() && GetUApi().GetInputByName("UAUIBack").LocalPress()) CloseMenu();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_Close) { CloseMenu(); return true; }
        if (m_WaitingForResponse) return true;

        int step = GetBetStep();
        if (w == m_BetMinusLarge) { AdjustBet(-(step * 10)); return true; }
        if (w == m_BetMinusSmall) { AdjustBet(-step); return true; }
        if (w == m_BetPlusSmall) { AdjustBet(step); return true; }
        if (w == m_BetPlusLarge) { AdjustBet(step * 10); return true; }
        if (w == m_MaxBet) { SetMaxBet(); return true; }

        if (w == m_Start)
        {
            m_WaitingForResponse = true;
            if (m_Message) m_Message.SetText("Dealing Go Fish...");
            SendStart();
            return true;
        }

        for (int i = 0; i < m_RankButtons.Count(); i++)
        {
            if (w == m_RankButtons.Get(i))
            {
                int rank = i + 2;
                m_LastAskedRank = rank;
                m_WaitingForResponse = true;
                HighlightAskedRank();
                if (m_Message) m_Message.SetText("ASKING FOR " + RankShort(rank));
                SendAsk(rank);
                return true;
            }
        }
        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OEGoFishNetState state)
    {
        if (!state) return;
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Bet) m_Bet.SetText(state.bet.ToString());
        if (m_Books) m_Books.SetText("YOU " + state.playerBooks.ToString() + " | DEALER " + state.dealerBooks.ToString());
        if (m_Message) m_Message.SetText(state.message);
        if (m_PondText) m_PondText.SetText("POND: " + state.pondCount.ToString());
        if (m_DealerMeta) m_DealerMeta.SetText("HAND " + state.dealerHandCount.ToString() + " | BOOKS: " + state.dealerBookText);
        if (m_PlayerMeta)
        {
            string shown = "HAND " + state.playerHandCount.ToString() + " | BOOKS: " + state.playerBookText;
            if (state.playerHandCount > 24) shown += " | SHOWING 24";
            m_PlayerMeta.SetText(shown);
        }

        if (state.animateCards) OECasinoAudio.PlayCardFlip();

        if (state.status == OE_GOFISH_STATUS_IDLE)
        {
            m_LastAskedRank = -1;
            if (m_Result) m_Result.SetText("READY");
            if (m_WagerPanel) m_WagerPanel.Show(true);
            RenderIdleCards();
            SetRankButtons(state.playerRankCounts, false);
            return;
        }

        RenderDealerBacks(state.dealerHandCount);
        RenderPlayerCards(state.playerCards);

        if (state.status == OE_GOFISH_STATUS_ACTIVE)
        {
            if (m_Result) m_Result.SetText("YOUR TURN");
            if (m_WagerPanel) m_WagerPanel.Show(false);
            SetRankButtons(state.playerRankCounts, true);
            return;
        }

        if (m_WagerPanel) m_WagerPanel.Show(true);
        m_LastAskedRank = -1;
        SetRankButtons(state.playerRankCounts, false);
        if (state.status == OE_GOFISH_STATUS_PLAYER_WIN) m_Result.SetText("WIN");
        else if (state.status == OE_GOFISH_STATUS_DEALER_WIN) m_Result.SetText("LOSS");
        else if (state.status == OE_GOFISH_STATUS_PUSH) m_Result.SetText("PUSH");
        else if (state.status == OE_GOFISH_STATUS_ERROR) m_Result.SetText("ERROR");
    }

    protected void RenderIdleCards()
    {
        string back = "OperationExileCasino/data/cards/oe_card_back.edds";
        for (int i = 0; i < m_DealerCards.Count(); i++)
        {
            ImageWidget dw = m_DealerCards.Get(i);
            if (i < 3) { dw.LoadImageFile(0, back); dw.SetImage(0); dw.Show(true); }
            else dw.Show(false);
        }
        for (int j = 0; j < m_PlayerCards.Count(); j++)
        {
            ImageWidget pw = m_PlayerCards.Get(j);
            if (j < 3) { pw.LoadImageFile(0, back); pw.SetImage(0); pw.Show(true); }
            else pw.Show(false);
        }
    }

    protected void RenderDealerBacks(int count)
    {
        string back = "OperationExileCasino/data/cards/oe_card_back.edds";
        for (int i = 0; i < m_DealerCards.Count(); i++)
        {
            ImageWidget widget = m_DealerCards.Get(i);
            if (i < count)
            {
                widget.LoadImageFile(0, back);
                widget.SetImage(0);
                widget.Show(true);
            }
            else widget.Show(false);
        }
    }

    protected void RenderPlayerCards(string encoded)
    {
        for (int i = 0; i < m_PlayerCards.Count(); i++) m_PlayerCards.Get(i).Show(false);
        TStringArray entries = new TStringArray;
        if (encoded != "") encoded.Split(";", entries);
        int slot = 0;
        foreach (string entry : entries)
        {
            if (slot >= m_PlayerCards.Count()) break;
            TStringArray parts = new TStringArray;
            entry.Split(",", parts);
            if (parts.Count() < 2) continue;
            int rank = parts.Get(0).ToInt();
            int suit = parts.Get(1).ToInt();
            string texture = GetCardFaceTexturePath(rank, suit);
            if (texture == "") continue;
            ImageWidget widget = m_PlayerCards.Get(slot);
            widget.LoadImageFile(0, texture);
            widget.SetImage(0);
            widget.Show(true);
            slot++;
        }
    }

    protected void SetRankButtons(string encodedCounts, bool active)
    {
        TStringArray counts = new TStringArray;
        encodedCounts.Split(",", counts);
        for (int i = 0; i < m_RankButtons.Count(); i++)
        {
            ButtonWidget button = m_RankButtons.Get(i);
            int count = 0;
            if (i < counts.Count()) count = counts.Get(i).ToInt();
            int rank = i + 2;
            string label = RankShort(rank);
            if (count > 0) label += " x" + count.ToString();
            button.SetText(label);
            bool legal = active && count > 0;
            button.Enable(legal);

            if (rank == m_LastAskedRank)
                button.SetColor(ARGB(255, 178, 126, 16));
            else if (legal)
                button.SetColor(ARGB(255, 28, 86, 46));
            else
                button.SetColor(ARGB(255, 20, 20, 20));
        }
        if (m_AskLabel) m_AskLabel.Show(active);
    }

    protected void HighlightAskedRank()
    {
        for (int i = 0; i < m_RankButtons.Count(); i++)
        {
            ButtonWidget button = m_RankButtons.Get(i);
            int rank = i + 2;
            if (rank == m_LastAskedRank)
                button.SetColor(ARGB(255, 178, 126, 16));
        }
    }

    protected string RankShort(int rank)
    {
        if (rank == 11) return "J";
        if (rank == 12) return "Q";
        if (rank == 13) return "K";
        if (rank == 14) return "A";
        return rank.ToString();
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

    protected void LoadDefaultBet()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.goFish)
            m_SelectedBet = OECasinoConfig.Instance.goFish.defaultBet;
    }

    protected int GetBetStep()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.goFish && OECasinoConfig.Instance.goFish.betStep > 0)
            return OECasinoConfig.Instance.goFish.betStep;
        return 10;
    }

    protected void ClampSelectedBet()
    {
        int minBet = 10;
        int maxBet = 1000;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.goFish)
        {
            minBet = OECasinoConfig.Instance.goFish.minBet;
            maxBet = OECasinoConfig.Instance.goFish.maxBet;
        }
        if (m_SelectedBet < minBet) m_SelectedBet = minBet;
        if (m_SelectedBet > maxBet) m_SelectedBet = maxBet;
        int step = GetBetStep();
        if (step > 1)
        {
            int offset = m_SelectedBet - minBet;
            m_SelectedBet = minBet + ((offset / step) * step);
        }
    }

    protected void AdjustBet(int delta)
    {
        m_SelectedBet += delta;
        ClampSelectedBet();
        UpdateBetControls();
    }

    protected void SetMaxBet()
    {
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.goFish)
            m_SelectedBet = OECasinoConfig.Instance.goFish.maxBet;
        ClampSelectedBet();
        UpdateBetControls();
    }

    protected void UpdateBetControls()
    {
        if (m_SelectedBetText) m_SelectedBetText.SetText(m_SelectedBet.ToString());
        int step = GetBetStep();
        if (m_BetMinusSmall) m_BetMinusSmall.SetText("-" + step.ToString());
        if (m_BetPlusSmall) m_BetPlusSmall.SetText("+" + step.ToString());
        if (m_BetMinusLarge) m_BetMinusLarge.SetText("-" + (step * 10).ToString());
        if (m_BetPlusLarge) m_BetPlusLarge.SetText("+" + (step * 10).ToString());
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_GOFISH_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_GOFISH_START, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void SendAsk(int rank)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_GOFISH_ASK, new Param1<int>(rank), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
