import random
import numpy as np
from dataclasses import dataclass
from enum import IntEnum, auto
from eldorado_environment.env.map import Resource

CARDS_PER_TYPE = 3
MKT_BOARD_SLOTS = 6
HAND_SIZE = 4

class CardType(IntEnum):

    EXPLORER = 0
    SCOUT = 1
    TRAILBLAZER = 2
    PIONEER = 3
    GIANT_MACHETE = 4

    SAILOR = 5
    CAPTAIN = 6

    TRAVELER = 7
    PHOTOGRAPHER = 8
    JOURNALIST = 9
    TREASURE_CHEST = 10
    MILLIONAIRE = 11

    JACK_OF_ALL_TRADES = 12
    ADVENTURER = 13
    PROP_PLANE = 14

    TRANSMITTER = 15
    CARTOGRAPHER = 16
    COMPASS = 17
    SCIENTIST = 18
    TRAVEL_LOG = 19
    NATIVE = 20
    

@dataclass
class Card:
    type: CardType
    cost: int
    single_use: bool
    resources: tuple[int]
    resource_types: tuple[Resource] = (
        Resource.MACHETE,
        Resource.PADDLE,
        Resource.COIN,
    )
    special_use: bool = False
    tag_for_removal = False

@dataclass
class Explorer(Card):
    type: CardType = CardType.EXPLORER
    cost: int = 0
    single_use: bool = False
    resources: tuple[int] = (1,0,0)

@dataclass
class Scout(Card):
    type: CardType = CardType.SCOUT
    cost: int = 1
    single_use: bool = False
    resources: tuple[int] = (2,0,0)

@dataclass
class Trailblazer(Card):
    type: CardType = CardType.TRAILBLAZER
    cost: int = 3
    single_use: bool = False
    resources: tuple[int] = (3,0,0)

@dataclass
class Pioneer(Card):
    type: CardType = CardType.PIONEER
    cost: int = 5
    single_use: bool = False
    resources: tuple[int] = (5,0,0)

@dataclass
class GiantMachete(Card):
    type: CardType = CardType.GIANT_MACHETE
    cost: int = 3
    single_use: bool = True
    resources: tuple[int] = (6,0,0)

@dataclass
class Sailor(Card):
    type: CardType = CardType.SAILOR
    cost: int = 0
    single_use: bool = False
    resources: tuple[int] = (0,1,0)

@dataclass
class Captain(Card):
    type: CardType = CardType.CAPTAIN
    cost: int = 2
    single_use: bool = False
    resources: tuple[int] = (0,3,0)

@dataclass
class Traveler(Card):
    type: CardType = CardType.TRAVELER
    cost: int = 0
    single_use: bool = False
    resources: tuple[int] = (0,0,1)

@dataclass
class Photographer(Card):
    type: CardType = CardType.PHOTOGRAPHER
    cost: int = 2
    single_use: bool = False
    resources: tuple[int] = (0,0,3)

@dataclass
class Journalist(Card):
    type: CardType = CardType.JOURNALIST
    cost: int = 3
    single_use: bool = False
    resources: tuple[int] = (0,0,3)

@dataclass
class TreasureChest(Card):
    type: CardType = CardType.TREASURE_CHEST
    cost: int = 3
    single_use: bool = True
    resources: tuple[int] = (0,0,4)

@dataclass
class Millionaire(Card):
    type: CardType = CardType.MILLIONAIRE
    cost: int = 5
    single_use: bool = False
    resources: tuple[int] = (0,0,4)

@dataclass
class JackOfAllTrades(Card):
    type: CardType = CardType.JACK_OF_ALL_TRADES
    cost: int = 2
    single_use: bool = False
    resources: tuple[int] = (1,1,1)

@dataclass
class Adventurer(Card):
    type: CardType = CardType.ADVENTURER
    cost: int = 4
    single_use: bool = False
    resources: tuple[int] = (2,2,2)

