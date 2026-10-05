class OEBlackjackSplitSettlement
{
    int payout;
    string shortResult;
    string detail;

    void OEBlackjackSplitSettlement()
    {
        payout = 0;
        shortResult = "";
        detail = "";
    }
}

class OEBlackjackSession
{
    string playerId;
    string stationId;
    int bet;
    int originalBet;
    bool finished;

    ref OECardDeck deck;
    ref array<ref OEPlayingCard> playerCards;
    ref array<ref OEPlayingCard> dealerCards;

    // Split state. Operation Exile Blackjack intentionally supports one split only.
    bool splitActive;
    bool splitAces;
    int activeSplitHand;
    bool splitHand1Finished;
    bool splitHand2Finished;
    ref array<ref OEPlayingCard> splitHand1;
    ref array<ref OEPlayingCard> splitHand2;
    string splitHand1Result;
    string splitHand2Result;
    int splitNetResult;
    int splitHandBet;
    int splitTotalStake;

    void OEBlackjackSession(string uid, string station, int wager)
    {
        playerId = uid;
        stationId = station;
        bet = wager;
        originalBet = wager;
        finished = false;
        deck = new OECardDeck();
        playerCards = new array<ref OEPlayingCard>;
        dealerCards = new array<ref OEPlayingCard>;

        splitActive = false;
        splitAces = false;
        activeSplitHand = 0;
        splitHand1Finished = false;
        splitHand2Finished = false;
        splitHand1 = new array<ref OEPlayingCard>;
        splitHand2 = new array<ref OEPlayingCard>;
        splitHand1Result = "";
        splitHand2Result = "";
        splitNetResult = 0;
        splitHandBet = 0;
        splitTotalStake = 0;
    }
}
