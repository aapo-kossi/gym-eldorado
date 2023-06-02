import numpy as np
from enum import Enum, auto
from cards import Card, Deck
from map import Direction, Resource


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

    def play_card(self, idx, use_special=False):
        ret_func = None
        ret_overrides = lambda _: {}
        phase = self.turn_phase
        card = self.deck.hand[idx]

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
            else:
                raise Exception("A player in an inactive turn state tried to play a card!")
        if card.single_use:
            self.deck.remove(card)
            return ret_func, ret_overrides
        self.deck.use(idx)
        return ret_func, ret_overrides
    
    def tag_for_removal(self, idx):
        idx = idx[:len(self.deck.hand)]
        for n, remove in enumerate(idx):
            self.deck.hand[n].tag_for_removal = remove
        self.resources[Resource.REMOVE] = sum(idx)
        
    def reset_resources(self):
        self.resources = np.zeros(len(Resource))

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
