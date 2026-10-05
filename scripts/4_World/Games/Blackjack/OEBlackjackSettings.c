class OEBlackjackSettings
{
    bool enabled = true;
    int minBet = 10;
    int maxBet = 5000;

    // Total return including the player's original stake.
    // 2.0 = even-money win, 2.5 = traditional 3:2 blackjack.
    float normalWinPayout = 2.0;
    float blackjackPayout = 2.5;

    int allowDoubleDown = 1;
    bool dealerHitsSoft17 = false;
}
