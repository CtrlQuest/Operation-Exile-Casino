class OECasinoMenuManager
{
    protected static ref OECasinoMenuManager s_Instance;
    protected ref OERideBusMenu m_RideBus;
    protected ref OEBlackjackMenu m_Blackjack;
    protected ref OEDiceBettingMenu m_DiceBetting;
    protected ref OESpinTheWinMenu m_SpinTheWin;
    protected ref OERouletteMenu m_Roulette;
    protected ref OEHighCardWarMenu m_HighCardWar;
    protected ref OECrapsMenu m_Craps;
    protected ref OEDoubleOrNothingMenu m_DoubleOrNothing;
    protected ref OECasinoHoldemMenu m_CasinoHoldem;
    protected ref OESlotMachineMenu m_SlotMachine;
    protected ref OEGoFishMenu m_GoFish;

    static OECasinoMenuManager Get()
    {
        if (!s_Instance) s_Instance = new OECasinoMenuManager();
        return s_Instance;
    }

    void OpenStation(OECasinoStation station)
    {
        if (!station || GetGame().GetUIManager().GetMenu()) return;

        if (station.game == OE_CASINO_GAME_RIDE_BUS)
        {
            if (!m_RideBus) m_RideBus = new OERideBusMenu();
            m_RideBus.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_RideBus, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_BLACKJACK)
        {
            if (!m_Blackjack) m_Blackjack = new OEBlackjackMenu();
            m_Blackjack.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_Blackjack, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_DICE_BETTING)
        {
            if (!m_DiceBetting) m_DiceBetting = new OEDiceBettingMenu();
            m_DiceBetting.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_DiceBetting, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_SPIN_THE_WIN)
        {
            if (!m_SpinTheWin) m_SpinTheWin = new OESpinTheWinMenu();
            m_SpinTheWin.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_SpinTheWin, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_ROULETTE)
        {
            if (!m_Roulette) m_Roulette = new OERouletteMenu();
            m_Roulette.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_Roulette, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_HIGH_CARD_WAR)
        {
            if (!m_HighCardWar) m_HighCardWar = new OEHighCardWarMenu();
            m_HighCardWar.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_HighCardWar, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_CRAPS)
        {
            if (!m_Craps) m_Craps = new OECrapsMenu();
            m_Craps.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_Craps, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_DOUBLE_OR_NOTHING)
        {
            if (!m_DoubleOrNothing) m_DoubleOrNothing = new OEDoubleOrNothingMenu();
            m_DoubleOrNothing.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_DoubleOrNothing, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_CASINO_HOLDEM)
        {
            if (!m_CasinoHoldem) m_CasinoHoldem = new OECasinoHoldemMenu();
            m_CasinoHoldem.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_CasinoHoldem, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_SLOT_MACHINE)
        {
            if (!m_SlotMachine) m_SlotMachine = new OESlotMachineMenu();
            m_SlotMachine.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_SlotMachine, null);
            return;
        }

        if (station.game == OE_CASINO_GAME_GO_FISH)
        {
            if (!m_GoFish) m_GoFish = new OEGoFishMenu();
            m_GoFish.SetStation(station.id);
            GetGame().GetUIManager().ShowScriptedMenu(m_GoFish, null);
            return;
        }
    }

    OERideBusMenu GetRideBusMenu()
    {
        return m_RideBus;
    }

    OEBlackjackMenu GetBlackjackMenu()
    {
        return m_Blackjack;
    }

    OEDiceBettingMenu GetDiceBettingMenu()
    {
        return m_DiceBetting;
    }

    OESpinTheWinMenu GetSpinTheWinMenu()
    {
        return m_SpinTheWin;
    }

    OERouletteMenu GetRouletteMenu()
    {
        return m_Roulette;
    }

    OEHighCardWarMenu GetHighCardWarMenu()
    {
        return m_HighCardWar;
    }

    OECrapsMenu GetCrapsMenu()
    {
        return m_Craps;
    }

    OEDoubleOrNothingMenu GetDoubleOrNothingMenu()
    {
        return m_DoubleOrNothing;
    }

    OECasinoHoldemMenu GetCasinoHoldemMenu()
    {
        return m_CasinoHoldem;
    }

    OESlotMachineMenu GetSlotMachineMenu()
    {
        return m_SlotMachine;
    }

    OEGoFishMenu GetGoFishMenu()
    {
        return m_GoFish;
    }
}
