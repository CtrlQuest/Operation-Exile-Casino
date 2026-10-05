class OECasinoStation
{
    string id = "";
    string game = "";
    vector position = "0 0 0";
    string objectType = ""; // Blank = any object at the configured position.
    float targetTolerance = 1.25;
    float playDistance = 4.0;
    bool enabled = true;

    void OECasinoStation(string stationId = "", string gameId = "", vector pos = "0 0 0")
    {
        id = stationId;
        game = gameId;
        position = pos;
    }
}
