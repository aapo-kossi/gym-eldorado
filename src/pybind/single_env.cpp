#include "api.h"
#include "constants.h"
#include "environment.h"
#include "geometry.h"
#include "pybind/common.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_single_env(py::module_ &m) {
  py::class_<eldorado_env>(m, "eldorado_env")
      .def(py::init<>())
      .def(py::init<unsigned long, u_char, u_char, Difficulty, unsigned int,
                    bool>())
      .def_property_readonly("agent_selection",
                             &eldorado_env::get_agent_selection)
      .def("init", &eldorado_env::init, py::arg("observations"),
           py::arg("info"), py::arg("rewards"),
           py::arg("selected_action_masks"))
      .def("reset", (void(eldorado_env::*)()) & eldorado_env::reset)
      .def("reset", (void(eldorado_env::*)(unsigned long, u_char, u_char,
                                           Difficulty, unsigned int, bool)) &
                        eldorado_env::reset)
      .def("step", &eldorado_env::step)
      .def("render", &eldorado_env::render)
      .def("get_map", &eldorado_env::get_map,
           py::return_value_policy::reference)
      .def("get_seed", &eldorado_env::get_seed)
      .def("get_n_players", &eldorado_env::get_n_players)
      .def("get_done", &eldorado_env::get_done);

  py::class_<ObsData>(m, "ObsData")
      .def_readonly("shared", &ObsData::shared,
                    py::return_value_policy::reference_internal)
      .def_readonly("player_data", &ObsData::player_data,
                    py::return_value_policy::reference_internal);

  py::class_<SharedObservation> py_shared_observation(m, "SharedObservation");
  py_shared_observation.def_readonly(
      "phase", &SharedObservation::phase,
      py::return_value_policy::reference_internal);
  bind_array3<SharedObservation, u_char, GRIDSIZE, GRIDSIZE, N_MAP_FEATURES>(
      py_shared_observation, "map", &SharedObservation::map);
  bind_array<SharedObservation, u_char, N_BUYABLETYPES>(
      py_shared_observation, "shop", &SharedObservation::shop);
  bind_array<SharedObservation, float, N_RESOURCETYPES>(
      py_shared_observation, "resources",
      &SharedObservation::current_resources);

  py::class_<PlayerData>(m, "PlayerData")
      .def_readonly("action_mask", &PlayerData::action_mask,
                    py::return_value_policy::reference_internal)
      .def_readonly("obs", &PlayerData::obs,
                    py::return_value_policy::reference_internal);

  py::class_<ActionMask> py_action_mask(m, "ActionMask");
  bind_array<ActionMask, bool, N_CARDTYPES + 1>(py_action_mask, "play",
                                                &ActionMask::play);
  bind_array<ActionMask, bool, N_CARDTYPES + 1>(py_action_mask, "play_special",
                                                &ActionMask::play_special);
  bind_array<ActionMask, bool, N_CARDTYPES + 1>(py_action_mask, "remove",
                                                &ActionMask::remove);
  bind_array<ActionMask, bool, N_DIRECTIONS>(py_action_mask, "move",
                                             &ActionMask::move);
  bind_array<ActionMask, bool, N_BUYABLETYPES + 1>(
      py_action_mask, "get_from_shop", &ActionMask::get_from_shop);

  py::class_<DeckObs> py_player_observation(m, "PlayerObservation");
  bind_array<DeckObs, u_char, N_CARDTYPES>(py_player_observation, "draw",
                                           &DeckObs::draw);
  bind_array<DeckObs, u_char, N_CARDTYPES>(py_player_observation, "hand",
                                           &DeckObs::hand);
  bind_array<DeckObs, u_char, N_CARDTYPES>(py_player_observation, "played",
                                           &DeckObs::played);
  bind_array<DeckObs, u_char, N_CARDTYPES>(py_player_observation, "discard",
                                           &DeckObs::discard);

  py::class_<ActionData> py_action_data(m, "ActionData");
  py_action_data.def_readwrite("play", &ActionData::play)
      .def_readwrite("play_special", &ActionData::play_special)
      .def_readwrite("move", &ActionData::move)
      .def_readwrite("get_from_shop", &ActionData::get_from_shop)
      .def_readwrite("remove", &ActionData::remove);
}
