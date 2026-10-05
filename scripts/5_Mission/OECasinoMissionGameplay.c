modded class MissionGameplay
{
    protected bool m_OECasinoConfigReceived;

    override void OnMissionStart()
    {
        super.OnMissionStart();
        GetDayZGame().Event_OnRPC.Insert(OECasinoHandleClientRPC);

        m_OECasinoConfigReceived = false;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.OECasinoRequestConfig, 1000, true);
    }

    override void OnMissionFinish()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.OECasinoRequestConfig);
        GetDayZGame().Event_OnRPC.Remove(OECasinoHandleClientRPC);
        super.OnMissionFinish();
    }

    protected void OECasinoRequestConfig()
    {
        if (m_OECasinoConfigReceived)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.OECasinoRequestConfig);
            return;
        }

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity()) return;

        GetGame().RPCSingleParam(player, OE_CASINO_RPC_CONFIG_REQUEST, new Param1<int>(0), true);
    }

    void OECasinoHandleClientRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == OE_CASINO_RPC_CONFIG_RESPONSE)
        {
            Param1<ref OECasinoConfig> configParam;
            if (ctx.Read(configParam))
            {
                OECasinoConfig.Instance = configParam.param1;
                m_OECasinoConfigReceived = true;
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.OECasinoRequestConfig);

                int stationCount = 0;
                if (OECasinoConfig.Instance && OECasinoConfig.Instance.stations)
                    stationCount = OECasinoConfig.Instance.stations.Count();

                Print("[OperationExileCasino] Client config received. Stations: " + stationCount.ToString());
            }
            return;
        }

        if (rpc_type == OE_CASINO_RPC_RTB_STATE)
        {
            Param1<ref OERideBusNetState> stateParam;
            if (!ctx.Read(stateParam)) return;

            OERideBusMenu menu = OECasinoMenuManager.Get().GetRideBusMenu();
            if (menu) menu.ApplyState(stateParam.param1);
        }

        if (rpc_type == OE_CASINO_RPC_BJ_STATE)
        {
            Param1<ref OEBlackjackNetState> bjStateParam;
            if (!ctx.Read(bjStateParam)) return;

            OEBlackjackMenu blackjackMenu = OECasinoMenuManager.Get().GetBlackjackMenu();
            if (blackjackMenu) blackjackMenu.ApplyState(bjStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_DICE_STATE)
        {
            Param1<ref OEDiceBettingNetState> diceStateParam;
            if (!ctx.Read(diceStateParam)) return;

            OEDiceBettingMenu diceMenu = OECasinoMenuManager.Get().GetDiceBettingMenu();
            if (diceMenu) diceMenu.ApplyState(diceStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_SPIN_STATE)
        {
            Param1<ref OESpinTheWinNetState> spinStateParam;
            if (!ctx.Read(spinStateParam)) return;

            OESpinTheWinMenu spinMenu = OECasinoMenuManager.Get().GetSpinTheWinMenu();
            if (spinMenu) spinMenu.ApplyState(spinStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_ROULETTE_STATE)
        {
            Param1<ref OERouletteNetState> rouletteStateParam;
            if (!ctx.Read(rouletteStateParam)) return;

            OERouletteMenu rouletteMenu = OECasinoMenuManager.Get().GetRouletteMenu();
            if (rouletteMenu) rouletteMenu.ApplyState(rouletteStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_WAR_STATE)
        {
            Param1<ref OEHighCardWarNetState> warStateParam;
            if (!ctx.Read(warStateParam)) return;

            OEHighCardWarMenu warMenu = OECasinoMenuManager.Get().GetHighCardWarMenu();
            if (warMenu) warMenu.ApplyState(warStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_CRAPS_STATE)
        {
            Param1<ref OECrapsNetState> crapsStateParam;
            if (!ctx.Read(crapsStateParam)) return;

            OECrapsMenu crapsMenu = OECasinoMenuManager.Get().GetCrapsMenu();
            if (crapsMenu) crapsMenu.ApplyState(crapsStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_DON_STATE)
        {
            Param1<ref OEDoubleOrNothingNetState> donStateParam;
            if (!ctx.Read(donStateParam)) return;

            OEDoubleOrNothingMenu donMenu = OECasinoMenuManager.Get().GetDoubleOrNothingMenu();
            if (donMenu) donMenu.ApplyState(donStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_HOLDEM_STATE)
        {
            Param1<ref OECasinoHoldemNetState> holdemStateParam;
            if (!ctx.Read(holdemStateParam)) return;

            OECasinoHoldemMenu holdemMenu = OECasinoMenuManager.Get().GetCasinoHoldemMenu();
            if (holdemMenu) holdemMenu.ApplyState(holdemStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_SLOT_STATE)
        {
            Param1<ref OESlotMachineNetState> slotStateParam;
            if (!ctx.Read(slotStateParam)) return;

            OESlotMachineMenu slotMenu = OECasinoMenuManager.Get().GetSlotMachineMenu();
            if (slotMenu) slotMenu.ApplyState(slotStateParam.param1);
            return;
        }


        if (rpc_type == OE_CASINO_RPC_GOFISH_STATE)
        {
            Param1<ref OEGoFishNetState> goFishStateParam;
            if (!ctx.Read(goFishStateParam)) return;

            OEGoFishMenu goFishMenu = OECasinoMenuManager.Get().GetGoFishMenu();
            if (goFishMenu) goFishMenu.ApplyState(goFishStateParam.param1);
            return;
        }

        if (rpc_type == OE_CASINO_RPC_SLOT_JACKPOT)
        {
            Param1<int> jackpotParam;
            if (!ctx.Read(jackpotParam)) return;

            OESlotMachineMenu jackpotMenu = OECasinoMenuManager.Get().GetSlotMachineMenu();
            if (jackpotMenu) jackpotMenu.ApplyJackpot(jackpotParam.param1);
            return;
        }
    }
}
