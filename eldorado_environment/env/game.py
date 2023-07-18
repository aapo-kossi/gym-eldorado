import numpy as np
from itertools import cycle
from eldorado_environment.env.map import Difficulty, Direction, Map, Resource
from eldorado_environment.env.player import Player, TurnPhase
from eldorado_environment.env import cards


N_X = 48
N_Y = 48

N_TURN_PHASES = len(TurnPhase)

N_STARTINGDECK = 8
N_STARTINGTYPES = 3
N_RESOURCETYPES = len(Resource)

MIN_N_HAND = 4
MAX_N_HAND = 12

N_CARDTYPES = 21
N_BUYABLETYPES = cards.Shop.n_types
N_SHOPSTACK = cards.Shop.cards_per_type

MAX_N_DECK = N_STARTINGDECK + N_BUYABLETYPES * 3

TURN_CYCLE = {
    TurnPhase.INACTIVE: TurnPhase.MOVEMENT,
    TurnPhase.MOVEMENT: TurnPhase.BUYING,
    TurnPhase.BUYING: TurnPhase.INACTIVE,
    TurnPhase.DEAD: TurnPhase.DEAD,
}

class Game:

    max_n_hand = MAX_N_HAND
    n_resourcetypes = N_RESOURCETYPES

    def __init__(
        self,
        agents,
        n_pieces,
        difficulty = "EASY",
        mapsize = (N_X,N_Y),
        rng = np.random.default_rng()
    ):
        self.agents = agents
        self.n_pieces = n_pieces
        self.difficulty = Difficulty[difficulty.upper()]
        self.grid_size = mapsize

        self.n_players = len(agents)
        self.map = Map.generate(n_pieces, difficulty=self.difficulty, rng=rng)
        self.players = [Player(agent, id, rng=rng) for id, agent in enumerate(agents)]
        self.map.add_players(self.n_players)
        self.shop = cards.Shop()
        self.done = False
        self.overrides = {}
        self.turn_counter = 0
        self.turn_counts = [0 for _ in self.players]

        self.player_selector = cycle(self.players)
        self.selected_player = next(self.player_selector)

        self._idx_direction_map = {
            n: dir for n, dir in enumerate(Direction.array())
        }

    def get_obs(self, agent):
        idx = self.agents.index(agent)
        player = self.players[idx]
        obs = {}
        obs['grid'] = self.map.observation(idx, self.grid_size)
        obs['phase'] = player.phase_observation()
        obs['shop'] = self.shop.observation()
        obs['hand'] = player.deck.hand_observation(MAX_N_HAND)
        obs['played'] = player.deck.played_observation(MAX_N_HAND)
        obs['deck'] = player.deck.all_cards_observation()
        obs['discard'] = player.deck.discarded_observation()
        obs['resources'] = player.resource_observation()

        mask = {}
        mask["play"] = player.deck.hand_mask(MAX_N_HAND + 1)
        mask["play_special"] = np.array([1,1], dtype=np.int8)
        mask["remove"] = player.deck.hand_removable_mask(MAX_N_HAND)
        mask["move"] = self.map.movement_mask(player.id, player.resources)
        mask["get_from_shop"] = self.shop.available_mask(player.resources[Resource.COIN])

        if "action_mask" in self.overrides:
            for k, v in self.overrides["action_mask"].items():
                mask[k] = v

        return {"observation": obs, "action_mask": mask}

    
    def step(self, action):

        if "step" in self.overrides:
            self.overrides["step"](action, self)
            self.overrides = {}
            self._maybe_cycle_player()
            return self.selected_player.agent, False

        loc = self.map.player_locations[self.selected_player.id]
        self.done = self.map.hex_array[loc].is_end and self.selected_player.turn_phase == TurnPhase.INACTIVE
        if self.done:
            return self.selected_player.agent, self.done


        #if start of turn, cycle to movement phase
        phase = self._maybe_cycle_phase()

        if phase == TurnPhase.MOVEMENT:

            target_direction = self._idx_direction_map[action["move"]]
            if np.any(target_direction != Direction.NONE.value):
                
                # movement
                resource, n_required, fin = self.map.move_in_direction(self.selected_player, target_direction)
                self.selected_player.num_spent[resource] += n_required
                if resource == Resource.REMOVE:
                    self.selected_player.remove_cards(n_required)
                self.selected_player.resources[resource] -= n_required
                self.selected_player.has_won = fin
                self.selected_player.num_movements += 1
            else:

                # tagging for removal
                remove_cards = action["remove"]
                self.selected_player.tag_for_removal(remove_cards)

                self._maybe_play_card(action)

        elif phase == TurnPhase.BUYING:
            desired_card = action["get_from_shop"]
            if desired_card:
                cost = self.shop.costs[desired_card - 1]
                self.selected_player.deck.add(self.shop.buy(desired_card - 1))
                self.selected_player.resources[Resource.COIN] -= cost
                self.selected_player.num_spent[Resource.COIN] += cost
                self.selected_player.num_added_cards += 1
            else:
                self._maybe_play_card(action)

        self._maybe_cycle_phase(action)
        self._maybe_cycle_player()
        return self.selected_player.agent, self.done

    def _maybe_cycle_phase(self, action=None):
        should_cycle = False
        player = self.selected_player
        phase = player.turn_phase
        if phase == TurnPhase.INACTIVE:
            should_cycle = True
        elif (action is not None) and not action["play"]:
            buying_phase = player.turn_phase == TurnPhase.BUYING
            moving_phase_still = (phase == TurnPhase.MOVEMENT) and (not action["move"])
            if buying_phase or moving_phase_still:
                should_cycle = True
        if should_cycle:
            player.turn_phase = TURN_CYCLE[phase]
        return player.turn_phase
    
    def _maybe_cycle_player(self):
        player = self.selected_player
        if player.has_won or player.turn_phase == TurnPhase.INACTIVE:
            player.deck.discard_played()
            cards_remaining = len(player.deck.hand)
            n_draw = MIN_N_HAND - cards_remaining
            if n_draw > 0:
                player.deck.draw(n_draw)
            player.reset_resources()
            self.selected_player = next(self.player_selector)
            self.turn_counter += 1

    def _maybe_play_card(self, action):
        played_card = action["play"]
        playing_something = bool(played_card)
        if playing_something:
            special_action, get_overrides = self.selected_player.play_card(played_card - 1, action["play_special"])
            self.overrides = get_overrides(self)

            # handle immediate special interactions like drawing cards.
            if special_action is not None:
                special_action(action, self)

    def get_rewards(self):
        agents = [p.agent for p in self.players]
        winners = [int(p.has_won) for p in self.players]
        n_winners = sum(winners)
        n_winners = n_winners + 1 if not n_winners else n_winners
        rewards = [self.n_players*w - n_winners for w in winners]
        return {a: r for a,r in zip(agents, rewards)}
