#pragma once

#include <array>
#include <random>
#include <string>

#include "api.h"

typedef void (*spcl_action_ptr)(ActionMask &action, Player &player, Map &map,
                                Shop &shop);

class Shop {
public:
  Shop(u_char cards_per_type = CARDS_PER_TYPE);
  void init(std::array<u_char, N_BUYABLETYPES> &obs_buffer);
  void reset();

  const Card &buy(size_t idx);
  const Card &transmit(size_t idx);
  void set_available_mask(float coins,
                          std::array<bool, N_BUYABLETYPES + 1> &mask) const;
  void set_transmit_mask(std::array<bool, N_BUYABLETYPES + 1> &mask) const;
  std::string to_string();
  std::string describe();

private:
  const Card &get(size_t idx);

  u_char cards_per_type;
  std::array<u_char, N_BUYABLETYPES> costs;
  u_char n_in_market;
  std::array<bool, N_BUYABLETYPES> in_market;
  std::array<u_char, N_BUYABLETYPES> *n_available;
};

class Deck {
public:
  Deck(std::default_random_engine *rng, DeckObs *data_buffer,
       ActionMask *am_buffer);

  Deck();

  void initialize(std::default_random_engine *rng, DeckObs *data_buffer,
                  ActionMask *am_buffer);
  void reset();

  u_char get_n_in_hand() const;
  u_char get_n_active() const;
  const std::string to_string() const;

  const std::array<bool, N_CARDTYPES + 1> &get_hand_mask() const;
  const std::array<std::array<bool, MAX_CARD_COPIES + 1>, N_CARDTYPES> &
  get_remove_mask() const;

  void draw(u_char n = HAND_SIZE);
  void discard(u_char idx);
  void remove_active(u_char idx);
  void remove_immediate(u_char idx);
  void discard_all_active();
  void discard_all_played();
  void move_discard_to_draw();
  void activate(u_char idx);
  void play_last_activated();
  void play_immediate(u_char idx);
  void add(u_char idx);

  DeckObs *get_player_data() const;

private:
  u_char n_in_hand;
  u_char n_active;
  u_char n_in_draw;
  u_char idx_last_activated;
  std::default_random_engine *rng;

  DeckObs *player_data;
  ActionMask *action_mask;

  const Card &get(u_char idx);
};

struct Card {

  const CardType type;
  const u_char cost;
  const bool starts_in_market;
  const bool singleUse;
  const std::array<u_char, CARD_RESOURCETYPES> resources;
  std::string_view description;
  spcl_action_ptr special_action = nullptr;
  bool is_special = false;

  std::string_view to_string() { return description; };

  Card(CardType t, u_char c, bool mkt, bool su,
       std::initializer_list<u_char> rs, std::string_view desc);
  Card(CardType t, u_char c, bool mkt, bool su,
       std::initializer_list<u_char> rs, std::string_view desc,
       spcl_action_ptr sa);
};

extern const std::array<Card, N_CARDTYPES> cards_by_type;
