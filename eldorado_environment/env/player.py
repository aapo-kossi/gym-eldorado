import numpy as np
from enum import Enum, auto
from eldorado_environment.env.cards import Card, Deck
from eldorado_environment.env.map import Direction, Resource


class TurnPhase(Enum):
    INACTIVE = 0
    MOVEMENT = 1
    BUYING   = 2
    DEAD     = 3

class Player:

    def __init__(self, agent, id, rng=np.random.default_rng()):
        self.rng = rng
        self.deck = Deck(rng=rng)
        self.agent = agent
        self.name = str(agent)
        self.id = id
        self.has_won = False
        self.reset_resources()
        self.turn_phase = TurnPhase.INACTIVE
        self.num_movements = 0
        self.num_added_cards = 0
        self.num_removed_cards = 0
        self.num_spent = {r: 0 for r in Resource}


    def play_card(self, cardtype, use_special=False):
        ret_func = None
        ret_overrides = lambda _: {}
        phase = self.turn_phase
        card = None
        i = 0
        while card is None:
            checked = self.deck.hand[i]
            if checked.type == cardtype:
                card = checked
            else:
                i += 1

        # Cancel the tag for removal, since the card is used
        if card.tag_for_removal:
            self.resources[Resource.REMOVE] -= 1
            card.tag_for_removal = False

        # handle special uses like Native, Transmitter, Compass
        if use_special and card.special_use:
            ret_func =  card.special_use
            ret_overrides = card.overrides
        else:
            if phase == TurnPhase.MOVEMENT:
                for R, n in zip(card.resource_types, card.resources):
                    self.resources[R] = n
                self.resources[Resource.USE] += 1
            elif phase == TurnPhase.BUYING:
                if card.resources[Resource.COIN] > 0:
                    self.resources[Resource.COIN] += card.resources[Resource.COIN]
                else:
                    self.resources[Resource.COIN] += 0.5

                # clamp coins to a maximum of 5, as no cards have a higher cost
                self.resources[Resource.COIN] = min(self.resources[Resource.COIN],5)
            else:
                raise Exception("A player in an inactive turn state tried to play a card!")
        remove_cond = card.single_use and not (card.special_use and not use_special)
        if remove_cond:
            self.deck.remove(card)
            return ret_func, ret_overrides
        self.deck.use(i)
        return ret_func, ret_overrides

    def tag_for_removal(self, counts):
        self.resources[Resource.REMOVE] = np.sum(counts)
        for card in self.deck.hand:
            if counts[card.type]:
                counts[card.type] -= 1
                card.tag_for_removal = True
            else: card.tag_for_removal = False

    def reset_resources(self):
        self.resources = np.zeros(len(Resource), dtype=np.float32)

    def resource_observation(self):
        return self.resources

    def phase_observation(self):
        return int(self.turn_phase == TurnPhase.BUYING)

    def describe_cards(self):
        s = "Current hand:\n"
        for card in self.deck.hand:
            s += type(card).__name__ + "\n"

        s += "\nCards played this turn:\n"
        for card in self.deck.played_cards:
            s += type(card).__name__ + "\n"

        s += "\nCurrent discard pile:\n"
        for card in self.deck.discard_pile:
            s += type(card).__name__ + " "

        return s

    def describe_resources(self):
        s = "Current resources:\n"
        for R, n in zip(Resource, self.resources):
            s += R.name + ": " + str(n) + "\n"
        return s

    def remove_cards(self, num_to_remove, enforce=True):
        removable = np.array([float(card.tag_for_removal) for card in self.deck.hand])
        n_removable = np.sum(removable)

        if enforce and n_removable < num_to_remove:
            raise Exception("Not enough cards tagged for removal, action is not valid!")

        hand_idx = np.arange(len(removable))
        removed_idx = self.rng.choice(hand_idx, size=num_to_remove, p=removable / n_removable, replace=False)
        removed_idx[::-1].sort()
        for idx in removed_idx:
            self.deck.remove(self.deck.hand[idx])
        self.num_removed_cards += len(removed_idx)

    @property
    def num_machetes_spent(self):
        return self.num_spent[Resource.MACHETE]

    @property
    def num_paddles_spent(self):
        return self.num_spent[Resource.PADDLE]

    @property
    def num_coins_spent(self):
        return self.num_spent[Resource.COIN]

    @property
    def num_cards_spent(self):
        return self.num_spent[Resource.USE]
