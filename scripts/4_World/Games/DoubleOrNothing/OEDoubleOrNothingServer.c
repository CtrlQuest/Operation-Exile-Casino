class OEDoubleOrNothingServer
{
    protected ref OECasinoConfig m_Config;
    protected ref OECasinoCurrency m_Currency;
    protected ref map<string, ref OEDoubleOrNothingSession> m_Sessions;

    void OEDoubleOrNothingServer(OECasinoConfig config)
    {
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
        m_Sessions = new map<string, ref OEDoubleOrNothingSession>;
    }

    void ApplyConfig(OECasinoConfig config)
    {
        if (!config) return;
        m_Config = config;
        m_Currency = new OECasinoCurrency(config.currencyValues);
    }

    void HandleSync(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<string> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        OECasinoStation station = ValidateStation(player, stationId);
        if (!station || station.game != OE_CASINO_GAME_DOUBLE_OR_NOTHING || !m_Config.doubleOrNothing || m_Config.doubleOrNothing.enabled != 1)
        {
            SendError(player, stationId, "This Double or Nothing table is not available.");
            return;
        }

        OEDoubleOrNothingSession session = GetActiveSession(sender);
        if (session)
        {
            if (session.stationId != stationId)
            {
                SendRejected(player, stationId, "You already have an active Double or Nothing run at another table.");
                return;
            }

            if (session.awaitingColour)
            {
                SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "Your run is committed. Pick RED or BLACK to reveal the next card.");
                return;
            }

            SendSessionState(player, session, OE_DON_STATUS_ACTIVE, "Your run is still active. CASH OUT or DOUBLE AGAIN.");
            return;
        }

        SendIdleState(player, stationId);
    }

    void HandleStart(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param2<string, int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        string stationId = req.param1;
        int bet = req.param2;
        OECasinoStation station = ValidateStation(player, stationId);

        if (!station || station.game != OE_CASINO_GAME_DOUBLE_OR_NOTHING || !m_Config.doubleOrNothing || m_Config.doubleOrNothing.enabled != 1)
        {
            SendError(player, stationId, "This Double or Nothing table is not available.");
            return;
        }

        if (bet < m_Config.doubleOrNothing.minBet || bet > m_Config.doubleOrNothing.maxBet)
        {
            SendRejected(player, stationId, "Wager must be between " + m_Config.doubleOrNothing.minBet.ToString() + " and " + m_Config.doubleOrNothing.maxBet.ToString() + " chips.");
            return;
        }

        OEDoubleOrNothingSession active = GetActiveSession(sender);
        if (active)
        {
            if (active.awaitingColour)
                SendSessionState(player, active, OE_DON_STATUS_CHOOSE, "Finish your current colour choice first.");
            else
                SendSessionState(player, active, OE_DON_STATUS_ACTIVE, "Finish or cash out your current run first.");
            return;
        }

        if (!m_Currency.TryDebit(player, bet))
        {
            SendRejected(player, stationId, "Not enough casino currency. Your wager was not placed.");
            return;
        }

        OEDoubleOrNothingSession session = new OEDoubleOrNothingSession(sender.GetId(), stationId, bet);
        m_Sessions.Set(sender.GetId(), session);

        SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "Wager locked. Pick RED or BLACK. Match the card colour to double to " + SafeDouble(session.currentValue).ToString() + " chips.");
    }

    void HandleDouble(PlayerIdentity sender, Object target)
    {
        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEDoubleOrNothingSession session = GetActiveSession(sender);
        if (!session)
        {
            SendRejected(player, "", "No active Double or Nothing run.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_DOUBLE_OR_NOTHING)
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (session.awaitingColour)
        {
            SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "This risk is already committed. Pick RED or BLACK.");
            return;
        }

        int maxDoubles = GetConfiguredMaxDoubles();
        if (session.successfulDoubles >= maxDoubles || session.currentValue >= GetConfiguredMaxPayout())
        {
            CashOut(player, session, true);
            return;
        }

        session.PrepareColourChoice();
        SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "DOUBLE AGAIN committed. Pick RED or BLACK. Lose this card and the full run is gone.");
    }

    void HandlePickColour(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Param1<int> req;
        if (!ctx.Read(req)) return;

        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEDoubleOrNothingSession session = GetActiveSession(sender);
        if (!session)
        {
            SendRejected(player, "", "No active Double or Nothing run.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_DOUBLE_OR_NOTHING)
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (!session.awaitingColour)
        {
            SendSessionState(player, session, OE_DON_STATUS_ACTIVE, "That card has already been resolved. CASH OUT or DOUBLE AGAIN.");
            return;
        }

        int choice = req.param1;
        if (choice != OE_DON_CHOICE_RED && choice != OE_DON_CHOICE_BLACK)
        {
            SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "Choose RED or BLACK.");
            return;
        }

        ResolveColourRisk(player, session, choice);
    }

    void HandleCashOut(PlayerIdentity sender, Object target)
    {
        PlayerBase player = ResolvePlayer(sender, target);
        if (!player) return;

        OEDoubleOrNothingSession session = GetActiveSession(sender);
        if (!session)
        {
            SendRejected(player, "", "No active Double or Nothing run to cash out.");
            return;
        }

        OECasinoStation station = ValidateStation(player, session.stationId);
        if (!station || station.game != OE_CASINO_GAME_DOUBLE_OR_NOTHING)
        {
            SendError(player, session.stationId, "You moved too far away from the table.");
            return;
        }

        if (session.awaitingColour || session.successfulDoubles < 1)
        {
            SendSessionState(player, session, OE_DON_STATUS_CHOOSE, "This risk is committed. Pick RED or BLACK before you can cash out.");
            return;
        }

        CashOut(player, session, false);
    }

    protected void ResolveColourRisk(PlayerBase player, OEDoubleOrNothingSession session, int choice)
    {
        if (!player || !session || session.finished || !session.awaitingColour) return;

        int maxDoubles = GetConfiguredMaxDoubles();
        if (session.successfulDoubles >= maxDoubles)
        {
            CashOut(player, session, true);
            return;
        }

        // Each risk uses a fresh standard-card draw. With two red suits and two
        // black suits this is an exact 50/50 colour game while still revealing
        // a real rank and suit using the casino's existing card artwork.
        int rank = Math.RandomInt(2, 15);
        int suit = Math.RandomInt(0, 4);
        bool cardIsRed = suit == OE_CARD_SUIT_HEARTS || suit == OE_CARD_SUIT_DIAMONDS;
        bool choseRed = choice == OE_DON_CHOICE_RED;
        bool won = cardIsRed == choseRed;

        session.awaitingColour = false;
        session.chosenColour = choice;
        session.cardRank = rank;
        session.cardSuit = suit;

        string choiceName = GetColourName(choice);
        string cardColour = "BLACK";
        if (cardIsRed) cardColour = "RED";
        string cardName = GetCardDisplayName(rank, suit);

        if (!won)
        {
            int riskedValue = session.currentValue;
            session.currentValue = 0;
            session.finished = true;
            session.payout = 0;
            session.netResult = 0 - session.originalBet;

            SendSessionState(player, session, OE_DON_STATUS_LOSS, "You picked " + choiceName + ". " + cardName + " is " + cardColour + ". NOTHING - the " + riskedValue.ToString() + " chip run is gone.");
            LogResult(player, session, "loss");
            return;
        }

        session.currentValue = SafeDouble(session.currentValue);
        session.successfulDoubles++;
        session.netResult = session.currentValue - session.originalBet;

        int maxPayout = GetConfiguredMaxPayout();
        bool reachedLimit = session.successfulDoubles >= maxDoubles || session.currentValue >= maxPayout;
        if (reachedLimit)
        {
            if (session.currentValue > maxPayout)
                session.currentValue = maxPayout;

            CashOut(player, session, true);
            return;
        }

        SendSessionState(player, session, OE_DON_STATUS_ACTIVE, "You picked " + choiceName + ". " + cardName + " is " + cardColour + " - DOUBLE! Take " + session.currentValue.ToString() + " or risk it for " + SafeDouble(session.currentValue).ToString() + ".");
    }

    protected void CashOut(PlayerBase player, OEDoubleOrNothingSession session, bool automatic)
    {
        if (!player || !session || session.finished) return;

        int payout = session.currentValue;
        int maxPayout = GetConfiguredMaxPayout();
        if (payout > maxPayout) payout = maxPayout;

        if (!m_Currency.Credit(player, payout))
        {
            // Keep the run active so the player can try the cash-out again rather
            // than losing a valid win because chip creation failed.
            session.awaitingColour = false;
            SendSessionState(player, session, OE_DON_STATUS_ACTIVE, "Cash-out could not be created. Your run is still active; try again or contact an admin.");
            return;
        }

        session.payout = payout;
        session.currentValue = payout;
        session.netResult = payout - session.originalBet;
        session.finished = true;
        session.awaitingColour = false;

        if (automatic)
            SendSessionState(player, session, OE_DON_STATUS_MAX_WIN, "Maximum run reached. Automatically paid " + payout.ToString() + " chips.");
        else
            SendSessionState(player, session, OE_DON_STATUS_CASHED, "CASHED OUT. Paid " + payout.ToString() + " chips.");

        if (automatic)
            LogResult(player, session, "max_cashout");
        else
            LogResult(player, session, "cashout");
    }

    protected int SafeDouble(int value)
    {
        if (value < 1) return 1;

        int cap = GetConfiguredMaxPayout();
        if (value >= cap) return cap;

        // Avoid integer overflow and never show a next value above the configured cap.
        if (value > (cap / 2)) return cap;
        return value * 2;
    }

    protected float GetConfiguredWinChance()
    {
        // Red vs Black is deliberately a true 50/50 game.
        return 50.0;
    }

    protected int GetConfiguredMaxDoubles()
    {
        int value = 6;
        if (m_Config && m_Config.doubleOrNothing)
            value = m_Config.doubleOrNothing.maxDoubles;

        if (value < 1) value = 1;
        if (value > 12) value = 12;
        return value;
    }

    protected int GetConfiguredMaxPayout()
    {
        int value = 500000;
        if (m_Config && m_Config.doubleOrNothing)
            value = m_Config.doubleOrNothing.maxPayout;

        if (value < 1) value = 500000;
        return value;
    }

    protected OEDoubleOrNothingSession GetActiveSession(PlayerIdentity sender)
    {
        if (!sender || !m_Sessions) return null;
        OEDoubleOrNothingSession session;
        if (!m_Sessions.Find(sender.GetId(), session)) return null;
        if (!session || session.finished) return null;
        return session;
    }

    protected PlayerBase ResolvePlayer(PlayerIdentity sender, Object target)
    {
        PlayerBase player = PlayerBase.Cast(target);
        if (!sender || !player || !player.GetIdentity()) return null;
        if (player.GetIdentity().GetId() != sender.GetId()) return null;
        return player;
    }

    protected OECasinoStation ValidateStation(PlayerBase player, string stationId)
    {
        if (!m_Config || !player) return null;
        OECasinoStation station = m_Config.GetStationById(stationId);
        if (!station) return null;
        if (vector.Distance(player.GetPosition(), station.position) > station.playDistance) return null;
        return station;
    }

    protected void SendIdleState(PlayerBase player, string stationId)
    {
        OEDoubleOrNothingNetState state = new OEDoubleOrNothingNetState();
        state.status = OE_DON_STATUS_IDLE;
        state.balance = m_Currency.GetBalance(player);
        state.maxDoubles = GetConfiguredMaxDoubles();
        state.winChancePercent = GetConfiguredWinChance();
        state.stationId = stationId;
        state.message = "Set your wager, then pick RED or BLACK to reveal a card and double your chips.";
        SendState(player, state);
    }

    protected void SendSessionState(PlayerBase player, OEDoubleOrNothingSession session, int status, string text)
    {
        OEDoubleOrNothingNetState state = new OEDoubleOrNothingNetState();
        state.status = status;
        state.bet = session.originalBet;
        state.balance = m_Currency.GetBalance(player);
        state.currentValue = session.currentValue;
        state.nextValue = SafeDouble(session.currentValue);
        state.successfulDoubles = session.successfulDoubles;
        state.maxDoubles = GetConfiguredMaxDoubles();
        state.winChancePercent = GetConfiguredWinChance();
        state.payout = session.payout;
        state.netResult = session.netResult;
        state.canDouble = status == OE_DON_STATUS_ACTIVE && !session.finished && !session.awaitingColour;
        state.canCashOut = status == OE_DON_STATUS_ACTIVE && !session.finished && !session.awaitingColour;
        state.awaitingColour = session.awaitingColour;
        state.chosenColour = session.chosenColour;
        state.cardRank = session.cardRank;
        state.cardSuit = session.cardSuit;
        state.stationId = session.stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendRejected(PlayerBase player, string stationId, string text)
    {
        OEDoubleOrNothingNetState state = new OEDoubleOrNothingNetState();
        state.status = OE_DON_STATUS_REJECTED;
        state.balance = m_Currency.GetBalance(player);
        state.maxDoubles = GetConfiguredMaxDoubles();
        state.winChancePercent = GetConfiguredWinChance();
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected void SendError(PlayerBase player, string stationId, string text)
    {
        OEDoubleOrNothingNetState state = new OEDoubleOrNothingNetState();
        state.status = OE_DON_STATUS_ERROR;
        state.balance = m_Currency.GetBalance(player);
        state.maxDoubles = GetConfiguredMaxDoubles();
        state.winChancePercent = GetConfiguredWinChance();
        state.stationId = stationId;
        state.message = text;
        SendState(player, state);
    }

    protected string GetColourName(int choice)
    {
        if (choice == OE_DON_CHOICE_RED) return "RED";
        if (choice == OE_DON_CHOICE_BLACK) return "BLACK";
        return "UNKNOWN";
    }

    protected string GetCardDisplayName(int rank, int suit)
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

        return rankName + " OF " + suitName;
    }

    protected void SendState(PlayerBase player, OEDoubleOrNothingNetState state)
    {
        if (!player || !player.GetIdentity() || !state) return;
        GetGame().RPCSingleParam(player, OE_CASINO_RPC_DON_STATE, new Param1<ref OEDoubleOrNothingNetState>(state), true, player.GetIdentity());
    }

    protected void LogResult(PlayerBase player, OEDoubleOrNothingSession session, string outcome)
    {
        if (!m_Config || !m_Config.enablePlayLogs || !player || !player.GetIdentity()) return;
        if (!FileExist(OE_CASINO_PROFILE_DIR)) MakeDirectory(OE_CASINO_PROFILE_DIR);

        FileHandle file = OpenFile(OE_CASINO_PROFILE_DIR + "plays.log", FileMode.APPEND);
        if (file == 0) return;

        string choiceName = GetColourName(session.chosenColour);
        string cardName = "NONE";
        if (session.cardRank >= 2 && session.cardSuit >= 0)
            cardName = GetCardDisplayName(session.cardRank, session.cardSuit);

        FPrintln(file, "DOUBLE_OR_NOTHING | " + player.GetIdentity().GetPlainId() + " | bet=" + session.originalBet + " | doubles=" + session.successfulDoubles + " | payout=" + session.payout + " | net=" + session.netResult + " | choice=" + choiceName + " | card=" + cardName + " | outcome=" + outcome + " | station=" + session.stationId);
        CloseFile(file);
    }
}
