class OECrapsSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 5000;

    // Total returns including the original stake. Standard Pass / Don't Pass
    // bets pay even money (1:1), so a winning 100 wager returns 200.
    float passLineReturnMultiplier = 2.0;
    float dontPassReturnMultiplier = 2.0;

    // Side-bet controls for the expanded table.
    int sideMinBet = 10;
    int maxSideBet = 5000;
    int maxSideBetTotal = 25000;

    // Keeps the alpha table readable and stops oversized Odds positions.
    // True-odds payout is still calculated from the actual point number.
    float maxOddsMultiplier = 5.0;
}
