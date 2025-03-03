#include "cards.h"
#include "constants.h"
#include "map.h"
#include "player.h"
#include <cassert>
#include <stdexcept>

template <u_char n>
void draw_action(ActionMask &, Player &player, Map &, Shop &) {
  player.draw(n);
};

template <u_char n, u_char m>
void draw_and_remove_action(ActionMask &mask, Player &player, Map &,
                            Shop &shop) {
  player.draw(n);
  player.n_removes = m;
  std::copy(mask.play.begin(), mask.play.end(), mask.remove.begin());
  player.disable_playing();
  shop.set_available_mask(0, mask.get_from_shop);
};

void transmit_action(ActionMask &mask, Player &p, Map &, Shop &shop) {
  mask.move.fill(false);
  mask.move[0] = true;
  p.disable_playing();
  shop.set_transmit_mask(mask.get_from_shop);
  p.next_card_free = true;
};

void native_action(ActionMask &mask, Player &player, Map &map, Shop &shop) {
  map.set_movement_mask(mask, player.get_id(), {100, 100, 100}, 100);
  player.next_move_free = true;
  player.disable_playing();
  shop.set_available_mask(0, mask.get_from_shop);
};

// FIXME: These are currently sensitive to being ordered identically to
// CardTypes in constants.h
const std::array<Card, N_CARDTYPES> cards_by_type = {
    Card(EXPLORER, 1, false, false, {1, 0, 0}, "Explorer"),
    Card(SCOUT, 1, true, false, {2, 0, 0}, "Scout"),
    Card(TRAILBLAZER, 3, true, false, {3, 0, 0}, "Trailblazer"),
    Card(PIONEER, 5, false, false, {5, 0, 0}, "Pioneer"),
    Card(GIANT_MACHETE, 3, false, true, {6, 0, 0}, "Giant machete"),

    Card(SAILOR, 1, false, false, {0, 1, 0}, "Sailor"),
    Card(CAPTAIN, 2, false, false, {0, 3, 0}, "Captain"),

    Card(TRAVELER, 1, false, false, {0, 0, 1}, "Traveler"),
    Card(PHOTOGRAPHER, 2, true, false, {0, 0, 2}, "Photographer"),
    Card(JOURNALIST, 3, false, false, {0, 0, 3}, "Journalist"),
    Card(TREASURE_CHEST, 3, true, false, {0, 0, 4}, "Treasure chest"),
    Card(MILLIONAIRE, 5, false, false, {0, 0, 4}, "Millionaire"),

    Card(JACK_OF_ALL_TRADES, 2, true, false, {1, 1, 1}, "Jack of all trades"),
    Card(ADVENTURER, 4, false, false, {2, 2, 2}, "Adventurer"),
    Card(PROP_PLANE, 4, false, true, {4, 4, 4}, "Prop plane"),

    Card(TRANSMITTER, 4, true, true, {0, 0, 0}, "Transmitter",
         spcl_action_ptr(transmit_action)),
    Card(CARTOGRAPHER, 4, false, false, {0, 0, 0}, "Cartographer",
         spcl_action_ptr(draw_action<2>)),
    Card(COMPASS, 2, false, true, {0, 0, 0}, "Compass",
         spcl_action_ptr(draw_action<3>)),
    Card(SCIENTIST, 4, false, false, {0, 0, 0}, "Scientist",
         spcl_action_ptr(draw_and_remove_action<1, 1>)),
    Card(TRAVEL_LOG, 3, false, true, {0, 0, 0}, "Travel log",
         spcl_action_ptr(draw_and_remove_action<2, 2>)),
    Card(NATIVE, 5, false, false, {0, 0, 0}, "Native",
         spcl_action_ptr(native_action))};
std::array<const Card *, N_BUYABLETYPES> shop_cards = {
    &cards_by_type[1],  &cards_by_type[2],  &cards_by_type[3],
    &cards_by_type[4],  &cards_by_type[6],  &cards_by_type[8],
    &cards_by_type[9],  &cards_by_type[10], &cards_by_type[11],
    &cards_by_type[12], &cards_by_type[13], &cards_by_type[14],
    &cards_by_type[15], &cards_by_type[16], &cards_by_type[17],
    &cards_by_type[18], &cards_by_type[19], &cards_by_type[20]};

