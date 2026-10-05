class OEDoubleOrNothingSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 5000;

    // Red vs Black uses a standard 52-card colour split: 26 red / 26 black.
    // This field is retained for backwards-compatible JSON configs and is
    // normalized to 50.0 by OECasinoConfig.
    float winChancePercent = 50.0;

    // Maximum number of successful doubles in one run.
    // At this point the server automatically pays the player to cap exposure.
    int maxDoubles = 6;

    // Hard cap for a single cash-out. This protects against accidentally huge
    // values if maxDoubles or wager limits are increased in configuration.
    int maxPayout = 500000;
}