@dataclass
class PropPlane(Card):
    type: CardType = CardType.PROP_PLANE
    cost: int = 4
    single_use: bool = True
    resources: tuple[int] = (4,4,4)

@dataclass
class Transmitter(Card):
    type: CardType = CardType.TRANSMITTER
    cost: int = 4
    single_use: bool = True
    resources: tuple[int] = (0,0,0)

    def special_use(self, game):
        return None
    
    def next_turn(self, action, game):
        desired_card = action["get_from_shop"]
        if desired_card:
            game.selected_player.deck.add(game.shop.transmit(desired_card - 1))
        return game.selected_player.agent, False

    def overrides(self, game):
        shop_mask = np.ones(game.shop.n_types + 1, dtype=np.int8)
        return {
            "action_mask": {
                "get_from_shop": shop_mask,
            },
            "step": self.next_turn,
        }


@dataclass
class Cartographer(Card):
    type: CardType = CardType.CARTOGRAPHER
    cost: int = 4
    single_use: bool = False
    resources: tuple[int] = (0,0,0)

    def special_use(self, action, game):
        game.selected_player.deck.draw(2)
        return
    
    def overrides(self, game):
        return {}

@dataclass
class Compass(Card):
    type: CardType = CardType.COMPASS
    cost: int = 2
    single_use: bool = True
    resources: tuple[int] = (0,0,0)

    def special_use(self, action, game):
        game.selected_player.deck.draw(3)
        return
    
    def overrides(self, game):
        return {}

@dataclass
class Scientist(Card):
    type: CardType = CardType.SCIENTIST
    cost: int = 4
    single_use: bool = False
    resources: tuple[int] = (0,0,0)

    def special_use(self, action, game):
        game.selected_player.deck.draw(1)

    def next_turn(self, action, game):
        remove_cards = action["remove"]
        game.selected_player.tag_for_removal(remove_cards)
        game.selected_player.remove_cards(1, enforce=False)
        return game.selected_player.agent, False

    def overrides(self, game):
        return {
            "step": self.next_turn
        }


@dataclass
class TravelLog(Card):
    type: CardType = CardType.TRAVEL_LOG
    cost: int = 3
    single_use: bool = True
    resources: tuple[int] = (0,0,0)

    def special_use(self, action, game):
        game.selected_player.deck.draw(2)

    def next_turn(self, action, game):
        remove_cards = action["remove"]
        game.selected_player.tag_for_removal(remove_cards)
        game.selected_player.remove_cards(2, enforce=False)
        return game.selected_player.agent, False

    def overrides(self, game):
        return {
            "step": self.next_turn
        }

@dataclass
class Native(Card):
    type: CardType = CardType.NATIVE
    cost: int = 5
    single_use: bool = False
    resources: tuple[int] = (0,0,0)

    def special_use(self, action, game):
        return None
    
    def next_turn(self, action, game):
        target_direction = game._idx_direction_map[action["move"]]
        resource, n_required, fin = game.map.move_in_direction(game.selected_player, target_direction)
        game.selected_player.has_won = fin

    def overrides(self, game):
        movement_mask = game.map.movement_mask(game.selected_player, np.full(game.n_resourcetypes, 10))
        return {
            "action_mask": {
                "move": movement_mask
            },
            "step": self.next_turn
        }

