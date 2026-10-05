class OECasinoConfig
{
    static ref OECasinoConfig Instance;

    int configVersion = 26;
    bool enablePlayLogs = true;
    bool hotReload = true;
    int hotReloadSeconds = 5;
    ref map<string, int> currencyValues;
    ref array<ref OECasinoStation> stations;
    ref OERideBusSettings rideBus;
    ref OEBlackjackSettings blackjack;
    ref OEDiceBettingSettings diceBetting;
    ref OESpinTheWinSettings spinTheWin;
    ref OERouletteSettings roulette;
    ref OEHighCardWarSettings highCardWar;
    ref OECrapsSettings craps;
    ref OEDoubleOrNothingSettings doubleOrNothing;
    ref OECasinoHoldemSettings casinoHoldem;
    ref OESlotMachineSettings slotMachine;
    ref OEGoFishSettings goFish;

    void OECasinoConfig()
    {
        currencyValues = new map<string, int>;
        stations = new array<ref OECasinoStation>;
        rideBus = new OERideBusSettings();
        blackjack = new OEBlackjackSettings();
        diceBetting = new OEDiceBettingSettings();
        spinTheWin = new OESpinTheWinSettings();
        roulette = new OERouletteSettings();
        highCardWar = new OEHighCardWarSettings();
        craps = new OECrapsSettings();
        doubleOrNothing = new OEDoubleOrNothingSettings();
        casinoHoldem = new OECasinoHoldemSettings();
        slotMachine = new OESlotMachineSettings();
        goFish = new OEGoFishSettings();
    }

    static OECasinoConfig Get()
    {
        if (!Instance)
        {
            Instance = new OECasinoConfig();
        }
        return Instance;
    }

    static OECasinoConfig LoadServer()
    {
        OECasinoConfig cfg = new OECasinoConfig();

        if (!FileExist(OE_CASINO_PROFILE_DIR))
        {
            MakeDirectory(OE_CASINO_PROFILE_DIR);
        }

        bool createdNewConfig = false;
        bool configChanged = false;

        if (FileExist(OE_CASINO_CONFIG_FILE))
        {
            JsonFileLoader<OECasinoConfig>.JsonLoadFile(OE_CASINO_CONFIG_FILE, cfg);
        }
        else
        {
            createdNewConfig = true;
            configChanged = true;
        }

        if (!cfg.currencyValues)
        {
            cfg.currencyValues = new map<string, int>;
            configChanged = true;
        }

        if (!cfg.stations)
        {
            cfg.stations = new array<ref OECasinoStation>;
            configChanged = true;
        }

        if (!cfg.rideBus)
        {
            cfg.rideBus = new OERideBusSettings();
            configChanged = true;
        }

        if (!cfg.blackjack)
        {
            cfg.blackjack = new OEBlackjackSettings();
            configChanged = true;
        }

        if (!cfg.diceBetting)
        {
            cfg.diceBetting = new OEDiceBettingSettings();
            configChanged = true;
        }

        if (!cfg.spinTheWin)
        {
            cfg.spinTheWin = new OESpinTheWinSettings();
            configChanged = true;
        }

        if (!cfg.roulette)
        {
            cfg.roulette = new OERouletteSettings();
            configChanged = true;
        }

        if (!cfg.highCardWar)
        {
            cfg.highCardWar = new OEHighCardWarSettings();
            configChanged = true;
        }

        if (!cfg.craps)
        {
            cfg.craps = new OECrapsSettings();
            configChanged = true;
        }

        if (!cfg.doubleOrNothing)
        {
            cfg.doubleOrNothing = new OEDoubleOrNothingSettings();
            configChanged = true;
        }

        if (!cfg.casinoHoldem)
        {
            cfg.casinoHoldem = new OECasinoHoldemSettings();
            configChanged = true;
        }

        if (!cfg.slotMachine)
        {
            cfg.slotMachine = new OESlotMachineSettings();
            configChanged = true;
        }

        if (!cfg.goFish)
        {
            cfg.goFish = new OEGoFishSettings();
            configChanged = true;
        }

        if (cfg.configVersion < 19)
        {
            // v0.14.0: Hold'em uses no-limit player raise sizing. Preserve the
            // previous 3x-style value as the dealer's own raise-size ceiling.
            cfg.casinoHoldem.noLimit = true;
            if (cfg.casinoHoldem.maxRaiseMultiplier >= 1 && cfg.casinoHoldem.maxRaiseMultiplier <= 10)
                cfg.casinoHoldem.dealerMaxRaiseMultiplier = cfg.casinoHoldem.maxRaiseMultiplier;
            else
                cfg.casinoHoldem.dealerMaxRaiseMultiplier = 3;
            cfg.configVersion = 19;
            configChanged = true;
        }

        if (cfg.configVersion < 20)
        {
            // v0.15.0: add the progressive three-reel Slot Machine settings.
            cfg.configVersion = 20;
            configChanged = true;
        }

        if (cfg.configVersion < 21)
        {
            // v0.15.2: safer progressive jackpot defaults plus a configurable hard cap.
            // Only replace the exact old 100000 default; preserve any custom value.
            if (cfg.slotMachine.jackpotSeed == 100000)
                cfg.slotMachine.jackpotSeed = 5000;
            if (cfg.slotMachine.jackpotMax < 1)
                cfg.slotMachine.jackpotMax = 50000;
            cfg.configVersion = 21;
            configChanged = true;
        }

        if (cfg.configVersion < 22)
        {
            // v0.15.6: legacy Slot Machine configs could deserialize a missing
            // contribution field as 0, which disabled progressive growth.
            // Repair that legacy value once; admins can deliberately set 0 again
            // after the config has migrated to version 22 if they want no growth.
            if (cfg.slotMachine.jackpotContributionPercent <= 0.0)
                cfg.slotMachine.jackpotContributionPercent = 10.0;
            cfg.configVersion = 22;
            configChanged = true;
        }

        if (cfg.configVersion < 23)
        {
            // v0.15.7: add the temporary one-shot jackpot test switch.
            // Always default it off during migration.
            cfg.slotMachine.debugForceNextJackpot = 0;
            cfg.configVersion = 23;
            configChanged = true;
        }

        if (cfg.configVersion < 24)
        {
            // v0.15.8 public cleanup: retire the temporary jackpot test helper.
            // Keep the legacy field only so configs written by v0.15.7 continue
            // to deserialize safely; public server logic never reads it.
            cfg.slotMachine.debugForceNextJackpot = 0;
            cfg.configVersion = 24;
            configChanged = true;
        }


        if (cfg.configVersion < 25)
        {
            // v0.16.0: add Gambling Go Fish settings.
            if (!cfg.goFish) cfg.goFish = new OEGoFishSettings();
            cfg.configVersion = 25;
            configChanged = true;
        }

        if (cfg.configVersion < 26)
        {
            // v0.16.3: catch up station templates on older installations.
            // Some servers installed Operation Exile: Casino before later games
            // such as Craps, Double or Nothing, Hold'em, Slot Machine or Go Fish
            // existed. Add one disabled template only when that game has no
            // station at all. Existing IDs, positions, enabled states and custom
            // station setups are never overwritten or duplicated.
            if (EnsureAllStationTemplates(cfg))
                configChanged = true;

            cfg.configVersion = 26;
            configChanged = true;
        }

        if (cfg.rideBus.choiceSeconds < 3 || cfg.rideBus.choiceSeconds > 30)
        {
            cfg.rideBus.choiceSeconds = 8;
            configChanged = true;
        }

        if (cfg.rideBus.timeoutBehavior != "lose" && cfg.rideBus.timeoutBehavior != "first_choice")
        {
            cfg.rideBus.timeoutBehavior = "lose";
            configChanged = true;
        }

        if (cfg.blackjack.minBet < 1)
        {
            cfg.blackjack.minBet = 10;
            configChanged = true;
        }

        if (cfg.blackjack.maxBet < cfg.blackjack.minBet)
        {
            cfg.blackjack.maxBet = 5000;
            configChanged = true;
        }

        if (cfg.blackjack.normalWinPayout < 1.0)
        {
            cfg.blackjack.normalWinPayout = 2.0;
            configChanged = true;
        }

        if (cfg.blackjack.blackjackPayout < 1.0)
        {
            cfg.blackjack.blackjackPayout = 2.5;
            configChanged = true;
        }

        if (cfg.diceBetting.minBet < 1)
        {
            cfg.diceBetting.minBet = 10;
            configChanged = true;
        }

        if (cfg.diceBetting.maxBet < cfg.diceBetting.minBet)
        {
            cfg.diceBetting.maxBet = 5000;
            configChanged = true;
        }

        if (cfg.diceBetting.highLowPayout < 1.0) cfg.diceBetting.highLowPayout = 2.0;
        if (cfg.diceBetting.oddEvenPayout < 1.0) cfg.diceBetting.oddEvenPayout = 2.0;
        if (cfg.diceBetting.doublesPayout < 1.0) cfg.diceBetting.doublesPayout = 6.0;
        if (cfg.diceBetting.exact2or12Payout < 1.0) cfg.diceBetting.exact2or12Payout = 31.0;
        if (cfg.diceBetting.exact3or11Payout < 1.0) cfg.diceBetting.exact3or11Payout = 16.0;
        if (cfg.diceBetting.exact4or10Payout < 1.0) cfg.diceBetting.exact4or10Payout = 11.0;
        if (cfg.diceBetting.exact5or9Payout < 1.0) cfg.diceBetting.exact5or9Payout = 8.0;
        if (cfg.diceBetting.exact6or8Payout < 1.0) cfg.diceBetting.exact6or8Payout = 6.0;
        if (cfg.diceBetting.exact7Payout < 1.0) cfg.diceBetting.exact7Payout = 5.0;

        if (cfg.spinTheWin.minBet < 1)
        {
            cfg.spinTheWin.minBet = 100;
            configChanged = true;
        }

        if (cfg.spinTheWin.maxBet < cfg.spinTheWin.minBet)
        {
            cfg.spinTheWin.maxBet = 5000;
            configChanged = true;
        }

        if (cfg.spinTheWin.defaultBet < cfg.spinTheWin.minBet || cfg.spinTheWin.defaultBet > cfg.spinTheWin.maxBet)
        {
            cfg.spinTheWin.defaultBet = cfg.spinTheWin.minBet;
            configChanged = true;
        }

        if (cfg.spinTheWin.jackpotStart < 1)
        {
            cfg.spinTheWin.jackpotStart = 10000;
            configChanged = true;
        }

        if (cfg.spinTheWin.jackpotContributionPercent < 0.0 || cfg.spinTheWin.jackpotContributionPercent > 100.0)
        {
            cfg.spinTheWin.jackpotContributionPercent = 25.0;
            configChanged = true;
        }

        if (cfg.spinTheWin.spinDurationSeconds < 2.0 || cfg.spinTheWin.spinDurationSeconds > 15.0)
        {
            cfg.spinTheWin.spinDurationSeconds = 5.0;
            configChanged = true;
        }

        if (cfg.spinTheWin.minimumRotations < 2 || cfg.spinTheWin.minimumRotations > 10)
        {
            cfg.spinTheWin.minimumRotations = 4;
            configChanged = true;
        }

        if (cfg.roulette.minBet < 1)
        {
            cfg.roulette.minBet = 10;
            configChanged = true;
        }

        if (cfg.roulette.maxBet < cfg.roulette.minBet)
        {
            cfg.roulette.maxBet = 5000;
            configChanged = true;
        }

        if (cfg.roulette.maxTotalBet < cfg.roulette.maxBet)
        {
            cfg.roulette.maxTotalBet = 25000;
            configChanged = true;
        }

        if (cfg.roulette.straightPayout < 1.0) { cfg.roulette.straightPayout = 36.0; configChanged = true; }
        if (cfg.roulette.splitPayout < 1.0) { cfg.roulette.splitPayout = 18.0; configChanged = true; }
        if (cfg.roulette.streetPayout < 1.0) { cfg.roulette.streetPayout = 12.0; configChanged = true; }
        if (cfg.roulette.cornerPayout < 1.0) { cfg.roulette.cornerPayout = 9.0; configChanged = true; }
        if (cfg.roulette.topLinePayout < 1.0) { cfg.roulette.topLinePayout = 7.0; configChanged = true; }
        if (cfg.roulette.sixLinePayout < 1.0) { cfg.roulette.sixLinePayout = 6.0; configChanged = true; }
        if (cfg.roulette.evenMoneyPayout < 1.0) { cfg.roulette.evenMoneyPayout = 2.0; configChanged = true; }
        if (cfg.roulette.dozenColumnPayout < 1.0) { cfg.roulette.dozenColumnPayout = 3.0; configChanged = true; }


        if (cfg.highCardWar.minBet < 1)
        {
            cfg.highCardWar.minBet = 10;
            configChanged = true;
        }
        if (cfg.highCardWar.maxBet < cfg.highCardWar.minBet)
        {
            cfg.highCardWar.maxBet = 5000;
            configChanged = true;
        }
        if (cfg.highCardWar.normalWinPayout < 1.0)
        {
            cfg.highCardWar.normalWinPayout = 2.0;
            configChanged = true;
        }
        if (cfg.highCardWar.warWinReturnMultiplier < 1.0)
        {
            cfg.highCardWar.warWinReturnMultiplier = 3.0;
            configChanged = true;
        }
        if (cfg.highCardWar.surrenderRefundMultiplier < 0.0 || cfg.highCardWar.surrenderRefundMultiplier > 1.0)
        {
            cfg.highCardWar.surrenderRefundMultiplier = 0.5;
            configChanged = true;
        }
        if (cfg.highCardWar.warBurnCards < 0 || cfg.highCardWar.warBurnCards > 10)
        {
            cfg.highCardWar.warBurnCards = 3;
            configChanged = true;
        }
        if (cfg.highCardWar.secondTieWins != 0 && cfg.highCardWar.secondTieWins != 1)
        {
            cfg.highCardWar.secondTieWins = 1;
            configChanged = true;
        }
        if (cfg.highCardWar.deckCount < 1 || cfg.highCardWar.deckCount > 8)
        {
            cfg.highCardWar.deckCount = 6;
            configChanged = true;
        }

        if (cfg.craps.minBet < 1)
        {
            cfg.craps.minBet = 10;
            configChanged = true;
        }
        if (cfg.craps.maxBet < cfg.craps.minBet)
        {
            cfg.craps.maxBet = 5000;
            configChanged = true;
        }
        if (cfg.craps.passLineReturnMultiplier < 1.0)
        {
            cfg.craps.passLineReturnMultiplier = 2.0;
            configChanged = true;
        }
        if (cfg.craps.dontPassReturnMultiplier < 1.0)
        {
            cfg.craps.dontPassReturnMultiplier = 2.0;
            configChanged = true;
        }
        if (cfg.craps.sideMinBet < 1)
        {
            cfg.craps.sideMinBet = 10;
            configChanged = true;
        }
        if (cfg.craps.maxSideBet < cfg.craps.sideMinBet)
        {
            cfg.craps.maxSideBet = 5000;
            configChanged = true;
        }
        if (cfg.craps.maxSideBetTotal < cfg.craps.maxSideBet)
        {
            cfg.craps.maxSideBetTotal = 25000;
            configChanged = true;
        }
        if (cfg.craps.maxOddsMultiplier < 1.0)
        {
            cfg.craps.maxOddsMultiplier = 5.0;
            configChanged = true;
        }

        if (cfg.doubleOrNothing.minBet < 1)
        {
            cfg.doubleOrNothing.minBet = 10;
            configChanged = true;
        }
        if (cfg.doubleOrNothing.maxBet < cfg.doubleOrNothing.minBet)
        {
            cfg.doubleOrNothing.maxBet = 5000;
            configChanged = true;
        }
        // v0.12.1: Red / Black card mode is a true 26-vs-26 colour game.
        // Keep the field in JSON for backwards compatibility, but normalize it
        // to 50 so older configs cannot disagree with the card rules/UI.
        if (cfg.doubleOrNothing.winChancePercent < 49.999 || cfg.doubleOrNothing.winChancePercent > 50.001)
        {
            cfg.doubleOrNothing.winChancePercent = 50.0;
            configChanged = true;
        }
        if (cfg.doubleOrNothing.maxDoubles < 1 || cfg.doubleOrNothing.maxDoubles > 12)
        {
            cfg.doubleOrNothing.maxDoubles = 6;
            configChanged = true;
        }
        if (cfg.doubleOrNothing.maxPayout < (cfg.doubleOrNothing.maxBet * 2))
        {
            cfg.doubleOrNothing.maxPayout = 500000;
            configChanged = true;
        }

        // v0.13.1: Heads-up Texas Hold'em against dealer AI. The old
        // Casino Hold'em paytable fields remain in the settings class only so
        // v0.13.0 JSON files can load; they are no longer used for gameplay.
        if (cfg.casinoHoldem.minBet < 1)
        {
            cfg.casinoHoldem.minBet = 10;
            configChanged = true;
        }
        if (cfg.casinoHoldem.maxBet < cfg.casinoHoldem.minBet)
        {
            cfg.casinoHoldem.maxBet = 5000;
            configChanged = true;
        }
        if (cfg.casinoHoldem.dealerBluffPercent < 0 || cfg.casinoHoldem.dealerBluffPercent > 50)
        {
            cfg.casinoHoldem.dealerBluffPercent = 12;
            configChanged = true;
        }
        if (cfg.casinoHoldem.dealerAggressionPercent < 0 || cfg.casinoHoldem.dealerAggressionPercent > 100)
        {
            cfg.casinoHoldem.dealerAggressionPercent = 55;
            configChanged = true;
        }
        if (cfg.casinoHoldem.maxRaisesPerStreet < 1 || cfg.casinoHoldem.maxRaisesPerStreet > 5)
        {
            cfg.casinoHoldem.maxRaisesPerStreet = 2;
            configChanged = true;
        }
        if (cfg.casinoHoldem.dealerMaxRaiseMultiplier < 1 || cfg.casinoHoldem.dealerMaxRaiseMultiplier > 10)
        {
            cfg.casinoHoldem.dealerMaxRaiseMultiplier = 3;
            configChanged = true;
        }
        if (cfg.casinoHoldem.maxRaiseMultiplier < 1 || cfg.casinoHoldem.maxRaiseMultiplier > 10)
        {
            cfg.casinoHoldem.maxRaiseMultiplier = 3;
            configChanged = true;
        }

        // v0.15.0: progressive Slot Machine defaults and safety checks.
        if (cfg.slotMachine.minBet < 1) { cfg.slotMachine.minBet = 10; configChanged = true; }
        if (cfg.slotMachine.maxBet < cfg.slotMachine.minBet) { cfg.slotMachine.maxBet = 1000; configChanged = true; }
        if (cfg.slotMachine.defaultBet < cfg.slotMachine.minBet || cfg.slotMachine.defaultBet > cfg.slotMachine.maxBet) { cfg.slotMachine.defaultBet = 100; configChanged = true; }
        if (cfg.slotMachine.betStep < 1) { cfg.slotMachine.betStep = 10; configChanged = true; }
        if (cfg.slotMachine.jackpotSeed < 1) { cfg.slotMachine.jackpotSeed = 5000; configChanged = true; }
        if (cfg.slotMachine.jackpotMax < cfg.slotMachine.jackpotSeed) { cfg.slotMachine.jackpotMax = 50000; if (cfg.slotMachine.jackpotMax < cfg.slotMachine.jackpotSeed) cfg.slotMachine.jackpotMax = cfg.slotMachine.jackpotSeed; configChanged = true; }
        if (cfg.slotMachine.jackpotContributionPercent < 0.0 || cfg.slotMachine.jackpotContributionPercent > 100.0) { cfg.slotMachine.jackpotContributionPercent = 10.0; configChanged = true; }
        if (cfg.slotMachine.jackpotRequiresMaxBet != 0 && cfg.slotMachine.jackpotRequiresMaxBet != 1) { cfg.slotMachine.jackpotRequiresMaxBet = 1; configChanged = true; }
        if (cfg.slotMachine.debugForceNextJackpot != 0) { cfg.slotMachine.debugForceNextJackpot = 0; configChanged = true; }
        if (cfg.slotMachine.pairReturnMultiplier < 1) { cfg.slotMachine.pairReturnMultiplier = 1; configChanged = true; }
        if (cfg.slotMachine.lemonTripleMultiplier < 1) { cfg.slotMachine.lemonTripleMultiplier = 5; configChanged = true; }
        if (cfg.slotMachine.cherryTripleMultiplier < 1) { cfg.slotMachine.cherryTripleMultiplier = 8; configChanged = true; }
        if (cfg.slotMachine.grapeTripleMultiplier < 1) { cfg.slotMachine.grapeTripleMultiplier = 12; configChanged = true; }
        if (cfg.slotMachine.bellTripleMultiplier < 1) { cfg.slotMachine.bellTripleMultiplier = 18; configChanged = true; }
        if (cfg.slotMachine.barTripleMultiplier < 1) { cfg.slotMachine.barTripleMultiplier = 30; configChanged = true; }
        if (cfg.slotMachine.sevenTripleMultiplier < 1) { cfg.slotMachine.sevenTripleMultiplier = 75; configChanged = true; }


        // v0.16.0: Gambling Go Fish defaults and safety checks.
        if (cfg.goFish.minBet < 1) { cfg.goFish.minBet = 10; configChanged = true; }
        if (cfg.goFish.maxBet < cfg.goFish.minBet) { cfg.goFish.maxBet = 1000; configChanged = true; }
        if (cfg.goFish.defaultBet < cfg.goFish.minBet || cfg.goFish.defaultBet > cfg.goFish.maxBet) { cfg.goFish.defaultBet = 100; configChanged = true; }
        if (cfg.goFish.betStep < 1) { cfg.goFish.betStep = 10; configChanged = true; }
        if (cfg.goFish.winPayout < 1.0 || cfg.goFish.winPayout > 10.0) { cfg.goFish.winPayout = 1.9; configChanged = true; }
        if (cfg.goFish.startingCards < 1 || cfg.goFish.startingCards > 10) { cfg.goFish.startingCards = 7; configChanged = true; }
        if (cfg.goFish.refillCards < 1 || cfg.goFish.refillCards > 10) { cfg.goFish.refillCards = 5; configChanged = true; }

        // v0.3.0: Operation Exile now ships its own casino-chip classes.
        // Migrate the exact legacy DayZCasinoV2 development classnames automatically
        // so an existing CasinoConfig.json no longer keeps that external dependency.
        int legacyValue = 0;
        bool hadLegacyCurrency = false;
        if (cfg.currencyValues.Find("CasinoChip", legacyValue)) hadLegacyCurrency = true;
        if (cfg.currencyValues.Find("CasinoChip_Red", legacyValue)) hadLegacyCurrency = true;
        if (cfg.currencyValues.Find("CasinoChip_Blue", legacyValue)) hadLegacyCurrency = true;
        if (cfg.currencyValues.Find("CasinoChip_Green", legacyValue)) hadLegacyCurrency = true;
        if (cfg.currencyValues.Find("CasinoChip_Yellow", legacyValue)) hadLegacyCurrency = true;

        if (hadLegacyCurrency)
        {
            cfg.currencyValues.Remove("CasinoChip");
            cfg.currencyValues.Remove("CasinoChip_Red");
            cfg.currencyValues.Remove("CasinoChip_Blue");
            cfg.currencyValues.Remove("CasinoChip_Green");
            cfg.currencyValues.Remove("CasinoChip_Yellow");
        }

        if (cfg.currencyValues.Count() == 0 || hadLegacyCurrency)
        {
            // Remove first so migration is deterministic even if a partially edited
            // development config already contains one or more OE entries.
            cfg.currencyValues.Remove("OE_CasinoChip_1");
            cfg.currencyValues.Remove("OE_CasinoChip_5");
            cfg.currencyValues.Remove("OE_CasinoChip_10");
            cfg.currencyValues.Remove("OE_CasinoChip_25");
            cfg.currencyValues.Remove("OE_CasinoChip_100");
            cfg.currencyValues.Remove("OE_CasinoChip_500");
            cfg.currencyValues.Remove("OE_CasinoChip_1000");

            cfg.currencyValues.Insert("OE_CasinoChip_1", 1);
            cfg.currencyValues.Insert("OE_CasinoChip_5", 5);
            cfg.currencyValues.Insert("OE_CasinoChip_10", 10);
            cfg.currencyValues.Insert("OE_CasinoChip_25", 25);
            cfg.currencyValues.Insert("OE_CasinoChip_100", 100);
            cfg.currencyValues.Insert("OE_CasinoChip_500", 500);
            cfg.currencyValues.Insert("OE_CasinoChip_1000", 1000);
            configChanged = true;
        }

        // Fresh installs also use the same catalogue helper. Normally the
        // v26 migration above has already populated these, but keeping this
        // first-run guard makes the intent explicit and future-safe.
        if (createdNewConfig && cfg.stations.Count() == 0)
        {
            if (EnsureAllStationTemplates(cfg))
                configChanged = true;
        }

        if (cfg.hotReloadSeconds < 2)
        {
            cfg.hotReloadSeconds = 5;
            configChanged = true;
        }

        // Only write when creating/repairing the config. Hot reload must not rewrite
        // a valid JSON file every few seconds.
        if (configChanged)
        {
            JsonFileLoader<OECasinoConfig>.JsonSaveFile(OE_CASINO_CONFIG_FILE, cfg);
        }

        Instance = cfg;
        return cfg;
    }

    protected static bool HasStationForGame(OECasinoConfig cfg, string gameId)
    {
        if (!cfg || !cfg.stations) return false;

        foreach (OECasinoStation station : cfg.stations)
        {
            if (station && station.game == gameId)
                return true;
        }

        return false;
    }

    protected static void AddStationTemplate(OECasinoConfig cfg, string stationId, string gameId)
    {
        if (!cfg) return;
        if (!cfg.stations) cfg.stations = new array<ref OECasinoStation>;

        OECasinoStation station = new OECasinoStation();
        station.id = stationId;
        station.game = gameId;
        station.position = vector.Zero;
        station.objectType = "";
        station.targetTolerance = 1.25;
        station.playDistance = 4.0;
        station.enabled = false;
        cfg.stations.Insert(station);
    }

    protected static bool EnsureStationTemplate(OECasinoConfig cfg, string stationId, string gameId)
    {
        if (HasStationForGame(cfg, gameId))
            return false;

        AddStationTemplate(cfg, stationId, gameId);
        return true;
    }

    protected static bool EnsureAllStationTemplates(OECasinoConfig cfg)
    {
        bool changed = false;

        if (EnsureStationTemplate(cfg, "ridebus_test", OE_CASINO_GAME_RIDE_BUS)) changed = true;
        if (EnsureStationTemplate(cfg, "blackjack_test", OE_CASINO_GAME_BLACKJACK)) changed = true;
        if (EnsureStationTemplate(cfg, "dicebetting_test", OE_CASINO_GAME_DICE_BETTING)) changed = true;
        if (EnsureStationTemplate(cfg, "spinthewin_test", OE_CASINO_GAME_SPIN_THE_WIN)) changed = true;
        if (EnsureStationTemplate(cfg, "roulette_test", OE_CASINO_GAME_ROULETTE)) changed = true;
        if (EnsureStationTemplate(cfg, "highcardwar_test", OE_CASINO_GAME_HIGH_CARD_WAR)) changed = true;
        if (EnsureStationTemplate(cfg, "craps_test", OE_CASINO_GAME_CRAPS)) changed = true;
        if (EnsureStationTemplate(cfg, "doubleornothing_test", OE_CASINO_GAME_DOUBLE_OR_NOTHING)) changed = true;
        if (EnsureStationTemplate(cfg, "holdem_test", OE_CASINO_GAME_CASINO_HOLDEM)) changed = true;
        if (EnsureStationTemplate(cfg, "slotmachine_test", OE_CASINO_GAME_SLOT_MACHINE)) changed = true;
        if (EnsureStationTemplate(cfg, "gofish_test", OE_CASINO_GAME_GO_FISH)) changed = true;

        return changed;
    }

    OECasinoStation GetStationById(string stationId)
    {
        if (!stations) return null;
        foreach (OECasinoStation station : stations)
        {
            if (station && station.enabled && station.id == stationId)
                return station;
        }
        return null;
    }

    OECasinoStation GetStationNearPosition(vector position)
    {
        if (!stations) return null;

        OECasinoStation nearest = null;
        float nearestDistance = 999999.0;

        foreach (OECasinoStation station : stations)
        {
            if (!station || !station.enabled) continue;

            float distance = vector.Distance(position, station.position);
            if (distance <= station.playDistance && distance < nearestDistance)
            {
                nearest = station;
                nearestDistance = distance;
            }
        }

        return nearest;
    }

    OECasinoStation GetStationForObject(Object obj)
    {
        if (!obj || !stations) return null;

        vector objectPos = obj.GetPosition();
        string typeName = obj.GetType();

        foreach (OECasinoStation station : stations)
        {
            if (!station || !station.enabled) continue;
            if (station.objectType != "" && station.objectType != typeName) continue;

            if (vector.Distance(objectPos, station.position) <= station.targetTolerance)
                return station;
        }
        return null;
    }
}
