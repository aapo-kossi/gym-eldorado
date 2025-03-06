#pragma once

#include <array>
#include <format>
/*#include <iostream>*/
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>
#include <string_view>

#include "api.h"
#include "constants.h"
#include "pybind/common.h"
#include "runner.h"
#include "vec_environment.h"
#include "vec_sampler.h"

namespace py = pybind11;
using namespace pybind11::literals;

extern std::string_view vec_env_cls;
extern std::string_view vec_sampler_cls;
extern std::string_view vec_runner_cls;

template <size_t N> class py_vec_env {
private:
  vec_cog_env<N> env;
  constexpr const static std::array<ptrdiff_t, 3> map_strides = {
      GRIDSIZE, GRIDSIZE, N_MAP_FEATURES};
  constexpr const static std::array<ptrdiff_t, 1> shop_strides = {
      N_BUYABLETYPES};
  constexpr const static std::array<ptrdiff_t, 1> cards_strides = {N_CARDTYPES};
  constexpr const static std::array<ptrdiff_t, 1> resource_strides = {
      N_RESOURCETYPES};

  constexpr const static std::array<ptrdiff_t, 1> play_mask_strides = {
      N_CARDTYPES + 1};
  constexpr const static std::array<ptrdiff_t, 1> special_mask_strides = {2};
  constexpr const static std::array<ptrdiff_t, 1> shop_mask_strides = {
      N_BUYABLETYPES + 1};
  constexpr const static std::array<ptrdiff_t, 1> move_mask_strides = {
      N_DIRECTIONS};
  constexpr const static std::array<ptrdiff_t, 2> remove_mask_strides = {
      N_CARDTYPES, MAX_CARD_COPIES + 1};

  constexpr const static std::array<ptrdiff_t, 1> n_envs_stride = {N};
  constexpr const static std::array<ptrdiff_t, 2> n_players_stride = {
      N, MAX_N_PLAYERS};

public:
  py_vec_env() : env{} {}

  void reset() { env.reset(); }

  void reset(uint32_t seed, u_char n_players, u_char n_pieces,
             Difficulty difficulty, unsigned int max_steps, bool render) {
    env.reset(seed, n_players, n_pieces, difficulty, max_steps, render);
  }

  void step(const py::array_t<ActionData> &actions) {
    py::buffer_info buf = actions.request();

    // Unsafe but also fast :D
    const auto &action_array =
        *reinterpret_cast<std::array<ActionData, N> *>(buf.ptr);

    env.step(action_array);
  }

  const py::array_t<ObsData> observe() {
    const auto &obs_array = env.get_observations();
    return create_numpy_view(&obs_array[0], n_envs_stride);
  }

  size_t get_num_envs() const { return env.get_num_envs(); }

  py::array_t<u_char> get_agent_selection() const {
    const std::array<u_char, N> &agents = env.get_agent_selections();
    return create_numpy_view(&agents[0], n_envs_stride);
  }

  py::array_t<bool> get_dones() const {
    const std::array<bool, N> &dones = env.get_dones();
    return create_numpy_view(&dones[0], n_envs_stride);
  }

  py::array_t<float> get_rewards() const {
    const std::array<std::array<float, MAX_N_PLAYERS>, N> &rewards =
        env.get_rewards();
    return create_numpy_view(&rewards[0][0], n_players_stride);
  }

  py::array_t<Info> get_infos() const {
    const std::array<Info, N> &infos_array = env.get_infos();
    return create_numpy_view(&infos_array[0], n_envs_stride);
  }

  py::array_t<ActionMask> get_selected_action_masks() const {
    const std::array<ActionMask, N> &masks_array =
        env.get_selected_action_masks();
    return create_numpy_view(&masks_array[0], n_envs_stride);
  }

  vec_cog_env<N> &get_env() { return env; }
};

template <size_t N> class py_vec_action_sampler {
private:
  constexpr const static std::array<ptrdiff_t, 1> n_envs_stride = {N};
  vec_action_sampler<N> samplers;

public:
  py_vec_action_sampler(uint32_t seed) : samplers{seed} {}
  py::array_t<ActionData> get_actions() {
    const std::array<ActionData, N> &act_arr = samplers.get_actions();
    return create_numpy_view(&act_arr[0], n_envs_stride);
  }
  void sample(const py::array_t<ActionMask> &am) {
    py::buffer_info buf = am.request();

    // Unsafe but also fast :D
    const auto &am_array =
        *reinterpret_cast<std::array<ActionMask, N> *>(buf.ptr);
    samplers.sample(am_array);
  }
  vec_action_sampler<N> &get_sampler() { return samplers; }
};

template <size_t N> class py_threaded_runner {
private:
  py_vec_env<N> &envs;
  py_vec_action_sampler<N> &samplers;
  ThreadedRunner<N> runner;
  constexpr const static std::array<ptrdiff_t, 1> n_envs_stride = {N};

public:
  py_threaded_runner(py_vec_env<N> &env_, py_vec_action_sampler<N> &samplers_,
                     size_t n_threads)
      : runner(env_.get_env(), samplers_.get_sampler(), n_threads), envs(env_),
        samplers(samplers_) {}
  py_vec_env<N> &get_envs() { return envs; }
  size_t get_n_threads() const { return runner.get_n_threads(); }
  py_vec_action_sampler<N> &get_samplers() { return samplers; }
  py::array_t<ActionData> get_actions() const {

    const auto &actions = runner.get_actions();
    return create_numpy_view(&actions[0], n_envs_stride);
  }
  py::array_t<ActionMask> get_action_masks() const {

    const auto &am = runner.get_action_masks();
    return create_numpy_view(&am[0], n_envs_stride);
  }
  void sample() { runner.sample(); }
  void step() { runner.step(); }
  void step_sync() {
    runner.step();
    runner.sync();
  }
  void sync() { runner.sync(); }
};

