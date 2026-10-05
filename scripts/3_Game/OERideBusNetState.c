class OERideBusNetState
{
    int status;
    int round;
    int bet;
    int cashoutValue;
    int balance;
    int decisionSeconds;
    string stationId;

    string card1;
    string card2;
    string card3;
    string card4;

    int card1Rank;
    int card1Suit;
    int card2Rank;
    int card2Suit;
    int card3Rank;
    int card3Suit;
    int card4Rank;
    int card4Suit;

    string message;

    void OERideBusNetState()
    {
        status = OE_RTB_STATUS_IDLE;
        round = OE_RTB_ROUND_COLOUR;
        bet = 0;
        cashoutValue = 0;
        balance = 0;
        decisionSeconds = 0;
        stationId = "";

        card1 = "";
        card2 = "";
        card3 = "";
        card4 = "";

        card1Rank = 0;
        card1Suit = -1;
        card2Rank = 0;
        card2Suit = -1;
        card3Rank = 0;
        card3Suit = -1;
        card4Rank = 0;
        card4Suit = -1;

        message = "";
    }
}
