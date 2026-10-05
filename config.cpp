class CfgPatches
{
    class OperationExileCasino
    {
        units[] =
        {
            "OE_CasinoChip_1",
            "OE_CasinoChip_5",
            "OE_CasinoChip_10",
            "OE_CasinoChip_25",
            "OE_CasinoChip_100",
            "OE_CasinoChip_500",
            "OE_CasinoChip_1000"
        };
        weapons[] = {};
        requiredVersion = 1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class OperationExileCasino
    {
        dir = "OperationExileCasino";
        name = "Operation Exile Casino";
        author = "CtrlQuest / Operation Exile";
        version = "0.16.3-station-template-catch-up-migration";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"OperationExileCasino/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"OperationExileCasino/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"OperationExileCasino/scripts/5_Mission"};
            };
        };
    };
};


class CfgSoundShaders
{
    class OECasino_CardFlip_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\card-flip",1}};
        volume = 0.85;
        range = 5;
        limitation = 0;
    };
    class OECasino_DiceRoll_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\rollingdice",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SpinTheWin_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\spin_the_win",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_RouletteWheel_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\roulette_wheel",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotSpin_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_spin",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotReelStop_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_reel_stop",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotWin_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_win",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotLose_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_lose",1}};
        volume = 0.90;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotJackpot_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_jackpot",1}};
        volume = 0.95;
        range = 5;
        limitation = 0;
    };
    class OECasino_SlotPayout_SoundShader
    {
        samples[] = {{"OperationExileCasino\sounds\slot_payout",1}};
        volume = 0.80;
        range = 5;
        limitation = 0;
    };
};

class CfgSoundSets
{
    class OECasino_CardFlip_SoundSet
    {
        soundShaders[] = {"OECasino_CardFlip_SoundShader"};
        volumeFactor = 1.0;
        frequencyFactor = 1.0;
        spatial = 0;
        loop = 0;
    };
    class OECasino_DiceRoll_SoundSet
    {
        soundShaders[] = {"OECasino_DiceRoll_SoundShader"};
        volumeFactor = 1.0;
        frequencyFactor = 1.0;
        spatial = 0;
        loop = 0;
    };
    class OECasino_SpinTheWin_SoundSet
    {
        soundShaders[] = {"OECasino_SpinTheWin_SoundShader"};
        volumeFactor = 1.0;
        frequencyFactor = 1.0;
        spatial = 0;
        loop = 0;
    };
    class OECasino_RouletteWheel_SoundSet
    {
        soundShaders[] = {"OECasino_RouletteWheel_SoundShader"};
        volumeFactor = 1.0;
        frequencyFactor = 1.0;
        spatial = 0;
        loop = 0;
    };
    class OECasino_SlotSpin_SoundSet
    {
        soundShaders[] = {"OECasino_SlotSpin_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
    class OECasino_SlotReelStop_SoundSet
    {
        soundShaders[] = {"OECasino_SlotReelStop_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
    class OECasino_SlotWin_SoundSet
    {
        soundShaders[] = {"OECasino_SlotWin_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
    class OECasino_SlotLose_SoundSet
    {
        soundShaders[] = {"OECasino_SlotLose_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
    class OECasino_SlotJackpot_SoundSet
    {
        soundShaders[] = {"OECasino_SlotJackpot_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
    class OECasino_SlotPayout_SoundSet
    {
        soundShaders[] = {"OECasino_SlotPayout_SoundShader"};
        volumeFactor = 1.0; frequencyFactor = 1.0; spatial = 0; loop = 0;
    };
};

class CfgVehicles
{
    class Inventory_Base;

    class OE_CasinoChip_Base: Inventory_Base
    {
        scope = 0;
        displayName = "Casino Chip";
        descriptionShort = "A casino chip used at Operation Exile Casino games.";
        model = "OperationExileCasino\Models\OE_CasinoChip_100.p3d";
        animClass = "NoFireClass";
        rotationFlags = 17;
        weight = 0.1;
        itemSize[] = {1,1};
        absorbency = 0;
        physLayer = "item_small";
        soundImpactType = "plastic";

        // Keep the stack behaviour used by the casino currency system, but spawn
        // a single chip by default. Scripts can combine these into stacks up to 10,000.
        canBeSplit = 1;
        varQuantityInit = 1;
        varQuantityMin = 0;
        varQuantityMax = 10000;
        varQuantityDestroyOnMin = 1;
        varStackMax = 10000;

        hiddenSelections[] = {"face", "side"};
        class DamageSystem
        {
            class GlobalHealth
            {
                class Health
                {
                    hitpoints = 100;
                    healthLevels[] =
                    {
                        {1.0, {}},
                        {0.7, {}},
                        {0.5, {}},
                        {0.3, {}},
                        {0.0, {}}
                    };
                };
            };
        };
    };

    class OE_CasinoChip_1: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $1";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_1_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_1_Side.paa"
        };
    };

    class OE_CasinoChip_5: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $5";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_5_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_5_Side.paa"
        };
    };

    class OE_CasinoChip_10: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $10";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_10_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_10_Side.paa"
        };
    };

    class OE_CasinoChip_25: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $25";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_25_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_25_Side.paa"
        };
    };

    class OE_CasinoChip_100: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $100";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_100_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_100_Side.paa"
        };
    };

    class OE_CasinoChip_500: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $500";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_500_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_500_Side.paa"
        };
    };

    class OE_CasinoChip_1000: OE_CasinoChip_Base
    {
        scope = 2;
        displayName = "Casino Chip $1,000";
        hiddenSelectionsTextures[] =
        {
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_1000_BaseColor.paa",
            "OperationExileCasino\Models\Data\CasinoChips\OE_Chip_1000_Side.paa"
        };
    };
};
