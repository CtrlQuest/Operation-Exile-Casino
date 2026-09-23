static const string OE_CASINO_PROFILE_DIR = "$profile:OperationExileCasino\\";
static const string OE_CASINO_CONFIG_FILE = "$profile:OperationExileCasino\\CasinoConfig.json";
static const string OE_CASINO_SPIN_JACKPOT_FILE = "$profile:OperationExileCasino\\SpinTheWinJackpot.json";

static const string OE_CASINO_GAME_RIDE_BUS = "ride_bus";
static const string OE_CASINO_GAME_BLACKJACK = "blackjack";
static const string OE_CASINO_GAME_DICE_BETTING = "dice_betting";

// Kept in one range so adding games does not scatter magic numbers through the project.
static const int OE_CASINO_RPC_CONFIG_REQUEST      = 1765101;
static const int OE_CASINO_RPC_CONFIG_RESPONSE     = 1765102;
static const int OE_CASINO_RPC_RTB_START           = 1765201;
static const int OE_CASINO_RPC_RTB_CHOICE          = 1765202;
static const int OE_CASINO_RPC_RTB_CASHOUT         = 1765203;
static const int OE_CASINO_RPC_RTB_STATE           = 1765204;
static const int OE_CASINO_RPC_RTB_SYNC            = 1765205;

static const int OE_RTB_STATUS_IDLE     = 0;
static const int OE_RTB_STATUS_PLAYING  = 1;
static const int OE_RTB_STATUS_WON      = 2;
static const int OE_RTB_STATUS_LOST     = 3;
static const int OE_RTB_STATUS_CASHED   = 4;
static const int OE_RTB_STATUS_ERROR    = 5;

static const int OE_RTB_ROUND_COLOUR    = 1;
static const int OE_RTB_ROUND_HIGH_LOW  = 2;
static const int OE_RTB_ROUND_IN_OUT    = 3;
static const int OE_RTB_ROUND_SUIT      = 4;

static const int OE_RTB_CHOICE_RED      = 10;
static const int OE_RTB_CHOICE_BLACK    = 11;
static const int OE_RTB_CHOICE_HIGHER   = 20;
static const int OE_RTB_CHOICE_LOWER    = 21;
static const int OE_RTB_CHOICE_INSIDE   = 30;
static const int OE_RTB_CHOICE_OUTSIDE  = 31;
static const int OE_RTB_CHOICE_HEARTS   = 40;
static const int OE_RTB_CHOICE_DIAMONDS = 41;
static const int OE_RTB_CHOICE_CLUBS    = 42;
static const int OE_RTB_CHOICE_SPADES   = 43;

static const int OE_CARD_SUIT_HEARTS   = 0;
static const int OE_CARD_SUIT_DIAMONDS = 1;
static const int OE_CARD_SUIT_CLUBS    = 2;
static const int OE_CARD_SUIT_SPADES   = 3;

// Blackjack RPCs and state/actions.
static const int OE_CASINO_RPC_BJ_SYNC             = 1765301;
static const int OE_CASINO_RPC_BJ_START            = 1765302;
static const int OE_CASINO_RPC_BJ_ACTION           = 1765303;
static const int OE_CASINO_RPC_BJ_STATE            = 1765304;

static const int OE_BJ_STATUS_IDLE       = 0;
static const int OE_BJ_STATUS_PLAYING    = 1;
static const int OE_BJ_STATUS_PLAYER_WIN = 2;
static const int OE_BJ_STATUS_DEALER_WIN = 3;
static const int OE_BJ_STATUS_PUSH       = 4;
static const int OE_BJ_STATUS_BLACKJACK  = 5;
static const int OE_BJ_STATUS_ERROR      = 6;

static const int OE_BJ_ACTION_HIT        = 1;
static const int OE_BJ_ACTION_STAND      = 2;
static const int OE_BJ_ACTION_DOUBLE     = 3;
static const int OE_BJ_ACTION_SPLIT      = 4;


// Dice Betting RPCs and bet types.
static const int OE_CASINO_RPC_DICE_SYNC          = 1765401;
static const int OE_CASINO_RPC_DICE_ROLL          = 1765402;
static const int OE_CASINO_RPC_DICE_STATE         = 1765403;

static const int OE_DICE_STATUS_IDLE              = 0;
static const int OE_DICE_STATUS_WIN               = 1;
static const int OE_DICE_STATUS_LOSS              = 2;
static const int OE_DICE_STATUS_ERROR             = 3;

static const int OE_DICE_BET_LOW                  = 1;
static const int OE_DICE_BET_HIGH                 = 2;
static const int OE_DICE_BET_ODD                  = 3;
static const int OE_DICE_BET_EVEN                 = 4;
static const int OE_DICE_BET_DOUBLES              = 5;
static const int OE_DICE_BET_EXACT                = 6;

// Spin the Win game id, RPCs and prize types.
static const string OE_CASINO_GAME_SPIN_THE_WIN = "spin_the_win";

