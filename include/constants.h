#pragma once
#include <cstdint>

using u_char = uint8_t;

class Map;
class Player;
class Shop;
struct Card;

enum class Difficulty { EASY, MEDIUM, HARD };
constexpr Difficulty DEFAULT_DIFFICULTY = Difficulty::EASY;

// cards
const u_char CARDS_PER_TYPE = 3;
const u_char MAX_CARD_COPIES = 4;
const u_char MKT_BOARD_SLOTS = 6;
const u_char HAND_SIZE = 4;
const u_char CARD_RESOURCETYPES = 3;
const u_char N_BUYABLETYPES = 18;
const u_char N_CARDTYPES = N_BUYABLETYPES + 3;
enum CardType {
  // Machete cards
  EXPLORER,
  SCOUT,
  TRAILBLAZER,
  PIONEER,
  GIANT_MACHETE,

  // Paddle cards
  SAILOR,
  CAPTAIN,

  // Gold cards
  TRAVELER,
  PHOTOGRAPHER,
  JOURNALIST,
  TREASURE_CHEST,
  MILLIONAIRE,

  // Multi-resource cards
  JACK_OF_ALL_TRADES,
  ADVENTURER,
  PROP_PLANE,

  // Special cards
  TRANSMITTER,
  CARTOGRAPHER,
  COMPASS,
  SCIENTIST,
  TRAVEL_LOG,
  NATIVE,
};

// map
const u_char N_MAP_FEATURES = 7; // Player locations, resources, is_end
const u_char GRIDSIZE = 48;
const u_char DEFAULT_N_PIECES = 3;
enum class Resource : u_char { MACHETE = 0, PADDLE, COIN, MAX_RESOURCE };
enum class Requirement : u_char {
  MACHETE = static_cast<u_char>(Resource::MACHETE),
  PADDLE = static_cast<u_char>(Resource::PADDLE),
  COIN = static_cast<u_char>(Resource::COIN),

  DISCARD,
  REMOVE,
  NULL_REQUIREMENT
};
constexpr u_char N_RESOURCETYPES = static_cast<u_char>(Resource::MAX_RESOURCE);
constexpr u_char N_REQUIREMENTS =
    static_cast<u_char>(Requirement::NULL_REQUIREMENT);

// environment
constexpr u_char MAX_N_PLAYERS = 4;
constexpr u_char MAX_FAILURES = 5;
enum class TurnPhase { INACTIVE = 0, MOVEMENT, BUYING, MAX_PHASE };
constexpr u_char N_PHASES = sizeof(TurnPhase);
constexpr unsigned int MAX_STEPS = 100000;
