import functools
from eldorado_environment.env import map
from eldorado_environment.env import game
import numpy as np
import gymnasium as gym
from pettingzoo import AECEnv
from pettingzoo.utils import agent_selector, wrappers
from gymnasium.vector.utils import batch_space
from gymnasium.spaces import MultiDiscrete, MultiBinary, Discrete, Box, Dict


def eldorado_env(**kwargs):
    env = raw_eldoradoenv(**kwargs)
    # env = wrappers.AssertOutOfBoundsWrapper(env)
    env = wrappers.OrderEnforcingWrapper(env)
    return env


class raw_eldoradoenv(AECEnv):

    metadata = {"render_modes": ["human", "ANSI"], "render_fps": 1, "name": "eldorado"}


    # define observation spaces
    # hex attributes: occupier[relative to self], n_machete, n_coin, n_paddle, n_use, n_destroy, is_end
    hex_space = MultiDiscrete([5,6,6,6,4,4,2])
    grid_space = batch_space(
        batch_space(
            hex_space,
            game.N_X
        ),
        game.N_Y
    )

    # phase information: is the player in the inactive (turn just started) phase or the  movement phase (if neither, then player is buying)
    phase_space = Discrete(2)

    # the shop information includes 1. how many of each card type are available and 2. which card types are on the market board
    shop_space = batch_space(MultiDiscrete([game.N_SHOPSTACK + 1, 2]), game.N_BUYABLETYPES)

    # hand information: which cards are in hand (unplayed)
    hand_space = batch_space(MultiBinary(game.N_CARDTYPES), game.MAX_N_HAND)

    # resource information: left-over resources accumulated during the turn (maximum of 5 per resource)
    resource_space = Box(0,5, shape = (game.N_RESOURCETYPES,))

    deck_space = batch_space(Discrete(5), game.N_CARDTYPES)

    _observation_space = Dict({
        "observation": Dict({
            "grid": grid_space,
            "phase": phase_space,
            "shop": shop_space,
            "hand": hand_space,
            "played": hand_space,
            "resources": resource_space,
            "deck": deck_space,
            "discard": deck_space
        }),
        "action_mask": Dict({
            "play": MultiBinary(game.MAX_N_HAND + 1),
            "play_special": MultiBinary(2),
            "remove": batch_space(Discrete(3),game.MAX_N_HAND),
            "move": MultiBinary(len(map.Direction)),
            "get_from_shop": MultiBinary(game.N_BUYABLETYPES + 1),
        })
    })
    
    # define action space:
    # which cards to play,
    # which cards to use special action for,
    # which cards to remove,
    # which direction to move in,
    # which card to acquire (through the transmitter or buying)
    _action_space = Dict({
        "play": Discrete(game.MAX_N_HAND + 1),
        "play_special": Discrete(2),
        "remove": MultiBinary(game.MAX_N_HAND),
        "move": Discrete(len(map.Direction)),
        "get_from_shop": Discrete(game.N_BUYABLETYPES + 1)
    })

    reward_range = (-3,3) # end of game reward for agent: n_players*is_winner(agent) - n_winners


    def __init__(self, n_players=4, n_pieces = 6, difficulty = map.Difficulty.HARD, render_mode=None):
        super().__init__(
            # self.observation_space,
            # self.action_space
        )
        self.n_players = n_players
        self.n_pieces = n_pieces
        self.difficulty = difficulty

        self.possible_agents = [f"player_{n}" for n in range(n_players)]
        self.agent_name_mapping = dict(
            zip(self.possible_agents, list(range(len(self.possible_agents))))
        )

        self.render_mode = render_mode

    @functools.lru_cache(maxsize=None)
    def observation_space(self, agent):
        return self._observation_space
    
    @functools.lru_cache(maxsize=None)
    def action_space(self, agent):
        return self._action_space
    
    def render(self):
        if self.render_mode is not None:
            if not self.game.done:
                player = self.game.selected_player
                print("Current map:\n")
                print(self.game.map.draw())
                print(f"currently playing: {player.name}")
                print(player.describe_cards())
                print(player.describe_resources())
            else:
                print("game over")
        else:
            gym.logger.warn(
                "You are calling render method without specifying any render mode."
            )
        return

    def observe(self, agent):
        return self.game.get_obs(agent)

    def reset(self, seed=None, options=None):

        rng = np.random.default_rng(seed)

        self.agents = self.possible_agents[:]
        self.game = game.Game(self.agents, self.n_pieces, self.difficulty, rng=rng)


        self.rewards = {ag: 0 for ag in self.agents}
        self._cumulative_rewards = {agent: 0 for agent in self.agents}
        self.terminations = {agent: False for agent in self.agents}
        self.truncations = {agent: False for agent in self.agents}
        self.infos = {agent: {} for agent in self.agents}
        self.last_player = None

        self.agent_selection = self.agents[0]

        return 

    def step(self, action):
        agent = self.game.selected_player.agent
        self.agent_selection = agent
        if (
            self.terminations[agent]
            or self.truncations[agent]
        ):
            self._was_dead_step(action)
            return
        
        self.agent_selection, done = self.game.step(action)
        if done:
            self.rewards = self.game.get_rewards()
            self.truncations = {
                agent: True for agent in self.agents
            }
            self.infos = self._get_infos_done(agent)
        return    

    # returns the complete dictionary of infos for each agent in the game as a dictionary keyed by the agent
    # to conform to pettingzoo api. Infos are returned only once per episode, after the final step has finished.
    def _get_infos_done(self):
        info_dict = {
            "episode":
                {
                    "total_length": self.game.turn_counter,
            }
        }
        stats_dict = {}
        for agent in self.agents:
            n_agent = self.agent_name_mapping[agent]
            player = self.game.players[n_agent]
            stats_dict[agent] = {
                "turns_taken": self.game.turn_counts[player.id],
                "returns": self.rewards[n_agent],
                "travelled_hexes": player.num_movements,
                "cards_added": player.num_added_cards,
                "cards_removed": player.num_removed_cards,
                "n_machete_uses": player.num_machetes_spent,
                "n_paddle_uses": player.num_paddles_spent,
                "n_coin_uses": player.num_coins_spent,
                "n_card_uses": player.num_cards_spent,
            }

        info_dict["player_stats"] = stats_dict

        return {agent: info_dict for agent in self.agents}

