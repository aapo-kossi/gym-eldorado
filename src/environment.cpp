
#include "player.h"
#include "map.h"
#include "cards.h"

typedef unsigned short ushort
ushort GRIDSIZE = 48;

// Define globals for python interface
struct Observation {
    ushort grid[GRIDSIZE][GRIDSIZE];
    ushort phase;
    ushort shop[N_BUYABLETYPES];
    ushort hand[N_CARDTYPES];
    ushort played[N_CARDTYPES];
    ushort resources[N_RESOURCETYPES];
    ushort deck[N_CARDTYPES];
    ushort discard[N_CARDTYPES];
};
struct ActionMask {
    ushort play[N_CARDTYPES + 1];
    ushort play_special;
    ushort remove[N_CARDTYPES][5];
    ushort move[N_DIRECTIONS];
    ushort get_from_shop[N_BUYABLETYPES + 1];
};
struct ObsData {
    struct Observation obs;
    struct ActionMask action_mask;
};

struct ActionData {
    ushort play[game.N_CARDTYPES + 1];
    ushort play_special[2];
    ushort remove[5][N_CARDTYPES];
    ushort move[N_DIRECTIONS];
    ushort get_from_shop[N_BUYABLETYPES + 1];
};

struct EpisodeInfo {
    unsigned int total_length;
};
struct AgentInfo {
    ushort turns_taken;
    ushort returns;
    ushort travelled_hexes;
    ushort cards_added;
    ushort cards_removed;
    ushort n_machete_uses;
    ushort n_paddle_uses;
    ushort n_coin_uses;
    ushort n_card_uses;
};
struct Info {
    struct EpisodeInfo episode_info;
    struct AgentInfo agent_infos[N_PLAYERS];
};
char render_data[1 << 13]

// Declarations for functions
void myFunction(int parameter1, double parameter2);

// Declarations for classes
class EldoradoEnv{
public:
    MyClass(
        int seed = NULL,
        ushort n_players = N_PLAYERS,
        ushort n_pieces = DEFAULT_N_PIECES,
        char* difficulty = DEFAULT_DIFFICULTY,
        unsigned int max_steps = 100000,
        bool render = false
    )
    ~MyClass();

    void reset(
        int seed = NULL,
        ushort n_players = N_PLAYERS,
        ushort n_pieces = DEFAULT_N_PIECES,
        char* difficulty = DEFAULT_DIFFICULTY,
        unsigned int max_steps = 100000,
        bool render = false
    );

    void step(
        struct ActionData action
    );

    void observe(
        ushort agent
    );

    void render();

private:
    // Private members or helper functions
    ;
};
