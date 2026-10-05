class OECasinoHoldemSession
{
    string playerId;
    string stationId;

    int startingBet;
    int playerContribution;
    int dealerContribution;
    int playerStreetBet;
    int dealerStreetBet;
    int pot;
    int street;
    int raisesThisStreet;

    bool finished;
    bool waitingForPlayer;

    ref OECardDeck deck;
    ref array<ref OEPlayingCard> playerCards;
    ref array<ref OEPlayingCard> dealerCards;
    ref array<ref OEPlayingCard> communityCards;

    int payout;
    int netResult;
    string playerHandName;
    string dealerHandName;
    string dealerLastAction;

    void OECasinoHoldemSession(string uid, string station, int openingBet)
    {
        playerId = uid;
        stationId = station;
        startingBet = openingBet;

        // The player posts the chosen opening bet and the house/dealer matches it.
        playerContribution = openingBet;
        dealerContribution = openingBet;
        playerStreetBet = 0;
        dealerStreetBet = 0;
        pot = openingBet * 2;
        street = OE_HOLDEM_STREET_PREFLOP;
        raisesThisStreet = 0;

        finished = false;
        waitingForPlayer = true;

        deck = new OECardDeck();
        playerCards = new array<ref OEPlayingCard>;
        dealerCards = new array<ref OEPlayingCard>;
        communityCards = new array<ref OEPlayingCard>;

        payout = 0;
        netResult = 0 - openingBet;
        playerHandName = "";
        dealerHandName = "";
        dealerLastAction = "DEALER MATCHED THE OPENING BET";
    }

    int AmountToCall()
    {
        int amount = dealerStreetBet - playerStreetBet;
        if (amount < 0) return 0;
        return amount;
    }

    void RefreshPot()
    {
        pot = playerContribution + dealerContribution;
    }
}
