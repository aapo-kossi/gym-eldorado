#include "constants.h"
#include "pybind/common.h"
#include "pybind/vectorized.h"

PYBIND11_MODULE(city_of_gold, m) {
  m.doc() = "Python bindings for The Quest for El Dorado C++ environment";

  PYBIND11_NUMPY_DTYPE(DeckObs, draw, hand, active, played, discard);
  PYBIND11_NUMPY_DTYPE(ActionMask, play, play_special, remove, get_from_shop,
                       move);
  PYBIND11_NUMPY_DTYPE(PlayerData, obs, action_mask);
  PYBIND11_NUMPY_DTYPE(SharedObservation, map, phase, shop, current_resources);
  PYBIND11_NUMPY_DTYPE(ObsData, shared, player_data);
  PYBIND11_NUMPY_DTYPE(ActionData, play, play_special, remove, move,
                       get_from_shop);

  PYBIND11_NUMPY_DTYPE(AgentInfo, steps_taken, returns, travelled_hexes,
                       cards_added, cards_removed, n_machete_uses,
                       n_paddle_uses, n_coin_uses, n_card_uses);
  PYBIND11_NUMPY_DTYPE(Info, total_length, agent_infos);

  bind_single_env(m);
  constexpr size_t max_envs = 256;

  py::class_<action_sampler>(m, "action_sampler")
      .def(py::init<unsigned long>(), py::arg("seed"))
      .def("sample", &action_sampler::sample);

  bind_runners<max_envs>(m);

  py::enum_<Difficulty>(m, "Difficulty")
      .value("EASY", Difficulty::EASY)
      .value("MEDIUM", Difficulty::MEDIUM)
      .value("HARD", Difficulty::HARD)
      .export_values();
}
