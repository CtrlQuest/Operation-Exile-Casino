class OERideBusSettings
{
    bool enabled = true;
    int minBet = 10;
    int maxBet = 5000;

    float round1Multiplier = 2.0;
    float round2Multiplier = 3.0;
    float round3Multiplier = 4.0;
    float round4Multiplier = 20.0;

    // Seconds available for each decision.
    int choiceSeconds = 8;

    // Timeout behaviour:
    // "lose"         = the wager is lost when time expires (default).
    // "first_choice" = automatically choose the first option for the round
    //                  (RED, HIGHER, INSIDE, HEARTS).
    string timeoutBehavior = "lose";

    // User-described rules: an equal rank is NOT higher or lower.
    bool equalCountsAsHigher = false;

    // Inside includes either boundary card, matching the supplied rules.
    bool insideIsInclusive = true;

    bool allowCashOut = true;
}
