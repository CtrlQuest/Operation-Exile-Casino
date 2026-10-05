modded class MissionServer
{
    protected ref OECasinoConfig m_OECasinoConfig;
    protected ref OERideBusServer m_OERideBusServer;
    protected ref OEBlackjackServer m_OEBlackjackServer;
    protected ref OEDiceBettingServer m_OEDiceBettingServer;
    protected ref OESpinTheWinServer m_OESpinTheWinServer;
    protected ref OERouletteServer m_OERouletteServer;
    protected ref OEHighCardWarServer m_OEHighCardWarServer;
    protected ref OECrapsServer m_OECrapsServer;
    protected ref OEDoubleOrNothingServer m_OEDoubleOrNothingServer;
    protected ref OECasinoHoldemServer m_OECasinoHoldemServer;
    protected ref OESlotMachineServer m_OESlotMachineServer;
    protected ref OEGoFishServer m_OEGoFishServer;

    override void OnInit()
    {
        super.OnInit();
        m_OECasinoConfig = OECasinoConfig.LoadServer();
        m_OERideBusServer = new OERideBusServer(m_OECasinoConfig);
        m_OEBlackjackServer = new OEBlackjackServer(m_OECasinoConfig);
        m_OEDiceBettingServer = new OEDiceBettingServer(m_OECasinoConfig);
        m_OESpinTheWinServer = new OESpinTheWinServer(m_OECasinoConfig);
        m_OERouletteServer = new OERouletteServer(m_OECasinoConfig);
        m_OEHighCardWarServer = new OEHighCardWarServer(m_OECasinoConfig);
        m_OECrapsServer = new OECrapsServer(m_OECasinoConfig);
        m_OEDoubleOrNothingServer = new OEDoubleOrNothingServer(m_OECasinoConfig);
        m_OECasinoHoldemServer = new OECasinoHoldemServer(m_OECasinoConfig);
        m_OESlotMachineServer = new OESlotMachineServer(m_OECasinoConfig);
        m_OEGoFishServer = new OEGoFishServer(m_OECasinoConfig);
        GetDayZGame().Event_OnRPC.Insert(OECasinoHandleRPC);

        OECasinoScheduleReload();
    }

    void ~MissionServer()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.OECasinoReloadConfig);
        GetDayZGame().Event_OnRPC.Remove(OECasinoHandleRPC);
    }

    protected void OECasinoScheduleReload()
    {
        if (!m_OECasinoConfig || !m_OECasinoConfig.hotReload) return;

        int reloadMs = m_OECasinoConfig.hotReloadSeconds * 1000;
        if (reloadMs < 2000) reloadMs = 5000;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.OECasinoReloadConfig, reloadMs, false);
    }

    protected void OECasinoReloadConfig()
    {
        OECasinoConfig updatedConfig = OECasinoConfig.LoadServer();
        if (!updatedConfig)
        {
            OECasinoScheduleReload();
            return;
        }

        m_OECasinoConfig = updatedConfig;

        if (m_OERideBusServer)
            m_OERideBusServer.ApplyConfig(m_OECasinoConfig);
        if (m_OEBlackjackServer)
            m_OEBlackjackServer.ApplyConfig(m_OECasinoConfig);
        if (m_OEDiceBettingServer)
            m_OEDiceBettingServer.ApplyConfig(m_OECasinoConfig);
        if (m_OESpinTheWinServer)
            m_OESpinTheWinServer.ApplyConfig(m_OECasinoConfig);
        if (m_OERouletteServer)
            m_OERouletteServer.ApplyConfig(m_OECasinoConfig);
        if (m_OEHighCardWarServer)
            m_OEHighCardWarServer.ApplyConfig(m_OECasinoConfig);
        if (m_OECrapsServer)
            m_OECrapsServer.ApplyConfig(m_OECasinoConfig);
        if (m_OEDoubleOrNothingServer)
            m_OEDoubleOrNothingServer.ApplyConfig(m_OECasinoConfig);
        if (m_OECasinoHoldemServer)
            m_OECasinoHoldemServer.ApplyConfig(m_OECasinoConfig);
        if (m_OESlotMachineServer)
            m_OESlotMachineServer.ApplyConfig(m_OECasinoConfig);
        if (m_OEGoFishServer)
            m_OEGoFishServer.ApplyConfig(m_OECasinoConfig);

        OECasinoBroadcastConfig();
        OECasinoScheduleReload();
    }

    protected void OECasinoBroadcastConfig()
    {
        if (!m_OECasinoConfig) return;

        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player || !player.GetIdentity()) continue;

            GetGame().RPCSingleParam(player, OE_CASINO_RPC_CONFIG_RESPONSE, new Param1<ref OECasinoConfig>(m_OECasinoConfig), true, player.GetIdentity());
        }
    }

    void OECasinoHandleRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == OE_CASINO_RPC_CONFIG_REQUEST)
        {
            PlayerBase player = PlayerBase.Cast(target);
            if (sender && player && player.GetIdentity() && player.GetIdentity().GetId() == sender.GetId())
            {
                GetGame().RPCSingleParam(player, OE_CASINO_RPC_CONFIG_RESPONSE, new Param1<ref OECasinoConfig>(m_OECasinoConfig), true, sender);
            }
            return;
        }

        if (m_OERideBusServer)
        {
            if (rpc_type == OE_CASINO_RPC_RTB_SYNC) { m_OERideBusServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_RTB_START) { m_OERideBusServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_RTB_CHOICE) { m_OERideBusServer.HandleChoice(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_RTB_CASHOUT) { m_OERideBusServer.HandleCashOut(sender, target); return; }
        }

        if (m_OEBlackjackServer)
        {
            if (rpc_type == OE_CASINO_RPC_BJ_SYNC) { m_OEBlackjackServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_BJ_START) { m_OEBlackjackServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_BJ_ACTION) { m_OEBlackjackServer.HandleAction(sender, target, ctx); return; }
        }

        if (m_OEDiceBettingServer)
        {
            if (rpc_type == OE_CASINO_RPC_DICE_SYNC) { m_OEDiceBettingServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_DICE_ROLL) { m_OEDiceBettingServer.HandleRoll(sender, target, ctx); return; }
        }

        if (m_OESpinTheWinServer)
        {
            if (rpc_type == OE_CASINO_RPC_SPIN_SYNC) { m_OESpinTheWinServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_SPIN_START) { m_OESpinTheWinServer.HandleSpin(sender, target, ctx); return; }
        }

        if (m_OERouletteServer)
        {
            if (rpc_type == OE_CASINO_RPC_ROULETTE_SYNC) { m_OERouletteServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_ROULETTE_SPIN) { m_OERouletteServer.HandleSpin(sender, target, ctx); return; }
        }

        if (m_OEHighCardWarServer)
        {
            if (rpc_type == OE_CASINO_RPC_WAR_SYNC) { m_OEHighCardWarServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_WAR_START) { m_OEHighCardWarServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_WAR_ACTION) { m_OEHighCardWarServer.HandleAction(sender, target, ctx); return; }
        }

        if (m_OECrapsServer)
        {
            if (rpc_type == OE_CASINO_RPC_CRAPS_SYNC) { m_OECrapsServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_CRAPS_START) { m_OECrapsServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_CRAPS_ROLL) { m_OECrapsServer.HandleRoll(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_CRAPS_BET_ACTION) { m_OECrapsServer.HandleBetAction(sender, target, ctx); return; }
        }

        if (m_OEDoubleOrNothingServer)
        {
            if (rpc_type == OE_CASINO_RPC_DON_SYNC) { m_OEDoubleOrNothingServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_DON_START) { m_OEDoubleOrNothingServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_DON_DOUBLE) { m_OEDoubleOrNothingServer.HandleDouble(sender, target); return; }
            if (rpc_type == OE_CASINO_RPC_DON_CASHOUT) { m_OEDoubleOrNothingServer.HandleCashOut(sender, target); return; }
            if (rpc_type == OE_CASINO_RPC_DON_PICK) { m_OEDoubleOrNothingServer.HandlePickColour(sender, target, ctx); return; }
        }

        if (m_OECasinoHoldemServer)
        {
            if (rpc_type == OE_CASINO_RPC_HOLDEM_SYNC) { m_OECasinoHoldemServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_HOLDEM_START) { m_OECasinoHoldemServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_HOLDEM_ACTION) { m_OECasinoHoldemServer.HandleAction(sender, target, ctx); return; }
        }

        if (m_OESlotMachineServer)
        {
            if (rpc_type == OE_CASINO_RPC_SLOT_SYNC) { m_OESlotMachineServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_SLOT_SPIN) { m_OESlotMachineServer.HandleSpin(sender, target, ctx); return; }
        }

        if (m_OEGoFishServer)
        {
            if (rpc_type == OE_CASINO_RPC_GOFISH_SYNC) { m_OEGoFishServer.HandleSync(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_GOFISH_START) { m_OEGoFishServer.HandleStart(sender, target, ctx); return; }
            if (rpc_type == OE_CASINO_RPC_GOFISH_ASK) { m_OEGoFishServer.HandleAsk(sender, target, ctx); return; }
        }
    }
}
