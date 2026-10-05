class OERouletteSettings
{
    int enabled = 1;

    // Minimum/maximum value of a single chip placement.
    int minBet = 10;
    int maxBet = 5000;

    // Maximum combined stake across every placement on one spin.
    int maxTotalBet = 25000;

    // Total return multipliers, including the original stake.
    // American double-zero roulette standard table returns:
    // Straight 35:1, Split 17:1, Street 11:1, Corner 8:1,
    // Top Line 6:1, Six Line 5:1, Dozens/Columns 2:1, even-money 1:1.
    float straightPayout = 36.0;
    float splitPayout = 18.0;
    float streetPayout = 12.0;
    float cornerPayout = 9.0;
    float topLinePayout = 7.0;
    float sixLinePayout = 6.0;
    float dozenColumnPayout = 3.0;
    float evenMoneyPayout = 2.0;
}