static const int OE_CASINO_RPC_SPIN_SYNC        = 1765501;
static const int OE_CASINO_RPC_SPIN_START       = 1765502;
static const int OE_CASINO_RPC_SPIN_STATE       = 1765503;

static const int OE_SPIN_STATUS_IDLE             = 0;
static const int OE_SPIN_STATUS_RESULT           = 1;
static const int OE_SPIN_STATUS_ERROR            = 2;

static const int OE_SPIN_PRIZE_LOSE              = 0;
static const int OE_SPIN_PRIZE_1X                = 1;
static const int OE_SPIN_PRIZE_2X                = 2;
static const int OE_SPIN_PRIZE_5X                = 5;
static const int OE_SPIN_PRIZE_JACKPOT           = 99;


// American double-zero Roulette game id, RPCs and bet types.
static const string OE_CASINO_GAME_ROULETTE = "roulette";

static const int OE_CASINO_RPC_ROULETTE_SYNC  = 1765601;
static const int OE_CASINO_RPC_ROULETTE_SPIN  = 1765602;
static const int OE_CASINO_RPC_ROULETTE_STATE = 1765603;

static const int OE_ROULETTE_STATUS_IDLE  = 0;
static const int OE_ROULETTE_STATUS_WIN   = 1;
static const int OE_ROULETTE_STATUS_LOSS  = 2;
static const int OE_ROULETTE_STATUS_ERROR = 3;
static const int OE_ROULETTE_STATUS_PUSH  = 4;

static const int OE_ROULETTE_BET_STRAIGHT = 1;
static const int OE_ROULETTE_BET_RED      = 2;
static const int OE_ROULETTE_BET_BLACK    = 3;
static const int OE_ROULETTE_BET_ODD      = 4;
static const int OE_ROULETTE_BET_EVEN     = 5;
static const int OE_ROULETTE_BET_LOW      = 6;
static const int OE_ROULETTE_BET_HIGH     = 7;
static const int OE_ROULETTE_BET_DOZEN_1  = 8;
static const int OE_ROULETTE_BET_DOZEN_2  = 9;
static const int OE_ROULETTE_BET_DOZEN_3  = 10;
static const int OE_ROULETTE_BET_COLUMN_1 = 11;
static const int OE_ROULETTE_BET_COLUMN_2 = 12;
static const int OE_ROULETTE_BET_COLUMN_3 = 13;
static const int OE_ROULETTE_BET_SPLIT    = 14;
static const int OE_ROULETTE_BET_STREET   = 15;
static const int OE_ROULETTE_BET_CORNER   = 16;
static const int OE_ROULETTE_BET_SIX_LINE = 17;
static const int OE_ROULETTE_BET_TOP_LINE = 18;

static const int OE_ROULETTE_MAX_BETS_PER_SPIN = 64;


// High Card / Casino War game id, RPCs and actions.
static const string OE_CASINO_GAME_HIGH_CARD_WAR = "high_card_war";

static const int OE_CASINO_RPC_WAR_SYNC   = 1765701;
static const int OE_CASINO_RPC_WAR_START  = 1765702;
static const int OE_CASINO_RPC_WAR_ACTION = 1765703;
static const int OE_CASINO_RPC_WAR_STATE  = 1765704;

static const int OE_WAR_STATUS_IDLE       = 0;
static const int OE_WAR_STATUS_TIE        = 1;
static const int OE_WAR_STATUS_PLAYER_WIN = 2;
static const int OE_WAR_STATUS_DEALER_WIN = 3;
static const int OE_WAR_STATUS_SURRENDER  = 4;
static const int OE_WAR_STATUS_WAR_WIN    = 5;
static const int OE_WAR_STATUS_WAR_LOSS   = 6;
static const int OE_WAR_STATUS_ERROR      = 7;

static const int OE_WAR_ACTION_SURRENDER = 1;
static const int OE_WAR_ACTION_GO_TO_WAR = 2;


// Craps game id, RPCs, line bets and phases.
// v0.11.2 table expansion: line bets + Odds, Come/Don't Come, Place and Field.
static const string OE_CASINO_GAME_CRAPS = "craps";

static const int OE_CASINO_RPC_CRAPS_SYNC       = 1765801;
static const int OE_CASINO_RPC_CRAPS_START      = 1765802;
static const int OE_CASINO_RPC_CRAPS_ROLL       = 1765803;
static const int OE_CASINO_RPC_CRAPS_STATE      = 1765804;
static const int OE_CASINO_RPC_CRAPS_BET_ACTION = 1765805;

static const int OE_CRAPS_STATUS_IDLE   = 0;
static const int OE_CRAPS_STATUS_ACTIVE = 1;
static const int OE_CRAPS_STATUS_WIN    = 2;
static const int OE_CRAPS_STATUS_LOSS   = 3;
static const int OE_CRAPS_STATUS_PUSH   = 4;
static const int OE_CRAPS_STATUS_ERROR  = 5;
static const int OE_CRAPS_STATUS_REJECTED = 6;

