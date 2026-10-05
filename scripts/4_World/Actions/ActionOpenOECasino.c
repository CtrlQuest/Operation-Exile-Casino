class ActionOpenOECasino : ActionInteractBase
{
    protected string m_LastPromptText = "";

    override string GetText()
    {
        // Resolve the nearby station on the client so the DayZ action prompt
        // names the actual game instead of using one generic casino label.
        PlayerBase player;
        if (GetGame()) player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player && OECasinoConfig.Instance)
        {
            OECasinoStation station = OECasinoConfig.Instance.GetStationNearPosition(player.GetPosition());
            if (station)
            {
                m_LastPromptText = "Play " + GetGameDisplayName(station.game);
                return m_LastPromptText;
            }
        }

        // DayZ can keep the action hint alive for a frame while ActionCondition
        // transitions to false. Keep the last valid station name during that
        // short fade instead of flashing the generic casino prompt.
        if (m_LastPromptText != "")
            return m_LastPromptText;

        return "Play casino game";
    }

    protected string GetGameDisplayName(string gameId)
    {
        if (gameId == OE_CASINO_GAME_RIDE_BUS) return "Ride the Bus";
        if (gameId == OE_CASINO_GAME_BLACKJACK) return "Blackjack";
        if (gameId == OE_CASINO_GAME_DICE_BETTING) return "Dice Betting";
        if (gameId == OE_CASINO_GAME_SPIN_THE_WIN) return "Spin the Win";
        if (gameId == OE_CASINO_GAME_ROULETTE) return "Roulette";
        if (gameId == OE_CASINO_GAME_HIGH_CARD_WAR) return "High Card / War";
        if (gameId == OE_CASINO_GAME_CRAPS) return "Craps";
        if (gameId == OE_CASINO_GAME_DOUBLE_OR_NOTHING) return "Double or Nothing";
        if (gameId == OE_CASINO_GAME_CASINO_HOLDEM) return "Texas Hold'em";
        if (gameId == OE_CASINO_GAME_SLOT_MACHINE) return "Slot Machine";
        if (gameId == OE_CASINO_GAME_GO_FISH) return "Go Fish";
        return "Casino Game";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTNone;
    }

    override bool HasTarget()
    {
        return false;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player) return false;
        if (!OECasinoConfig.Instance) return false;

        return OECasinoConfig.Instance.GetStationNearPosition(player.GetPosition()) != null;
    }

    override void OnStartClient(ActionData action_data)
    {
        super.OnStartClient(action_data);
        if (!action_data || !action_data.m_Player || !OECasinoConfig.Instance) return;

        OECasinoStation station = OECasinoConfig.Instance.GetStationNearPosition(action_data.m_Player.GetPosition());
        if (station)
        {
            OECasinoMenuManager.Get().OpenStation(station);
        }
    }
}

modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(ActionOpenOECasino);
    }
}

modded class PlayerBase
{
    override void SetActions(out TInputActionMap InputActionMap)
    {
        super.SetActions(InputActionMap);
        AddAction(ActionOpenOECasino, InputActionMap);
    }
}
