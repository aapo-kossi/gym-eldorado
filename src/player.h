#pragma once

// Include necessary libraries if needed
#include <random>
#include <string>
#include "environment.h"
#include "cards.py"

// Declarations for constants, macros, and global variables
enum TurnPhase {INACTIVE, MOVEMENT, BUYING, DEAD};

// Declarations for classes
class Player {
public:
    template< class RNG >
    Player(
        ushort agent_id, RNG &gen = std::default_random_engine()
    ); // constructor

    ~Player(); // destructor

    CardFunctions playCard(Card card, bool useSpecial = false);
    void tagForRemoval(int parameter);
    void resetResources(int parameter);
    void resetResources(int parameter);
    ushort phaseObservation();
    std::string describeCards();
    std::string describeResources();
    void removeCards(ushort n, bool enforce = true);
};
