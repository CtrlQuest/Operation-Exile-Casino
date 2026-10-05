class OEDiceBettingSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 5000;

    // Total return multipliers, including the original stake.
    float highLowPayout = 2.0;
    float oddEvenPayout = 2.0;
    float doublesPayout = 6.0;

    // Exact two-dice total payouts. These are also total returns.
    float exact2or12Payout = 31.0;
    float exact3or11Payout = 16.0;
    float exact4or10Payout = 11.0;
    float exact5or9Payout = 8.0;
    float exact6or8Payout = 6.0;
    float exact7Payout = 5.0;
}
