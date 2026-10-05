class OERouletteSpinRequest
{
    string stationId;
    ref array<int> betTypes;
    ref array<int> amounts;
    ref array<int> a;
    ref array<int> b;
    ref array<int> c;
    ref array<int> d;
    ref array<int> e;
    ref array<int> f;

    void OERouletteSpinRequest()
    {
        stationId = "";
        betTypes = new array<int>;
        amounts = new array<int>;
        a = new array<int>;
        b = new array<int>;
        c = new array<int>;
        d = new array<int>;
        e = new array<int>;
        f = new array<int>;
    }
}