class Shop:

    cards = (
        Scout,
        Trailblazer,
        Pioneer,
        GiantMachete,
        Captain,
        Photographer,
        Journalist,
        TreasureChest,
        Millionaire,
        JackOfAllTrades,
        Adventurer,
        PropPlane,
        Transmitter,
        Cartographer,
        Compass,
        Scientist,
        TravelLog,
        Native,
    )
    costs = np.array([
        s.cost for s in cards
    ])

    n_types = len(cards)
    cards_per_type = CARDS_PER_TYPE

    def __init__(self):
        self.in_market = np.zeros(self.n_types, dtype=np.int8)
        self.in_market[[0,1,9,5,7,12]] = 1

        self.n_available = np.array([CARDS_PER_TYPE] * len(self.cards))

    @property
    def n_in_market(self):
        return sum(int(x) for x in self.in_market)

    def buy(self, idx):
        if not self.in_market[idx]:
            if self.n_in_market >= MKT_BOARD_SLOTS:
                raise ValueError(f"The market board is full and card {idx} is not in it, cannot buy!")
            else:
                self.in_market[idx] = 1
        return self.get(idx)
    
    def transmit(self, idx):
        return self.get(idx)
    
    def get(self, idx):

        # return the requested card if available
        if self.n_available[idx] > 0:
            self.n_available[idx] -= 1
            
            # free a market board slot if there are no cards left
            if self.n_available[idx] == 0:
                self.in_market[idx] = 0
            return self.cards[idx]()
        return None
    
    def observation(self):
        return self.n_available, self.in_market
    
    def available_mask(self, coins):
        mask = np.zeros(self.n_types + 1, dtype=np.int8)
        mask[0] = 1
        can_afford = coins >= self.costs
        if self.n_in_market < MKT_BOARD_SLOTS:
            mask[1:] = can_afford & (self.n_available > 0)
        else:
            mask[1:] = can_afford & self.in_market & (self.n_available > 0)
        return mask

class Deck:
    def __init__(self, rng=np.random.default_rng()):
        self.rng = rng
        self._all_cards = [
            Explorer(),
            Explorer(),
            Explorer(),
            Sailor(),
            Traveler(),
            Traveler(),
            Traveler(),
            Traveler(),
        ]
        self._discard_pile = list(self._all_cards)
        self._draw_pile = []
        self._hand = []
        self._played_cards = []
        self._shuffle()
        self.draw()

    @property
    def hand(self):
        return self._hand

    @property
    def played_cards(self):
        return self._played_cards

    @property
    def discard_pile(self):
        return self._discard_pile
    
    def discarded_observation(self):
        return self.convert_to_unordered_obs(self._discard_pile)
    
    def hand_observation(self, n):
        return self.convert_to_obs(self._hand, n)
    
    def played_observation(self, n):
        return self.convert_to_obs(self._played_cards, n)
    
    def all_cards_observation(self):
        return self.convert_to_unordered_obs(self._all_cards)
    
    def hand_mask(self, n):
        count = len(self._hand)
        mask = np.zeros(n, dtype=np.int8)
        mask[:count+1] = 1
        return mask
    
    def hand_removable_mask(self, n):
        return self.hand_mask(n+1)[1:] * 2

    @staticmethod
    def convert_to_obs(cards, max_size):
        obs = np.zeros((max_size, len(CardType)), dtype=np.int8)
        idx = [card.type for card in cards]
        pos = np.arange(len(idx))
        obs[pos, idx] = 1
        return obs

    @staticmethod
    def convert_to_unordered_obs(cards):
        obs = np.zeros(len(CardType), dtype=np.int8)
        idx = np.array([card.type.value for card in cards], dtype=np.int8)
        np.add.at(obs, idx, 1)
        return obs

    def draw(self, n = HAND_SIZE):

        if len(self._draw_pile) < n:
            self._shuffle()
            self._draw_pile.extend(self.discard_pile)
            self._discard_pile = []

        new_hand, self._draw_pile = self._draw_pile[:n], self._draw_pile[n:]
        self._hand.extend(new_hand)

    def discard_played(self):
        self._discard_pile.extend(self._played_cards)
        self._played_cards = []

    def use(self, idx):
        self._played_cards.append(self._hand.pop(idx))
    
    def _shuffle(self):
        self.rng.shuffle(self._discard_pile)

    def add(self, card):
        self._discard_pile.append(card)
        self._all_cards.append(card)

    def remove(self, card):
        self._all_cards.remove(card)
        self._hand.remove(card)
        del card
