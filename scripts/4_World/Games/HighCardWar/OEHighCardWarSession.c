class OEHighCardWarSession
{
    string playerId;
    string stationId;
    int originalBet;
    int totalStake;
    bool finished;
    bool waitingOnTieDecision;
    bool warRound;

    ref OEHighCardWarDeck deck;
    ref OEPlayingCard playerCard;
    ref OEPlayingCard dealerCard;
    ref OEPlayingCard warPlayerCard;
    ref OEPlayingCard warDealerCard;

    int payout;
    int netResult;

    void OEHighCardWarSession(string uid, string station, int wager, int deckCount)
    {
        playerId = uid;
        stationId = station;
        originalBet = wager;
        totalStake = wager;
        finished = false;
        waitingOnTieDecision = false;
        warRound = false;
        deck = new OEHighCardWarDeck(deckCount);
        payout = 0;
        netResult = 0;
    }
}
