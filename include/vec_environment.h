#pragma once

#include "api.h"
#include "constants.h"
#include "environment.h"

/* The classes defined here have ownership of the data structures
 * used by the game and the RL algorithms on the cpu side */

template <size_t N> class vec_cog_env {
private:
  std::array<ObsData, N> observations;
  std::array<std::array<float, MAX_N_PLAYERS>, N> rewards;
  std::array<bool, N> dones;
  std::array<u_char, N> agent_selections;
  std::array<ActionMask, N> selected_action_masks;
  std::array<Info, N> infos;
  std::array<cog_env, N> environments;
  size_t num_envs = N;
  size_t num_players;

public:
  vec_cog_env()
      : observations{}, rewards{}, dones{}, infos{}, agent_selections{},
        num_players(MAX_N_PLAYERS) {
    for (size_t i = 0; i < N; i++) {
      environments[i].init(observations[i], infos[i], rewards[i],
                           selected_action_masks[i]);
    }
  }

  void reset() {
    for (auto &env : environments) {
      env.reset();
    }
  }

  void reset(uint32_t seed, u_char n_players, u_char n_pieces,
             Difficulty difficulty, unsigned int max_steps, bool render) {
    for (size_t i = 0; i < N; i++) {
      environments[i].reset(seed + i, n_players, n_pieces, difficulty,
                            max_steps, render);
    }
  }

  void step(const std::array<ActionData, N> &actions) {
    for (size_t i = 0; i < num_envs; ++i) {
      step_single(actions[i], i);
    }
    return;
  }

  void step_single(const ActionData &act, size_t i) {
    cog_env &env = environments[i];
    env.step(act);
    dones[i] = env.get_done();
    if (dones[i]) {
      env.reset();
    }
    agent_selections[i] = env.get_agent_selection();
  }

  size_t get_num_envs() const { return N; }
  u_char get_num_players() const { return environments[0].n_players; }

  const std::array<u_char, N> &get_agent_selections() const {
    return agent_selections;
  }
  const std::array<Info, N> &get_infos() const { return infos; }
  const Info &get_info(size_t i) const { return infos[i]; }
  const std::array<std::array<float, MAX_N_PLAYERS>, N> &get_rewards() const {
    return rewards;
  }
  const std::array<bool, N> &get_dones() const { return dones; }
  const std::array<ObsData, N> &get_observations() const {
    return observations;
  }
  const std::array<ActionMask, N> &get_selected_action_masks() const {
    return selected_action_masks;
  }
};
