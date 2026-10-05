class OECasinoHoldemSettings
{
    int enabled = 1;
    int minBet = 10;
    int maxBet = 5000;

    // Heads-up dealer AI tuning. The dealer never sees the player's hidden
    // cards; decisions are based only on its own hole cards and the public board.
    int dealerBluffPercent = 12;
    int dealerAggressionPercent = 55;

    // v0.14.0: no-limit player betting. The player can raise up to the chips
    // they actually have available after covering any call. The dealer can
    // still vary its own raise size up to dealerMaxRaiseMultiplier x opening bet.
    bool noLimit = true;
    int dealerMaxRaiseMultiplier = 3;

    // Used only when noLimit is disabled. Kept for servers that prefer a
    // restricted betting structure.
    int maxRaisesPerStreet = 2;

    // Legacy v0.13.x field. Retained so old JSON configs continue to load.
    int maxRaiseMultiplier = 3;

    // v0.13.0 legacy fields kept so existing CasinoConfig.json files can load
    // cleanly. They are no longer used by the heads-up Texas Hold'em game.
    int callBetMultiplier = 2;
    int dealerQualifyPairRank = 4;
    float anteRoyalFlushReturn = 101.0;
    float anteStraightFlushReturn = 21.0;
    float anteFourKindReturn = 11.0;
    float anteFullHouseReturn = 4.0;
    float anteFlushReturn = 3.0;
    float anteStraightReturn = 2.0;
    float anteTripsReturn = 2.0;
    float anteTwoPairReturn = 2.0;
    float antePairReturn = 2.0;
    float anteHighCardReturn = 2.0;
    float callWinReturn = 2.0;
}
