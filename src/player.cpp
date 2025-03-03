#include "api.h"
#include "cards.h"
#include "constants.h"
#include "player.h"
#include <cassert>
#include <string>

Player::Player(u_char agent_id, std::default_random_engine *gen_,
               DeckObs *data_buffer,
               std::array<float, N_RESOURCETYPES> *resources_buffer,
               ActionMask *am_buffer)
    : has_won(false), player_data(data_buffer), player_actionmask(am_buffer),
      resources_data(resources_buffer), id(agent_id), n_movements(), gen(gen_),
      n_added_cards(), deck(gen, data_buffer, am_buffer), n_spent{} {};

void Player::initialize(u_char agent_id, std::default_random_engine *gen_,
                        DeckObs *data_buffer, ActionMask *am_buffer,
                        ActionMask *persistent_am,
                        std::array<float, N_RESOURCETYPES> *res_buf) {
  id = agent_id;
  gen = gen_;
  player_data = data_buffer;
  player_actionmask = am_buffer;
  am_storage = persistent_am;
  resources_data = res_buf;
  deck.initialize(gen, player_data, player_actionmask);
}

void Player::reset() {
  has_won = false;
  movement_in_progress = false;
  next_card_free = false;
  next_move_free = false;
  n_removes = 0;
  steps_taken = 0;
  n_movements = 0;
  n_added_cards = 0;
  player_data->reset();
  player_actionmask->reset();
  deck.reset();
  save_actionmask();
  n_spent.fill(0);
}

Player::Player() {}; // default constructor (uninitialized)

void Player::play_card(CardType card_type, TurnPhase phase) {
  const Card &card = cards_by_type[card_type];

  switch (phase) {
  case TurnPhase::MOVEMENT:
    std::copy(card.resources.begin(), card.resources.end(),
              resources_data->begin());
    break;
  case TurnPhase::BUYING:
    if (card.resources[static_cast<u_char>(Resource::COIN)] > 0) {
      (*resources_data)[static_cast<u_char>(Resource::COIN)] +=
          card.resources[static_cast<u_char>(Resource::COIN)];
    } else {
      (*resources_data)[static_cast<u_char>(Resource::COIN)] += 0.5f;
    }
    break;
  default:
    assert(false &&
           "A player tried to play a card in an invalid environment state");
    break;
  }
  deck.activate(card_type);
};

spcl_action_ptr Player::play_special(CardType card_type) {
  const Card &card = cards_by_type[card_type];
  if (card.singleUse) {
    deck.remove_immediate(card_type);
  } else {
    deck.play_immediate(card_type);
  }
  return card.special_action;
}

std::string Player::describe_resources() const {
  return "Rendering player resources is a work in progress";
};

void Player::discard_cards(u_char n) {
  u_char n_discardable = deck.get_n_active();
  if (n > n_discardable) {
    assert(false && "Trying to discard more cards than have been activated!!");
  }

  for (u_char i = 0; i < n; i++) {
    std::uniform_int_distribution<size_t> dist(0, n_discardable - 1 - i);
    size_t target = dist(*gen);
    u_char card_type = 0;
    while (target >= player_data->active[card_type]) {
      target -= player_data->active[card_type];
      ++card_type;
    }

    deck.discard(card_type);
  }

  n_discarded += n;
}

void Player::remove_cards(u_char n, [[maybe_unused]] bool enforce) {
  u_char n_removable = deck.get_n_active();

  if (n > n_removable) {
    n = n_removable;
    assert(!enforce && "Cannot remove this number of cards: " && n);
  }

  for (u_char i = 0; i < n; i++) {
    std::uniform_int_distribution<size_t> dist(0, n_removable - 1 - i);
    size_t target = dist(*gen);

    // scan cards
    u_char card_type = 0;
    while (target >= player_data->active[card_type]) {
      target -= player_data->active[card_type];
      ++card_type;
    }

    assert((card_type < N_CARDTYPES) && "buffer overflow removing card");

    // remove the card
    deck.remove_active(card_type);
  }
  n_removed += n;
}

void Player::remove_from_hand(u_char card_type) {
  deck.remove_immediate(card_type);
}

void Player::pay(u_char n) {
  (*resources_data)[static_cast<u_char>(Resource::COIN)] -= n;
}

void Player::handle_requirement(Requirement requirement, u_char n) {
  u_char requirement_idx = static_cast<u_char>(requirement);
  if (requirement_idx < static_cast<u_char>(Resource::MAX_RESOURCE)) {
    float resource_left = (*resources_data)[requirement_idx] - n;
    resources_data->fill(0);
    (*resources_data)[requirement_idx] = resource_left;
    if (!movement_in_progress) {
      deck.play_last_activated();
      movement_in_progress = true;
    }
  } else if (requirement == Requirement::REMOVE) {
    remove_cards(n);
    (*resources_data).fill(0);
    movement_in_progress = false;
  } else if (requirement == Requirement::DISCARD) {
    discard_cards(n);
    (*resources_data).fill(0);
    movement_in_progress = false;
  } else {
    assert(false && "Unknown requirement!");
  }
}

void Player::reset_resources() { resources_data->fill(0); };

void Player::save_actionmask() { *am_storage = *player_actionmask; };

void Player::load_actionmask() { *player_actionmask = *am_storage; };

void Player::end_turn() {
  deck.discard_all_active();
  deck.discard_all_played();
  u_char remaining_cards = deck.get_n_in_hand();
  int n_draw = HAND_SIZE - remaining_cards;
  if (n_draw > 0) {
    deck.draw(static_cast<u_char>(n_draw));
  }
  reset_resources();
  save_actionmask();
};

void Player::draw(u_char n) { deck.draw(n); };

void Player::add_card(u_char idx) {
  deck.add(idx);
  n_added_cards++;
};
void Player::moved() { n_movements++; };
void Player::stepped() { steps_taken++; };

void Player::disable_playing() {
  player_actionmask->play.fill(false);
  player_actionmask->play[0] = true;
  player_actionmask->play_special.fill(false);
  player_actionmask->play_special[0] = true;
};

void Player::enable_playing() {
  player_actionmask->remove.fill(false);
  player_actionmask->remove[0] = true;
  for (u_char idx = 1; idx < player_actionmask->play.size(); idx++) {
    player_actionmask->play[idx] = player_data->hand[idx - 1] > 0;
    player_actionmask->play_special[idx] =
        player_actionmask->play[idx] && (cards_by_type[idx - 1].is_special);
  }
};

const std::array<unsigned int, N_RESOURCETYPES> &Player::get_n_spent() {
  return n_spent;
};

const std::array<float, N_RESOURCETYPES> &Player::get_resources() {
  return *resources_data;
};

u_char Player::get_id() const { return id; };

unsigned int Player::get_n_movements() const { return n_movements; };

DeckObs *Player::get_player_data() const { return player_data; };

u_char Player::get_n_added_cards() const { return n_added_cards; };
unsigned int Player::get_n_discarded() const { return n_added_cards; };
u_char Player::get_n_removed() const { return n_added_cards; };
u_char Player::get_n_active() const { return deck.get_n_active(); };

const Deck &Player::get_deck() const { return deck; };

u_char Player::get_steps_taken() const { return steps_taken; };
