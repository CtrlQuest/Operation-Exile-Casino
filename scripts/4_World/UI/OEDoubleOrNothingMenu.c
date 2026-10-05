class OEDoubleOrNothingMenu : UIScriptedMenu
{
    protected Widget m_Root;
    protected TextWidget m_Balance;
    protected TextWidget m_Stake;
    protected TextWidget m_Result;
    protected TextWidget m_CurrentLabel;
    protected TextWidget m_CurrentValue;
    protected TextWidget m_NextValue;
    protected TextWidget m_RunValue;
    protected TextWidget m_OddsValue;
    protected TextWidget m_Message;
    protected TextWidget m_SelectedBetValue;
    protected TextWidget m_CardResult;
    protected TextWidget m_StatusPrompt;
    protected Widget m_PlayPanel;
    protected Widget m_ActiveCardSlot;
    protected Widget m_ActiveCardFace;
    protected ImageWidget m_ActiveCardFaceImage;
    protected ImageWidget m_ActiveCardBack;
    protected TextWidget m_ActiveCardText;

    protected Widget m_StartPanel;
    protected Widget m_ColourChoicePanel;
    protected Widget m_ActionPanel;

    protected ButtonWidget m_Close;
    protected ButtonWidget m_BetMinus100;
    protected ButtonWidget m_BetMinus10;
    protected ButtonWidget m_BetPlus10;
    protected ButtonWidget m_BetPlus100;
    protected ButtonWidget m_Start;
    protected ButtonWidget m_Red;
    protected ButtonWidget m_Black;
    protected ButtonWidget m_Double;
    protected ButtonWidget m_CashOut;

    protected string m_StationId;
    protected bool m_WaitingForResponse;
    protected bool m_PlayFlipOnNextCard;
    protected int m_SelectedBet = 100;

    void SetStation(string stationId)
    {
        m_StationId = stationId;
    }

    override Widget Init()
    {
        if (m_Root) return m_Root;

        m_Root = GetGame().GetWorkspace().CreateWidgets("OperationExileCasino/layouts/DoubleOrNothing.layout");
        layoutRoot = m_Root;

        m_Balance = TextWidget.Cast(m_Root.FindAnyWidget("balanceValue"));
        m_Stake = TextWidget.Cast(m_Root.FindAnyWidget("stakeValue"));
        m_Result = TextWidget.Cast(m_Root.FindAnyWidget("resultValue"));
        m_CurrentLabel = TextWidget.Cast(m_Root.FindAnyWidget("currentLabel"));
        m_CurrentValue = TextWidget.Cast(m_Root.FindAnyWidget("currentValue"));
        m_NextValue = TextWidget.Cast(m_Root.FindAnyWidget("nextValue"));
        m_RunValue = TextWidget.Cast(m_Root.FindAnyWidget("runValue"));
        m_OddsValue = TextWidget.Cast(m_Root.FindAnyWidget("oddsValue"));
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("messageText"));
        m_SelectedBetValue = TextWidget.Cast(m_Root.FindAnyWidget("selectedBetValue"));
        m_CardResult = TextWidget.Cast(m_Root.FindAnyWidget("cardResult"));
        m_StatusPrompt = TextWidget.Cast(m_Root.FindAnyWidget("statusPrompt"));
        m_PlayPanel = m_Root.FindAnyWidget("playPanel");
        m_ActiveCardSlot = m_Root.FindAnyWidget("activeCardSlot");
        m_ActiveCardFace = m_Root.FindAnyWidget("activeCardFace");
        m_ActiveCardFaceImage = ImageWidget.Cast(m_Root.FindAnyWidget("activeCardImage"));
        m_ActiveCardBack = ImageWidget.Cast(m_Root.FindAnyWidget("activeCardBack"));
        m_ActiveCardText = TextWidget.Cast(m_Root.FindAnyWidget("activeCardText"));

        m_StartPanel = m_Root.FindAnyWidget("startPanel");
        m_ColourChoicePanel = m_Root.FindAnyWidget("colourChoicePanel");
        m_ActionPanel = m_Root.FindAnyWidget("actionPanel");

        m_Close = ButtonWidget.Cast(m_Root.FindAnyWidget("closeButton"));
        m_BetMinus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus100"));
        m_BetMinus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betMinus10"));
        m_BetPlus10 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus10"));
        m_BetPlus100 = ButtonWidget.Cast(m_Root.FindAnyWidget("betPlus100"));
        m_Start = ButtonWidget.Cast(m_Root.FindAnyWidget("startButton"));
        m_Red = ButtonWidget.Cast(m_Root.FindAnyWidget("redButton"));
        m_Black = ButtonWidget.Cast(m_Root.FindAnyWidget("blackButton"));
        m_Double = ButtonWidget.Cast(m_Root.FindAnyWidget("doubleButton"));
        m_CashOut = ButtonWidget.Cast(m_Root.FindAnyWidget("cashOutButton"));

        if (!m_PlayPanel || !m_ActiveCardSlot || !m_ActiveCardFace || !m_ActiveCardFaceImage || !m_ActiveCardBack)
            Print("[OperationExileCasino][DoubleOrNothing] UI WARNING: RideBus-style card widgets were not all created from DoubleOrNothing.layout");
        else
            Print("[OperationExileCasino][DoubleOrNothing] RideBus-style card widgets loaded successfully");

        LoadCardBack();
        if (m_PlayPanel) m_PlayPanel.Show(true);
        ClampSelectedBet();
        UpdateSelectedBetText();
        ShowStartControls();
        ResetRunDisplay();
        ShowCardBack();
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
        if (m_PlayPanel) m_PlayPanel.Show(true);
        ShowCardBack();
        RequestState();
    }

    override void OnHide()
    {
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

        if (w == m_Start)
        {
            m_WaitingForResponse = true;
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("WAGER LOCKING...");
            if (m_StatusPrompt) m_StatusPrompt.SetText("WAITING FOR SERVER");
            if (m_Message) m_Message.SetText("Placing " + m_SelectedBet.ToString() + " chip wager...");
            SendStart();
            return true;
        }

        if (w == m_Red)
        {
            m_WaitingForResponse = true;
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("YOU PICKED RED");
            if (m_StatusPrompt) m_StatusPrompt.SetText("REVEALING CARD...");
            if (m_Message) m_Message.SetText("Drawing a card for RED...");
            m_PlayFlipOnNextCard = true;
            SendPickColour(OE_DON_CHOICE_RED);
            return true;
        }

        if (w == m_Black)
        {
            m_WaitingForResponse = true;
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("YOU PICKED BLACK");
            if (m_StatusPrompt) m_StatusPrompt.SetText("REVEALING CARD...");
            if (m_Message) m_Message.SetText("Drawing a card for BLACK...");
            m_PlayFlipOnNextCard = true;
            SendPickColour(OE_DON_CHOICE_BLACK);
            return true;
        }

        if (w == m_Double)
        {
            m_WaitingForResponse = true;
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("NEXT CARD READY");
            if (m_StatusPrompt) m_StatusPrompt.SetText("COMMITTING YOUR CURRENT WIN...");
            if (m_Message) m_Message.SetText("Committing the current win to another card...");
            SendDouble();
            return true;
        }

        if (w == m_CashOut)
        {
            m_WaitingForResponse = true;
            if (m_StatusPrompt) m_StatusPrompt.SetText("CASHING OUT...");
            if (m_Message) m_Message.SetText("Cashing out...");
            SendCashOut();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void ApplyState(OEDoubleOrNothingNetState state)
    {
        if (!state) return;
        m_WaitingForResponse = false;

        if (m_Balance) m_Balance.SetText(state.balance.ToString());
        if (m_Stake) m_Stake.SetText(state.bet.ToString());
        if (m_Message) m_Message.SetText(state.message);
        if (m_OddsValue) m_OddsValue.SetText("50% WIN");
        if (m_CurrentLabel) m_CurrentLabel.SetText("CURRENT VALUE");

        if (state.status == OE_DON_STATUS_IDLE)
        {
            m_PlayFlipOnNextCard = false;
            if (m_Result) m_Result.SetText("-");
            ResetRunDisplay();
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("CARD READY");
            if (m_StatusPrompt) m_StatusPrompt.SetText("PLACE A WAGER TO START");
            ShowStartControls();
            return;
        }

        if (state.status == OE_DON_STATUS_REJECTED)
        {
            m_PlayFlipOnNextCard = false;
            if (m_Result) m_Result.SetText("BET NOT PLACED");
            ResetRunDisplay();
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("NO CARD DRAWN");
            if (m_StatusPrompt) m_StatusPrompt.SetText("ADJUST YOUR WAGER AND TRY AGAIN");
            ShowStartControls();
            return;
        }

        if (state.status == OE_DON_STATUS_ERROR)
        {
            m_PlayFlipOnNextCard = false;
            if (m_Result) m_Result.SetText("ERROR");
            ResetRunDisplay();
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("TABLE ERROR");
            if (m_StatusPrompt) m_StatusPrompt.SetText("CHECK THE MESSAGE BELOW");
            ShowStartControls();
            return;
        }

        if (m_CurrentValue) m_CurrentValue.SetText(state.currentValue.ToString());
        if (m_NextValue) m_NextValue.SetText(state.nextValue.ToString());
        if (m_RunValue) m_RunValue.SetText(state.successfulDoubles.ToString() + " / " + state.maxDoubles.ToString());

        if (state.status == OE_DON_STATUS_CHOOSE)
        {
            if (m_Result) m_Result.SetText("PICK COLOUR");
            ShowCardBack();
            if (m_CardResult) m_CardResult.SetText("FACE-DOWN CARD");
            if (m_StatusPrompt) m_StatusPrompt.SetText("PICK RED OR BLACK - WIN TO DOUBLE TO " + state.nextValue.ToString());
            ShowColourControls();
            return;
        }

        if (state.cardRank >= 2 && state.cardSuit >= 0)
        {
            if (m_PlayFlipOnNextCard)
            {
                OECasinoAudio.PlayCardFlip();
                m_PlayFlipOnNextCard = false;
            }
            ShowCardFace(state.cardRank, state.cardSuit);
            if (m_CardResult) m_CardResult.SetText(GetCardDisplayText(state.cardRank, state.cardSuit));
        }
        else
        {
            ShowCardBack();
        }

        if (state.status == OE_DON_STATUS_ACTIVE)
        {
            if (m_Result) m_Result.SetText("DOUBLE");
            if (m_StatusPrompt) m_StatusPrompt.SetText("TAKE " + state.currentValue.ToString() + " OR RISK IT FOR " + state.nextValue.ToString());
            ShowActionControls();
            return;
        }

        ShowStartControls();

        if (state.status == OE_DON_STATUS_LOSS)
        {
            if (m_Result) m_Result.SetText("NOTHING -" + state.bet.ToString());
            if (m_CurrentValue) m_CurrentValue.SetText("0");
            if (m_NextValue) m_NextValue.SetText("-");
            if (m_StatusPrompt) m_StatusPrompt.SetText("RUN LOST - PLACE A NEW WAGER WHEN READY");
            return;
        }

        if (state.status == OE_DON_STATUS_CASHED)
        {
            if (m_Result) m_Result.SetText("CASHED +" + state.netResult.ToString());
            if (m_NextValue) m_NextValue.SetText("-");
            if (m_StatusPrompt) m_StatusPrompt.SetText("WIN PAID - PLACE A NEW WAGER WHEN READY");
            return;
        }

        if (state.status == OE_DON_STATUS_MAX_WIN)
        {
            if (m_Result) m_Result.SetText("PAID " + state.payout.ToString());
            if (m_CurrentLabel) m_CurrentLabel.SetText("PAID OUT");
            if (m_CurrentValue) m_CurrentValue.SetText(state.payout.ToString());
            if (m_NextValue) m_NextValue.SetText("-");
            if (m_StatusPrompt) m_StatusPrompt.SetText("MAXIMUM RUN REACHED - PAID AUTOMATICALLY");
        }
    }

    protected void ResetRunDisplay()
    {
        if (m_Stake) m_Stake.SetText("0");
        if (m_CurrentLabel) m_CurrentLabel.SetText("CURRENT VALUE");
        if (m_CurrentValue) m_CurrentValue.SetText("0");
        if (m_NextValue) m_NextValue.SetText("-");

        int maxDoubles = 6;
        if (OECasinoConfig.Instance && OECasinoConfig.Instance.doubleOrNothing)
            maxDoubles = OECasinoConfig.Instance.doubleOrNothing.maxDoubles;

        if (m_RunValue) m_RunValue.SetText("0 / " + maxDoubles.ToString());
    }

    protected void ShowStartControls()
    {
        if (m_StartPanel) m_StartPanel.Show(true);
        if (m_ColourChoicePanel) m_ColourChoicePanel.Show(false);
        if (m_ActionPanel) m_ActionPanel.Show(false);
        ClampSelectedBet();
        UpdateSelectedBetText();
    }

    protected void ShowColourControls()
    {
        if (m_StartPanel) m_StartPanel.Show(false);
        if (m_ColourChoicePanel) m_ColourChoicePanel.Show(true);
        if (m_ActionPanel) m_ActionPanel.Show(false);
    }

    protected void ShowActionControls()
    {
        if (m_StartPanel) m_StartPanel.Show(false);
        if (m_ColourChoicePanel) m_ColourChoicePanel.Show(false);
        if (m_ActionPanel) m_ActionPanel.Show(true);
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

        if (OECasinoConfig.Instance && OECasinoConfig.Instance.doubleOrNothing)
        {
            minBet = OECasinoConfig.Instance.doubleOrNothing.minBet;
            maxBet = OECasinoConfig.Instance.doubleOrNothing.maxBet;
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

    protected void LoadCardBack()
    {
        if (!m_ActiveCardBack) return;
        m_ActiveCardBack.LoadImageFile(0, "OperationExileCasino/data/cards/oe_card_back.edds");
        m_ActiveCardBack.SetImage(0);
    }

    protected void ShowCardBack()
    {
        if (m_ActiveCardSlot) m_ActiveCardSlot.Show(true);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(false);
        if (m_ActiveCardBack) m_ActiveCardBack.Show(true);
        if (m_ActiveCardFaceImage) m_ActiveCardFaceImage.Show(false);
        if (m_ActiveCardText)
        {
            m_ActiveCardText.SetText("");
            m_ActiveCardText.Show(false);
        }
    }

    protected void ShowCardFace(int rank, int suit)
    {
        string texturePath = GetCardFaceTexturePath(rank, suit);
        string cardText = GetCardDisplayText(rank, suit);

        if (m_ActiveCardSlot) m_ActiveCardSlot.Show(true);
        if (m_ActiveCardBack) m_ActiveCardBack.Show(false);
        if (m_ActiveCardFace) m_ActiveCardFace.Show(true);

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
            m_ActiveCardText.SetText(cardText);
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

    protected string GetCardDisplayText(int rank, int suit)
    {
        string rankName = rank.ToString();
        if (rank == 11) rankName = "JACK";
        else if (rank == 12) rankName = "QUEEN";
        else if (rank == 13) rankName = "KING";
        else if (rank == 14) rankName = "ACE";

        string suitName = "SPADES";
        if (suit == OE_CARD_SUIT_HEARTS) suitName = "HEARTS";
        else if (suit == OE_CARD_SUIT_DIAMONDS) suitName = "DIAMONDS";
        else if (suit == OE_CARD_SUIT_CLUBS) suitName = "CLUBS";

        string colourName = "BLACK";
        if (suit == OE_CARD_SUIT_HEARTS || suit == OE_CARD_SUIT_DIAMONDS)
            colourName = "RED";

        return rankName + " OF " + suitName + " - " + colourName;
    }

    protected void RequestState()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_SYNC, new Param1<string>(m_StationId), true);
    }

    protected void SendStart()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_START, new Param2<string, int>(m_StationId, m_SelectedBet), true);
    }

    protected void SendPickColour(int choice)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_PICK, new Param1<int>(choice), true);
    }

    protected void SendDouble()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_DOUBLE, new Param1<int>(0), true);
    }

    protected void SendCashOut()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_CASHOUT, new Param1<int>(0), true);
    }

    protected void CloseMenu()
    {
        GetGame().GetUIManager().Back();
    }
}