template <size_t N> void bind_vec_env(py::module_ &m) {
  std::string num = std::to_string(N);
  std::string name = std::string(vec_env_cls) + num;
  std::string doc = std::format(
      R"pbdoc(
      Vectorized city of gold environment for {0} environments.

      :meth:`{1}.reset` must be called first to initialize the environments
      before stepping. This also allows setting the parameters for
      random number generation, number of players, and map generation.
      Sets default game parameters:
      * seed: std::random_device()()
      * n_players: 4
      * n_pieces: 3
      * difficulty: Difficulty.EASY
      * max_steps: 100_000
      * render: False

      :return: The (uninitialized) vectorized environments.
      )pbdoc",
      N, name);

  py::class_<py_vec_env<N>>(m, name.c_str(), doc.c_str())
      .def(py::init(), "Create the environments")
      .def("reset", py::overload_cast<>(&py_vec_env<N>::reset), R"pbdoc(
           Reset all environments, not modifying current parameters

           If no environment parameters have been previously set,
           the default parameters from when the instance was constructed
           are used.

           :return: None
      )pbdoc")
      .def("reset",
           py::overload_cast<uint32_t, u_char, u_char, Difficulty, unsigned int,
                             bool>(&py_vec_env<N>::reset))
      .def("step", &py_vec_env<N>::step, py::arg("actions"))
      .def_property_readonly("observations", &py_vec_env<N>::observe,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("num_envs", &py_vec_env<N>::get_num_envs)
      .def_property_readonly("agent_selection",
                             &py_vec_env<N>::get_agent_selection,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("selected_action_masks",
                             &py_vec_env<N>::get_selected_action_masks,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("dones", &py_vec_env<N>::get_dones,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("rewards", &py_vec_env<N>::get_rewards,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("infos", &py_vec_env<N>::get_infos,
                             py::return_value_policy::reference_internal);
}

template <size_t N> void bind_vec_sampler(py::module_ &m) {
  py::class_<py_vec_action_sampler<N>>(
      m, (std::string(vec_sampler_cls) + std::to_string(N)).c_str())
      .def(py::init([](std::optional<size_t> seed) {
             return std::make_unique<py_vec_action_sampler<N>>(
                 seed.value_or(std::random_device{}()));
           }),
           py::arg("seed") = py::none())
      /*.def(py::init<py::array_t<uint32_t>>(), py::arg("seeds"))*/
      .def("get_actions", &py_vec_action_sampler<N>::get_actions,
           py::return_value_policy::reference_internal)
      .def("sample", &py_vec_action_sampler<N>::sample, py::arg("action_mask"));
  /*.def("sample_single", &py_vec_action_sampler<N>::sample_single);*/
}

template <size_t N> void bind_runner(py::module_ &m) {
  py::class_<py_threaded_runner<N>>(
      m, (std::string(vec_runner_cls) + std::to_string(N)).c_str())
      .def(py::init([](py_vec_env<N> &env_, py_vec_action_sampler<N> &sampler_,
                       std::optional<size_t> n_threads) {
             return std::make_unique<py_threaded_runner<N>>(
                 env_, sampler_,
                 n_threads.value_or(std::thread::hardware_concurrency()));
           }),
           py::arg("env"), py::arg("sampler"),
           py::arg("n_threads") = py::none())
      .def("get_envs", &py_threaded_runner<N>::get_envs,
           py::return_value_policy::reference)
      .def("get_n_threads", &py_threaded_runner<N>::get_n_threads)
      .def("get_samplers", &py_threaded_runner<N>::get_samplers,
           py::return_value_policy::reference)
      .def("get_actions", &py_threaded_runner<N>::get_actions,
           py::return_value_policy::reference)
      .def("get_action_masks", &py_threaded_runner<N>::get_action_masks,
           py::return_value_policy::reference)
      .def("step", &py_threaded_runner<N>::step)
      .def("step_sync", &py_threaded_runner<N>::step_sync)
      .def("sync", &py_threaded_runner<N>::sync)
      .def("sample", &py_threaded_runner<N>::sample);
}

void bind_vec_getters(py::module_ &m_parent, py::module_ &m_samplers, py::module_ &m_envs, py::module_ &m_runners);

template <size_t N, size_t i=0> void bind_internal(py::module_ &m_s, py::module_ &m_e, py::module_ &m_r) {
  bind_vec_sampler<i>(m_s);
  bind_vec_env<i>(m_e);
  bind_runner<i>(m_r);
  if constexpr ( i >= N) return;
  else if constexpr ( i >= 8) return bind_internal<N,i*2>(m_s, m_e, m_r);
  else return bind_internal<N,i+1>(m_s, m_e, m_r);
}

template <size_t N> void bind_runners(py::module_ &m_parent) {
  auto m_env = m_parent.def_submodule("env", "Vectorized environments");
  auto m_sam = m_parent.def_submodule("sampler", "Vectorized action samplers");
  auto m_run = m_parent.def_submodule("runner", "Vectorized runners");
  bind_internal<N>(m_env, m_sam, m_run);
  bind_vec_getters(m_parent, m_env, m_sam, m_run);
}

