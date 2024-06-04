// Include necessary libraries if needed
#include <random>
#include <string>
#include "map.h"

// Declarations for constants, macros, and global variables
const ushort CARDS_PER_TYPE = 3;
const ushort MKT_BOARD_SLOTS = 6;
const ushort HAND_SIZE = 4;
const ushort CARD_RESOURCETYPES = 3;
const ushort N_BUYABLETYPES = 18;
const ushort N_CARDTYPES = N_BUYABLETYPES + 3;

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


// Declarations for classes
class Card {
public:
    Card(); // constructor
    ~Card(); // destructor

    std::string to_string();

    CardType type;
    ushort cost;
    bool singleUse;
    ushort resources[CARD_RESOURCETYPES];
    std::string description;
    bool specialUse = false;
    bool tagForRemoval = false;
};

class Shop {
public:
    Shop(); // constructor
    ~Shop(); // destructor

    ushort nInMarket();
    Card buy(int idx);
    Card transmit(int idx);
    ushort* observation();
    bool* availableMask(ushort coins);
    std::string to_string();

    Card* cards[N_BUYABLETYPES];
    ushort costs[N_BUYABLETYPES];
    ushort n_types = N_BUYABLETYPES;
    ushort cards_per_type = CARDS_PER_TYPE;
    bool in_market[N_BUYABLETYPES];
    ushort n_available[CARD_RESOURCETYPES];
private:
    Card get(int idx);
};

class Deck {
public:
    template < class RNG >
    Deck(RNG &rng = std::default_random_engine()); // constructor
    ~Deck(); // destructor

    ushort nInMarket();
    Card buy(int idx);
    Card transmit(int idx);
    ushort* observation();
    bool* availableMask(ushort coins);
    std::string to_string();

    ushort all_cards[N_BUYABLETYPES];
    ushort discard_pile[N_BUYABLETYPES];
    ushort draw_pile[N_BUYABLETYPES];
    ushort hand[N_BUYABLETYPES];
    ushort played_cards[N_BUYABLETYPES];

    ushort* hand_mask(l;);
    void draw(ushort n = HAND_SIZE);
    void discard_played();
    void use(ushort idx);
    void remove(ushort idx);
    void add(ushort idx);

private:
    Card get(ushort idx);
    void shuffle();
};

// Inline functions or template declarations
inline void inlineFunction() {
    // Function definition here
}

template <typename T>
T templateFunction(T parameter) {
    // Function definition here
    return parameter;
}

