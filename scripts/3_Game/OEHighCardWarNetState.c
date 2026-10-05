class OEHighCardWarNetState
{
    int status;
    int bet;
    int totalStake;
    int balance;
    int payout;
    int netResult;

    int playerRank;
    int playerSuit;
    int dealerRank;
    int dealerSuit;

    int warPlayerRank;
    int warPlayerSuit;
    int warDealerRank;
    int warDealerSuit;

    bool canSurrender;
    bool canGoToWar;
    bool warRound;

    string stationId;
    string message;

    void OEHighCardWarNetState()
    {
        status = OE_WAR_STATUS_IDLE;
        bet = 0;
        totalStake = 0;
        balance = 0;
        payout = 0;
        netResult = 0;

        playerRank = 0;
        playerSuit = 0;
        dealerRank = 0;
        dealerSuit = 0;
        warPlayerRank = 0;
        warPlayerSuit = 0;
        warDealerRank = 0;
        warDealerSuit = 0;

        canSurrender = false;
        canGoToWar = false;
        warRound = false;
        stationId = "";
        message = "";
    }
}
