#include "constants.h"
#include "environment.h"
#include "sampler.h"
#include <iostream>
#include <ostream>

int main() {
  action_sampler sampler;

  uint32_t max_steps = 1e7;
  ObsData observation{};
  Info info{};
  ActionMask mask{};
  std::array<float, MAX_N_PLAYERS> rewards{};
  cog_env env;
  env.init(observation, info, rewards, mask);
  env.reset(54321, 4, 1, DEFAULT_DIFFICULTY, max_steps, false);
  std::vector<Info> finished_episodes;

  for (uint32_t i = 0; i < max_steps; i++) {
    u_char current_agent = env.get_agent_selection();
    ActionData act =
        sampler.sample(observation.player_data[current_agent].action_mask);
    env.step(act);
    if (env.get_done()) {
      finished_episodes.push_back(env.get_info());
      env.reset();
    }
  }

  for (Info i : finished_episodes) {
    std::cout << "Game finished with length " << i.total_length << std::endl;
    for (AgentInfo ag : i.agent_infos) {
      std::cout << "Player: " << ' ';
      std::cout << "return " << ag.returns << ' ';
      std::cout << ", added " << ag.cards_added << ' ';
      std::cout << ", removed " << ag.cards_added << ' ';
      std::cout << ", card uses " << ag.n_card_uses << ' ';
      std::cout << ", coins " << ag.n_coin_uses << ' ';
      std::cout << ", machetes " << ag.n_machete_uses << ' ';
      std::cout << ", paddles" << ag.n_paddle_uses << ' ';
      std::cout << ", turns " << ag.steps_taken << ' ';
      std::cout << ", moved " << ag.travelled_hexes << std::endl << std::endl;
    }
  }
}
