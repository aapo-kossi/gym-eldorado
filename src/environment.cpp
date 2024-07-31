#include <random>
#include <cstdlib>
#include <iostream>

#include "environment.h"
#include "player.h"
#include "map.h"
#include "cards.h"

static const ushort RNG_WORDSIZE = 64;
static const ActionData NULL_ACTION;

eldorado_env::eldorado_env(
    int seed = NULL,
    ushort n_players = N_PLAYERS,
    ushort n_pieces = DEFAULT_N_PIECES,
    char* difficulty = DEFAULT_DIFFICULTY,
    unsigned int max_steps = 100000,
    bool render = false
) : seed(seed)
  , n_players(n_players)
  , n_pieces(n_pieces)
  , difficulty(difficulty)
  , max_steps(max_steps)
  , b_render(render)
  , rng(seed) {

};

eldorado_env::~eldorado_env() {};

void eldorado_env::reset(
    int seed = NULL,
    ushort n_players = N_PLAYERS,
    ushort n_pieces = DEFAULT_N_PIECES,
    char* difficulty = DEFAULT_DIFFICULTY,
    unsigned int max_steps = 100000,
    bool render = false
) {
        n_players = n_players;
        n_pieces = n_pieces;
        difficulty = difficulty;
        max_steps = max_steps;
        b_render = render;
        if ( seed != NULL ) {
            rng = default_random_generator(seed);
        }

        truncations = std::vector<bool>(n_players, false);
        rewards = std::vector<float>(n_players, 0.0);
        std::vector<AgentInfo> infos = {};
        EpisodeInfo ep_info = {0};

        info = {ep_info, infos};
        last_player = NULL;

        agent_selection = 0;

        map = Map::generate();
        std::vector<Player> players;
        for (auto i=0; i < n_players; i++) {
            players.push_back(Player(i, &rng));
        }
        map.add_players(n_players);
        shop = ;
        done = false;
        step_override = NULL;
        turn_counter = 0;
        std::vector<unsigned int> turn_counts(n_players, 0);
}

void eldorado_env::next_agent() {
    agent_selection = (agent_selection + 1);
    agent_selection -= n_players * (agent_selection >= n_players);
}

void eldorado_env::next_phase() {
    phase = (phase + 1);
    phase -= TurnPhase.END * (phase >= TurnPhase.END);
}

void eldorado_env::step( struct ActionData &action ) {
    bool dead_step = truncations[agent_selection];
    if ( dead_step ) { return; }

    if ( step_override != NULL ) {
        step_override( &action, &players[agent_selection], &map );
        maybe_cycle_players();
    }
        
    HexCoord loc = map.player_locations[agent_selection];
    done = map.hex_array[loc].is_end;
    if (done) { return; }
    
    maybe_cycle_phase();

    if ( phase == TurnPhase.MOVEMENT ) {
        movement_turn(&action);
    }
    else {
        shop_turn(&action);
    }
    maybe_cycle_phase();
    maybe_cycle_player();

    done = done | (turn_counter >= max_steps); 
    if ( done ) {
        info.episode_info.total_length = turn_counter;
        for ( auto agent = 0; agent < n_players; agent++ ) {
            Player player = players[agent];
            AgentInfo agent_info = info.agent_infos[agent];
            agent_info.turns_taken = turn_counts[agent];
            agent_info.returns = rewards[agent] = get_reward(agent);
            agent_info.travelled_hexes = player.n_movements;
            agent_info.cards_added = player.n_added_cards
            agent_info.cards_removed = player.n_removed_cards;
            agent_info.n_machete_uses = player.n_spent_machete;
            agent_info.n_paddle_uses = player.n_spent_paddle;
            agent_info.n_coin_uses = player.n_spent_coin;
            agent_info.n_card_uses = player.n_spent_card;
        }
        truncations = std::vector<bool>(n_players, true);
    }
};

