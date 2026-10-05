class OESlotMachineSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 1000;
    int defaultBet = 100;
    int betStep = 10;

    int jackpotSeed = 5000;
    int jackpotMax = 50000;
    float jackpotContributionPercent = 10.0;
    int jackpotRequiresMaxBet = 1;

    // Legacy compatibility field from the temporary v0.15.7 test build.
    // Public builds ignore it and config validation always resets it to 0.
    int debugForceNextJackpot = 0;

    int pairReturnMultiplier = 1;
    int lemonTripleMultiplier = 5;
    int cherryTripleMultiplier = 8;
    int grapeTripleMultiplier = 12;
    int bellTripleMultiplier = 18;
    int barTripleMultiplier = 30;
    int sevenTripleMultiplier = 75;
}

class OESlotMachineJackpotData
{
    int amount = 5000;
}