void Shop::init(std::array<u_char, N_BUYABLETYPES> &obs_buffer) {
  n_available = &obs_buffer;
  n_available->fill(cards_per_type); // a new shop is full
};

Shop::Shop(u_char cards_per_type_)
    : cards_per_type(cards_per_type_), n_in_market(MKT_BOARD_SLOTS) {
  for (size_t i = 0; i < N_BUYABLETYPES; i++) {
    const Card &card = *shop_cards[i];
    in_market[i] = card.starts_in_market;
    costs[i] = card.cost;
  }
};

void Shop::reset() {
  n_available->fill(cards_per_type);
  for (size_t i = 0; i < shop_cards.size(); i++) {
    const Card &card = *shop_cards[i];
    in_market[i] = card.starts_in_market;
  }
}

const Card &Shop::buy(size_t idx) {
  n_in_market += static_cast<u_char>(1 - in_market[idx]);
  in_market[idx] = true;
  return get(idx);
};
const Card &Shop::transmit(size_t idx) { return get(idx); };

void Shop::set_available_mask(
    float coins, std::array<bool, N_BUYABLETYPES + 1> &mask) const {

  if (n_in_market < MKT_BOARD_SLOTS) {
    for (size_t i = 0; i < costs.size(); i++) {
      mask[i + 1] = ((*n_available)[i] > 0) && (coins > costs[i]);
    }
  } else {
    for (size_t i = 0; i < costs.size(); i++) {
      mask[i + 1] = in_market[i] && (coins > costs[i]);
    }
  }
};

void Shop::set_transmit_mask(std::array<bool, N_BUYABLETYPES + 1> &mask) const {
  for (size_t i = 0; i < costs.size(); i++) {
    mask[i + 1] = (*n_available)[i] > 0;
  }
};

std::string Shop::to_string() {
  return "Rendering the shop is a work in progress";
};
std::string Shop::describe() {
  return "Rendering the shop is a work in progress";
};

const Card &Shop::get(size_t idx) {
  if (!--(*n_available)[idx] && in_market[idx]) {
    in_market[idx] = false;
    n_in_market -= 1;
  }
  return *shop_cards[idx];
};

Deck::Deck(std::default_random_engine *rng_, DeckObs *data_buffer,
           ActionMask *am_buffer)
    : n_in_hand(0), n_in_draw(0), rng(rng_), player_data(data_buffer),
      action_mask(am_buffer) {
  player_data->discard[EXPLORER] = 3;
  player_data->discard[TRAVELER] = 4;
  player_data->discard[SAILOR] = 1;
  draw();
}; // constructor

Deck::Deck() : n_in_hand(0), n_in_draw(0) {};

void Deck::initialize(std::default_random_engine *rng_, DeckObs *data_buffer,
                      ActionMask *am_buffer) {
  rng = rng_;
  player_data = data_buffer;
  action_mask = am_buffer;
}

void Deck::reset() {
  player_data->discard[EXPLORER] = 3;
  player_data->discard[TRAVELER] = 4;
  player_data->discard[SAILOR] = 1;
  n_in_draw = 0;
  n_in_hand = 0;
  n_active = 0;
  draw();
}

const std::string Deck::to_string() const {
  return "Rendering the deck is a work in progress";
};

u_char Deck::get_n_active() const { return n_active; };

u_char Deck::get_n_in_hand() const { return n_in_hand; };

DeckObs *Deck::get_player_data() const { return player_data; };

void Deck::draw(u_char n) {
  if (n_in_draw < n) {
    move_discard_to_draw();
  }
  n = std::min(n, n_in_draw);

  for (int i = 0; i < n; i++) {
    std::uniform_int_distribution<size_t> dist(0, n_in_draw - 1);
    size_t target = dist(*rng);

    // Scan drawpile until target count is hit
    size_t card_type = 0;
    while (target >= player_data->draw[card_type]) {
      target -= player_data->draw[card_type];
      ++card_type;
    }

    // Draw the card
    --player_data->draw[card_type];
    --n_in_draw;
    ++player_data->hand[card_type];
    action_mask->play[card_type + 1] = true;
    action_mask->play_special[card_type + 1] =
        cards_by_type[card_type].is_special;
    assert((card_type <= N_CARDTYPES) &&
           "buffer overflow trying to draw a card");
  }
  n_in_hand += n;
};

