#pragma once

#include <random>

#include "api.h"
#include "cards.h"
#include "constants.h"

// Declarations for classes
class Player {
public:
  bool has_won;
  bool movement_in_progress;
  u_char n_removes;
  u_char next_card_free;
  u_char next_move_free;

  Player(u_char agent_id, std::default_random_engine *gen, DeckObs *data_buffer,
         std::array<float, N_RESOURCETYPES> *resources_ptr,
         ActionMask *am_buffer); // constructor
  Player();

  void initialize(u_char agent_id, std::default_random_engine *gen,
                  DeckObs *data_buffer, ActionMask *am_buffer,
                  ActionMask *persistent_am,
                  std::array<float, N_RESOURCETYPES> *res_buf);
  void reset();

  void play_card(CardType card_type, TurnPhase phase);
  spcl_action_ptr play_special(CardType card_type);
  void tag_for_removal(const std::array<u_char, N_CARDTYPES> &to_remove);
  void reset_resources();
  std::string describe_resources() const;
  void remove_cards(u_char n, bool enforce = true);
  void remove_from_hand(u_char card_type);
  void discard_cards(u_char n);
  void handle_requirement(Requirement requirement, u_char n);
  void pay(u_char n);
  void end_turn();
  void draw(u_char n);
  void add_card(u_char idx);
  void moved();
  void stepped();
  void disable_playing();
  void enable_playing();

  u_char get_id() const;
  DeckObs *get_player_data() const;
  unsigned int get_n_discarded() const;
  u_char get_n_removed() const;
  u_char get_n_active() const;
  unsigned int get_n_movements() const;
  u_char get_n_added_cards() const;
  u_char get_steps_taken() const;
  const Deck &get_deck() const;
  const std::array<unsigned int, N_RESOURCETYPES> &get_n_spent();
  const std::array<float, N_RESOURCETYPES> &get_resources();
  void load_actionmask();

private:
  DeckObs *player_data = nullptr;
  ActionMask *player_actionmask = nullptr;
  ActionMask *am_storage = nullptr;
  std::array<float, N_RESOURCETYPES> *resources_data = nullptr;

  u_char id;
  unsigned int n_movements;
  std::default_random_engine *gen = nullptr;
  u_char n_added_cards;
  u_char n_removed;
  unsigned int n_discarded;
  u_char steps_taken;
  Deck deck;
  std::array<unsigned int, N_RESOURCETYPES> n_spent;
  void save_actionmask();
};

inline TurnPhase cycle_phase(TurnPhase previous) {
  u_char i_previous = static_cast<u_char>(previous);
  u_char i_new = (i_previous + 1);
  u_char i_end = static_cast<u_char>(TurnPhase::MAX_PHASE);
  i_new -= static_cast<u_char>(i_end * (i_new >= i_end));
  return TurnPhase(i_new);
}
