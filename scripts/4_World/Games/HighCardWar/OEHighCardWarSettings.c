class OEHighCardWarSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 5000;

    // Casino War style defaults.
    // Normal win: original wager wins even money (2x total return).
    float normalWinPayout = 2.0;

    // After a tie, GO TO WAR adds a second wager equal to the original.
    // A successful war returns 3x the ORIGINAL wager: the original pushes,
    // and the added war wager wins even money. Example 100 + 100 at risk -> 300 returned.
    float warWinReturnMultiplier = 3.0;

    // Surrender returns half of the original wager.
    float surrenderRefundMultiplier = 0.5;

    // Standard Casino War presentation burns three cards before the war cards.
    int warBurnCards = 3;

    // Vegas-style rule used by MGM: a second tie after going to war wins for the player.
    int secondTieWins = 1;

    // Number of ordinary 52-card decks shuffled together for this game.
    int deckCount = 6;
}
