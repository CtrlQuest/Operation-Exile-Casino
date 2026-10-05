# Operation Exile: Casino

**Operation Exile: Casino** is a configurable casino and gambling framework for **DayZ**, created by **CtrlQuest / Operation Exile**.

It originally started as a private casino system for the Operation Exile server. As the project grew into a much larger collection of games, UI systems, animations, sounds, casino chips, configurable stations and server-side game logic, we decided to release it for other DayZ communities to use as well.

> **Licence:** Source Available — see [`LICENSE`](LICENSE).  
> For normal server use, please use the **official Steam Workshop release**.

## Links

- **Official Steam Workshop:** https://steamcommunity.com/sharedfiles/filedetails/?id=3803931966
- **Source Repository:** https://github.com/CtrlQuest/Operation-Exile-Casino

The GitHub repository may occasionally be slightly behind the Steam Workshop immediately after an update. I will do my best to keep both releases aligned.

---

## Current Games

Operation Exile: Casino currently includes:

- 🚌 **Ride the Bus**
- 🃏 **Blackjack**
- 🎲 **Dice Betting**
- 🎡 **Spin the Win**
- 🔴 **Roulette**
- 🃏 **High Card / War**
- 🎲 **Craps**
- 💀 **Double or Nothing**
- ♠️ **Texas Hold'em** — Heads-Up vs Dealer AI
- 🎰 **Slot Machine** — Progressive 3-Reel Fruit Machine
- 🐟 **Go Fish** — Player vs Dealer

More games and systems may be added over time.

---

## Main Features

- Custom casino-chip currency system
- Server-authoritative game results and payouts
- JSON-based server configuration
- Configurable minimum and maximum bets
- Configurable wager steps and payouts
- Individual casino stations placed around the map
- Game-specific `F Play...` interaction prompts
- Card, reel, wheel and dice animations
- Game-specific sound effects
- Persistent Slot Machine progressive jackpot
- Configurable jackpot seed, cap and losing-wager contribution
- Configurable Slot Machine maximum bet and jackpot eligibility
- Session recovery for supported games
- Play logging
- Hot-reloadable configuration
- Automatic station-template catch-up migration for older server configs

The mod is still primarily developed around the needs of **Operation Exile**, but the public version is designed so other server owners can balance and configure it around their own economy.

---

## Source Files & Customisation

This repository is provided so server owners and modders can inspect and customise the project.

You can use the source to:

- Learn how the casino systems work
- Modify the games for your own server
- Retexture the cards and card backs
- Replace wheel / Spin the Win artwork
- Change Slot Machine artwork
- Modify UI layouts
- Replace sounds where licensing permits
- Change game balancing
- Add your own features
- Build private custom versions for your own server

### Steam Workshop derivatives

This repository is **not** permission to simply rebuild the PBO and upload an unchanged copy of Operation Exile: Casino to the Steam Workshop.

A separate Workshop derivative must include **genuine, meaningful retexture/custom visual work**, must clearly state that it is based on Operation Exile: Casino, must credit **CtrlQuest / Operation Exile**, and must link back to the official Workshop release.

Changing only the mod name, metadata, config values, file paths, branding, or one token texture is not enough.

Please read the full [`LICENSE`](LICENSE) before redistributing a derivative build.

---

## Server Installation

For ordinary server use:

1. Subscribe to the **official Operation Exile: Casino** Workshop item.
2. Add the mod to the server's normal DayZ mod list.
3. Ensure connecting clients load the same Workshop mod.
4. Copy the supplied `.bikey` from the mod's `Keys` folder into the server's `keys` folder if signature verification is enabled.
5. Start the server once.
6. The mod creates its configuration under the server profile directory.

Main config:

```text
$profile:OperationExileCasino\CasinoConfig.json
```

Slot Machine jackpot state:

```text
$profile:OperationExileCasino\SlotMachineJackpot.json
```

When play logging is enabled:

```text
$profile:OperationExileCasino\plays.log
```

---

## Casino Chips

Default casino chip classes:

```text
OE_CasinoChip_1
OE_CasinoChip_5
OE_CasinoChip_10
OE_CasinoChip_25
OE_CasinoChip_100
OE_CasinoChip_500
OE_CasinoChip_1000
```

Default values:

```text
$1
$5
$10
$25
$100
$500
$1,000
```

The casino framework calculates a player's available chip balance and creates payouts from these values.

---

## Casino Stations

Playable casino locations are stored in the `stations` array inside `CasinoConfig.json`.

Example:

```json
{
  "id": "craps_main",
  "game": "craps",
  "position": [1234.5, 12.3, 5678.9],
  "objectType": "",
  "targetTolerance": 1.25,
  "playDistance": 4.0,
  "enabled": 1
}
```

### Current game IDs

```text
ride_bus
blackjack
dice_betting
spin_the_win
roulette
high_card_war
craps
double_or_nothing
casino_holdem
slot_machine
go_fish
```