void Deck::discard(u_char idx) {
  --n_active;
  player_data->active[idx]--;
  player_data->discard[idx]++;
};

void Deck::discard_all_active() {
  n_active = 0;
  for (size_t i = 0; i < N_CARDTYPES; i++) {
    player_data->discard[i] += player_data->active[i];
    player_data->active[i] = 0;
  }
};

void Deck::discard_all_played() {
  for (size_t i = 0; i < N_CARDTYPES; i++) {
    player_data->discard[i] += player_data->played[i];
    player_data->played[i] = 0;
  }
};

void Deck::move_discard_to_draw() {
  for (size_t i = 0; i < N_CARDTYPES; i++) {
    player_data->draw[i] += player_data->discard[i];
    n_in_draw += player_data->discard[i];
    player_data->discard[i] = 0;
  }
};

void Deck::activate(u_char idx) {

  --n_in_hand;
  ++n_active;
  idx_last_activated = idx;
  u_char prev_n_in_hand = player_data->hand[idx]--;
  assert((prev_n_in_hand >= player_data->hand[idx]) && "Over-playing!");
  ++player_data->active[idx];
  action_mask->play[idx + 1] = prev_n_in_hand > 1;
  action_mask->play_special[idx + 1] =
      action_mask->play[idx + 1] && (cards_by_type[idx].is_special);
};

void Deck::play_last_activated() {
  --n_active;
  --player_data->active[idx_last_activated];
  if (!cards_by_type[idx_last_activated].singleUse) {
    ++player_data->played[idx_last_activated];
  }
};

void Deck::play_immediate(u_char idx) {

  --n_in_hand;
  u_char prev_n_in_hand = player_data->hand[idx]--;
  assert((prev_n_in_hand >= player_data->hand[idx]) &&
         "Over-playing card with special action!");
  ++player_data->played[idx];
  action_mask->play[idx + 1] = prev_n_in_hand > 1;
  action_mask->play_special[idx + 1] =
      action_mask->play[idx + 1] && (cards_by_type[idx].is_special);
};

void Deck::remove_active(u_char idx) {
  --n_active;
  [[maybe_unused]] u_char prev_n_active = player_data->active[idx]--;
  assert((prev_n_active >= player_data->active[idx]) && "Over-removing!");
};

void Deck::remove_immediate(u_char idx) {
  --n_in_hand;
  u_char prev_n_in_hand = player_data->hand[idx]--;
  assert((prev_n_in_hand >= player_data->hand[idx]) && "Over-removing!");
  action_mask->remove[idx + 1] =
      action_mask->remove[idx + 1] && prev_n_in_hand > 1;
  action_mask->play[idx + 1] = action_mask->play[idx + 1] && prev_n_in_hand > 1;
  action_mask->play_special[idx + 1] =
      action_mask->play[idx + 1] && (cards_by_type[idx].is_special);
};

void Deck::add(u_char idx) { ++player_data->discard[idx]; };

const Card &Deck::get(u_char idx) { return cards_by_type[idx]; };

// constructor for basic cards
Card::Card(CardType t, u_char c, bool mkt, bool su,
           std::initializer_list<u_char> rs, std::string_view desc)
    : type(t), cost(c), starts_in_market(mkt), singleUse(su),
      resources([&rs]() {
        if (rs.size() != CARD_RESOURCETYPES) {
          throw std::invalid_argument("Incorrect number of resource elements.");
        }
        std::array<u_char, 3> temp{};
        std::copy(rs.begin(), rs.end(), temp.begin());
        return temp;
      }()),
      description(desc) {};

// constructor for special cards
Card::Card(CardType t, u_char c, bool mkt, bool su,
           std::initializer_list<u_char> rs, std::string_view desc,
           spcl_action_ptr sa)
    : type(t), cost(c), starts_in_market(mkt), singleUse(su),
      resources([&rs]() {
        if (rs.size() != CARD_RESOURCETYPES) {
          throw std::invalid_argument("Incorrect number of resource elements.");
        }
        std::array<u_char, 3> temp{};
        std::copy(rs.begin(), rs.end(), temp.begin());
        return temp;
      }()),
      description(desc), special_action(sa), is_special(true) {};