inline void eldorado_env::movement_turn( struct ActionData &action ) {
    player = players[agent_selection];
    Direction dir = DIRECTIONS[action.move];
    if ( dir != Direction.NONE ) {
        MovementData data = map.move_in_direction(player, dir);
        player.n_spent[data.resource] += data.n_required;
        player.resources[data.resource] -= n_required;
        if ( data.resource == Resources.REMOVE ) {
            player.remove_cards(n_required);
        }
        player.has_won = data.fin;
        player.n_movements += 1;
    } else {
        player.tag_for_removal( &(action.remove) );
        maybe_play_card( &action );
}

inline void eldorado_env::shop_turn( struct ActionData &action ) {
    player = players[agent_selection];
    ushort desired_card = action.get_from_shop;
    if ( desired_card ) {
        ushort i = desired_card - 1;
        ushort cost = shop.costs[i];
        player.deck.add(shop.buy(i));
        player.resources[Resources.COIN] -= cost;
        player.n_spent[Resources.COIN] += cost;
        player.n_added_cards += 1;
    else {
        maybe_play_card( &action );
    }
}

inline void eldorado_env::maybe_cycle_phase(const ActionData &action ) {
    player = players[agent_selection];
    bool should_cycle = phase == TurnPhase.INACTIVE;
    bool not_playing = (action != NULL_ACTION) && (!action.play);
    bool in_shop = phase == TurnPhase.SHOP;
    bool movement_phase_still = (phase == TurnPhase.MOVEMENT) && (!action.move);
    should_cycle = should_cycle || (not_playing && (in_shop || movement_phase_still));
    }
    if ( should_cycle ) { next_phase(); }
}

inline void eldorado_env::maybe_cycle_player() {
    player = players[agent_selection];
    if ( player.has_won | (phase == TurnPhase.INACTIVE) ) {
        player.deck.discard_played();
        auto remaining_cards = player.deck.hand_size();
        auto n_draw = MIN_N_HAND - remaining_cards;
        if ( n_draw > 0 ) {
            player.deck.draw(n_draw);
        }
        player.reset_resources();
        next_agent();
        turn_counter++;
    }
}

void eldorado_env::observe( ushort agent ) {
    player = players[agent];
    obs.grid = map.observation(agent, grid_size);
    obs.phase = phase;
    obs.shop = shop.obs();
    obs.hand = player.deck.hand_obs();
    obs.played = player.deck.played_obs();
    obs.deck = player.deck.deck_obs();
    obs.discard = player.deck.discard_obs();
    obs.resources = player.resource_obs();

    action_mask.play = player.deck.hand_mask();
    action_mask.play_special = {1, 1};
    action_mask.remove = player.deck.hand_removable_mask();
    action_mask.move = map.movement_mask(agent, &(player.resources));
    action_mask.get_from_shop = shop.available_mask( &(player.resources[Resources.COIN]) );

    if ( mask_override != NULL ) {
        mask_override( &action_mask ) 
    }

};

inline void eldorado_env::maybe_play_card(const ActionData &action) {
    player = players[agent_selection];
    ushort played_card = action.play;
    bool playing(played_card);
    if (playing) {
        ushort i = played_card - 1;
        CardData card_data = player.play_card(i, action.play_special);
        special_action = CardData.special_action;
        step_override = CardData.step_override;
        mask_override = CardData.mask_override;

        if ( special_action != NULL ) {
            special_action(&action, &player, &map);
        }
    }
}

void eldorado_env::render() {
    if (b_render) {
        clear_console();
            if (!done) {
                player = players[agent_selection];
                cout << "\nCurrent map:\n" << endl;
                cout << map.draw() << endl;
                cout << "\nThe shop:" << endl;
                cout << shop.describe() << endl;
                cout << f"currently playing: {player.name}" << endl;
                cout << player.describe_cards() << endl;
                cout << player.describe_resources() << endl;
            }
            else {
                cout << "game over" << endl;
            }
    }
        else:
            cout << "You are calling render method without specifying any render mode." << endl;
        return
}

void clear_console() {
    #ifndef WINDOWS
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

