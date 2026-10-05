class OEGoFishSession
{
    string playerId;
    string stationId;
    int bet;
    bool finished;

    ref OECardDeck deck;
    ref array<ref OEPlayingCard> playerHand;
    ref array<ref OEPlayingCard> dealerHand;
    ref array<int> playerBookRanks;
    ref array<int> dealerBookRanks;
    ref array<int> dealerMemoryRanks;

    void OEGoFishSession(string uid, string station, int wager)
    {
        playerId = uid;
        stationId = station;
        bet = wager;
        finished = false;
        deck = new OECardDeck();
        playerHand = new array<ref OEPlayingCard>;
        dealerHand = new array<ref OEPlayingCard>;
        playerBookRanks = new array<int>;
        dealerBookRanks = new array<int>;
        dealerMemoryRanks = new array<int>;
    }
}