static const int OE_CRAPS_BET_PASS_LINE      = 1;
static const int OE_CRAPS_BET_DONT_PASS_LINE = 2;

static const int OE_CRAPS_SIDE_ODDS      = 10;
static const int OE_CRAPS_SIDE_COME      = 11;
static const int OE_CRAPS_SIDE_DONT_COME = 12;
static const int OE_CRAPS_SIDE_FIELD     = 13;
static const int OE_CRAPS_SIDE_PLACE     = 14;

static const int OE_CRAPS_ACTION_ADD   = 1;
static const int OE_CRAPS_ACTION_UNDO  = 2;
static const int OE_CRAPS_ACTION_CLEAR = 3;

static const int OE_CRAPS_PHASE_COME_OUT = 0;
static const int OE_CRAPS_PHASE_POINT    = 1;

// Double or Nothing game id, RPCs and states.
static const string OE_CASINO_GAME_DOUBLE_OR_NOTHING = "double_or_nothing";

static const int OE_CASINO_RPC_DON_SYNC    = 1765901;
static const int OE_CASINO_RPC_DON_START   = 1765902;
static const int OE_CASINO_RPC_DON_DOUBLE  = 1765903;
static const int OE_CASINO_RPC_DON_CASHOUT = 1765904;
static const int OE_CASINO_RPC_DON_STATE   = 1765905;
static const int OE_CASINO_RPC_DON_PICK    = 1765906;

static const int OE_DON_STATUS_IDLE     = 0;
static const int OE_DON_STATUS_ACTIVE   = 1;
static const int OE_DON_STATUS_LOSS     = 2;
static const int OE_DON_STATUS_CASHED   = 3;
static const int OE_DON_STATUS_MAX_WIN  = 4;
static const int OE_DON_STATUS_ERROR    = 5;
static const int OE_DON_STATUS_REJECTED = 6;
static const int OE_DON_STATUS_CHOOSE   = 7;

static const int OE_DON_CHOICE_NONE  = 0;
static const int OE_DON_CHOICE_RED   = 1;
static const int OE_DON_CHOICE_BLACK = 2;

// Heads-up Texas Hold'em (player vs dealer AI). The public station id stays
// casino_holdem so existing server locations do not need to be rebuilt.
static const string OE_CASINO_GAME_CASINO_HOLDEM = "casino_holdem";

static const int OE_CASINO_RPC_HOLDEM_SYNC   = 1766001;
static const int OE_CASINO_RPC_HOLDEM_START  = 1766002;
static const int OE_CASINO_RPC_HOLDEM_ACTION = 1766003;
static const int OE_CASINO_RPC_HOLDEM_STATE  = 1766004;

static const int OE_HOLDEM_STATUS_IDLE        = 0;
static const int OE_HOLDEM_STATUS_PLAYER_TURN = 1;
static const int OE_HOLDEM_STATUS_PLAYER_WIN  = 2;
static const int OE_HOLDEM_STATUS_DEALER_WIN  = 3;
static const int OE_HOLDEM_STATUS_PUSH        = 4;
static const int OE_HOLDEM_STATUS_PLAYER_FOLD = 5;
static const int OE_HOLDEM_STATUS_DEALER_FOLD = 6;
static const int OE_HOLDEM_STATUS_ERROR       = 7;
static const int OE_HOLDEM_STATUS_REJECTED    = 8;

// Compatibility alias for older v0.13.0 code paths/documentation.
static const int OE_HOLDEM_STATUS_DECISION = OE_HOLDEM_STATUS_PLAYER_TURN;

static const int OE_HOLDEM_ACTION_FOLD  = 1;
static const int OE_HOLDEM_ACTION_CALL  = 2;
static const int OE_HOLDEM_ACTION_CHECK = 3;
static const int OE_HOLDEM_ACTION_RAISE = 4;

static const int OE_HOLDEM_STREET_PREFLOP  = 0;
static const int OE_HOLDEM_STREET_FLOP     = 1;
static const int OE_HOLDEM_STREET_TURN     = 2;
static const int OE_HOLDEM_STREET_RIVER    = 3;
static const int OE_HOLDEM_STREET_SHOWDOWN = 4;

// Poker hand categories, weakest to strongest.
static const int OE_POKER_HAND_HIGH_CARD      = 0;
static const int OE_POKER_HAND_PAIR           = 1;
static const int OE_POKER_HAND_TWO_PAIR       = 2;
static const int OE_POKER_HAND_THREE_KIND     = 3;
static const int OE_POKER_HAND_STRAIGHT       = 4;
static const int OE_POKER_HAND_FLUSH          = 5;
static const int OE_POKER_HAND_FULL_HOUSE     = 6;
static const int OE_POKER_HAND_FOUR_KIND      = 7;
static const int OE_POKER_HAND_STRAIGHT_FLUSH = 8;
static const int OE_POKER_HAND_ROYAL_FLUSH    = 9;