You can create multiple stations for the same game as long as every station has its own unique `id` and position.

---

## First Run & Station Migration

A fresh configuration receives a disabled template station for every currently supported casino game.

Templates are created at:

```text
[0.0, 0.0, 0.0]
```

and are disabled by default so a fresh server does not create overlapping interactions at map origin.

From **v0.16.3 onward**, older configurations also use a station-template catch-up migration.

When an older config upgrades, the mod checks each supported game by its **game type**:

- If at least one station for that game already exists, it is left completely untouched.
- If no station exists for that game, a disabled template is appended.
- Existing station IDs, positions, object types, distances and enabled states are never replaced by this migration.
- Custom station names are recognised correctly; a server does not need to use the default `*_test` ID.
- The catch-up runs as a version migration rather than continually recreating deliberately deleted templates every restart.

This means servers that installed Operation Exile: Casino before newer games such as Craps, Hold'em, Slot Machine or Go Fish can safely receive the missing station templates without losing their existing setup.

---

## Example Station Template

```json
{
  "id": "gofish_test",
  "game": "go_fish",
  "position": [0.0, 0.0, 0.0],
  "objectType": "",
  "targetTolerance": 1.25,
  "playDistance": 4.0,
  "enabled": 0
}
```

Set the real world position and change `enabled` to `1` when you are ready to use the station.

---

## Slot Machine Progressive Jackpot

The Slot Machine includes a persistent shared progressive jackpot.

Server owners can configure settings including:

- Minimum bet
- Maximum bet
- Default bet
- Bet step
- Starting jackpot
- Maximum jackpot cap
- Losing-wager contribution percentage
- Whether MAX BET is required for jackpot eligibility
- Pair return
- Individual triple-symbol payouts

Example:

```json
"slotMachine": {
  "enabled": 1,
  "minBet": 10,
  "maxBet": 1000,
  "defaultBet": 100,
  "betStep": 10,
  "jackpotSeed": 5000,
  "jackpotMax": 50000,
  "jackpotContributionPercent": 10.0,
  "jackpotRequiresMaxBet": 1,
  "pairReturnMultiplier": 1,
  "lemonTripleMultiplier": 5,
  "cherryTripleMultiplier": 8,
  "grapeTripleMultiplier": 12,
  "bellTripleMultiplier": 18,
  "barTripleMultiplier": 30,
  "sevenTripleMultiplier": 75
}
```

The jackpot survives server restarts and is shared by Slot Machine stations using the same server jackpot state.

---

## Go Fish

Go Fish is a Player vs Dealer gambling game using the casino's standard playing-card system.

Features include:

- 7-card opening hands
- Four-of-a-kind books
- Rank-based asking
- Go Fish pond draws
- Dealer AI
- Configurable wager limits
- Configurable win payout
- Tie wager returns

---

## Development Priorities

Operation Exile: Casino is primarily developed for our own Operation Exile server.

Development will generally prioritise:

1. Features we want for Operation Exile
2. Bug fixes
3. Improvements to existing games
4. Features that also make sense for the public release

Suggestions and bug reports are welcome, but custom per-server versions are not guaranteed.

---

## Future Ideas

Ideas being considered include:

- Multiplayer/shared casino tables
- Player vs Player Poker
- Multiplayer wagering
- Casino tournaments
- Russian Roulette
- Casino/server event games
- Additional casino and card games

Roadmap ideas are not guaranteed until they are released.

---

## Bug Reports

If reporting a problem, please include as much as possible:

- Game affected
- What you expected to happen
- What actually happened
- Relevant client/server RPT errors
- Screenshots or video where useful
- Relevant `CasinoConfig.json` settings
- Mod version

Good reproduction information makes bugs much easier to find and fix.

---

## Third-Party Material

Some audio and other assets may come from third-party sources and are **not relicensed** under the Operation Exile: Casino source licence.

See [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) before redistributing a derivative build.

---

## Licence

Operation Exile: Casino is released under the custom **Operation Exile: Casino Source Available License v1.0**.

In short:

- ✅ You may inspect, learn from and modify the source.
- ✅ You may make private/custom builds for servers you operate.
- ✅ GitHub forks for development are allowed with the licence and attribution retained.
- ✅ A genuinely retextured derivative Workshop release can be permitted under the licence conditions.
- ❌ You may not simply repack/reupload the unchanged mod to Steam Workshop.
- ❌ Renaming or making trivial changes does not make an independent Workshop reupload acceptable.
- 🔗 Permitted Workshop derivatives must credit the original project and link to the official Workshop release.

Read [`LICENSE`](LICENSE) for the complete terms.

---

## Credits

**Operation Exile: Casino**  
Created by **CtrlQuest / Operation Exile**

Official Workshop:  
https://steamcommunity.com/sharedfiles/filedetails/?id=3803931966

Source:  
https://github.com/CtrlQuest/Operation-Exile-Casino

© 2026 CtrlQuest / Operation Exile. All rights reserved except as expressly granted by the project licence.
