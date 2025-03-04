#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "environment.h"
#include "sampler.h"

// Test: Constructor
TEST_CASE("eldorado_env constructor initializes correctly") {

  cog_env env;
}

// Test: Reset
TEST_CASE("eldorado_env reset reinitializes the environment correctly") {
  ObsData observation{};
  ActionMask mask{};
  Info info{};
  std::array<float, MAX_N_PLAYERS> rewards{};
  cog_env env;
  env.init(observation, info, rewards, mask);

  // Resetting with identical parameters regenerates the same map
  env.reset(123, 3, 8, Difficulty::MEDIUM, 200, false);
  std::vector<MapPiece *> orig_pieces = env.get_map().get_pieces();
  env.reset(123, 3, 8, Difficulty::MEDIUM, 200, false);
  std::vector<MapPiece *> same_pieces = env.get_map().get_pieces();
  bool equal = true;
  REQUIRE_NE(orig_pieces.size(), 0);
  REQUIRE(orig_pieces.size() == same_pieces.size());
  for (size_t i = 0; i < orig_pieces.size(); i++) {
    equal = equal && (orig_pieces[i] == same_pieces[i]);
  }
  CHECK(equal);

  // Changing the seed generates a new map
  env.reset(124, 3, 8, Difficulty::MEDIUM, 200, false);
  std::vector<MapPiece *> new_pieces = env.get_map().get_pieces();
  equal = true;
  REQUIRE_NE(orig_pieces.size(), 0);
  REQUIRE(orig_pieces.size() == new_pieces.size());
  for (size_t i = 0; i < orig_pieces.size(); i++) {
    equal = equal && (orig_pieces[i] == new_pieces[i]);
  }
  int orig_seed = env.get_seed();
  ushort orig_n_players = env.get_n_players();
  ushort orig_n_pieces = env.get_n_pieces();
  Difficulty orig_difficulty = env.get_difficulty();
  unsigned int orig_max_steps = env.get_max_steps();
  bool orig_render = env.get_render();

  // Resetting with default parameters doesn't overwrite previous parameters
  env.reset();
  int seed = env.get_seed();
  ushort n_players = env.get_n_players();
  ushort n_pieces = env.get_n_pieces();
  Difficulty difficulty = env.get_difficulty();
  unsigned int max_steps = env.get_max_steps();
  bool render = env.get_render();
  CHECK(seed == orig_seed);
  CHECK(n_players == orig_n_players);
  CHECK(n_pieces == orig_n_pieces);
  CHECK(difficulty == orig_difficulty);
  CHECK(max_steps == orig_max_steps);
  CHECK(render == orig_render);
}

// Test: Fails to initialize when too few unique pieces exist
TEST_CASE(
    "eldorado_env cannot initialize with more easy pieces than are defined") {
  ObsData observation{};
  ActionMask mask{};
  Info info{};
  std::array<float, MAX_N_PLAYERS> rewards{};
  cog_env env;
  env.init(observation, info, rewards, mask);

  CHECK_NOTHROW(env.reset(124, 3, 3, Difficulty::EASY, 200, false));
  CHECK_THROWS_AS(env.reset(123, 3, 4, Difficulty::EASY, 200, false),
                  generate_map_failure);
}

// Test: Take random steps until max_steps
TEST_CASE(
    "eldorado_env step processes actions, ends game when max_steps is hit") {
  action_sampler sampler;

  ObsData observation{};
  ActionMask mask{};
  Info info{};
  std::array<float, MAX_N_PLAYERS> rewards{};
  cog_env env;
  env.init(observation, info, rewards, mask);
  env.reset(54321, 4, 5, Difficulty::MEDIUM, 100, false);

  CHECK_NOTHROW(do {
    ushort current_agent = env.get_agent_selection();
    ActionData act =
        sampler.sample(observation.player_data[current_agent].action_mask);
    env.step(act);
    /*} while (false)*/
  } while (!env.get_done()));
  CHECK(env.get_info().total_length == 100);
}

// Test: Game ends when a player begins their turn on a goal hex
TEST_CASE("eldorado_env correctly handles the game ending") {
  action_sampler sampler;

  uint32_t max_steps = 100000;
  ObsData observation{};
  ActionMask mask{};
  Info info{};
  std::array<float, MAX_N_PLAYERS> rewards{};
  cog_env env;
  env.init(observation, info, rewards, mask);
  env.reset(54321, 4, 1, DEFAULT_DIFFICULTY, max_steps, false);

  CHECK_NOTHROW(do {
    ushort current_agent = env.get_agent_selection();
    ActionData act =
        sampler.sample(observation.player_data[current_agent].action_mask);
    env.step(act);
  } while (!env.get_done()));
  REQUIRE(env.get_info().total_length < max_steps);
  float total_return = 0;
  for (AgentInfo i : env.get_info().agent_infos) {
    total_return += i.returns;
  }
  CHECK(total_return == 0.0);
  CHECK(env.get_info().agent_infos[0].returns != 0.0);
}
