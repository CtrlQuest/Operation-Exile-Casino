class OESlotMachineNetState
{
    int status = OE_SLOT_STATUS_IDLE;
    int bet = 0;
    int balance = 0;
    int reel1 = OE_SLOT_SYMBOL_LEMON;
    int reel2 = OE_SLOT_SYMBOL_CHERRY;
    int reel3 = OE_SLOT_SYMBOL_GRAPE;
    int payout = 0;
    int netResult = 0;
    int jackpot = 0;
    bool animate = false;
    bool jackpotEligible = false;
    string stationId = "";
    string resultLabel = "";
    string message = "";
}
