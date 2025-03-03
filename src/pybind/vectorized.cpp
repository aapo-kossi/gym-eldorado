#include "pybind/vectorized.h"
#include <string_view>

std::string_view vec_env_cls = "vec_eldorado_env_";
std::string_view vec_sampler_cls = "vec_sampler";
std::string_view vec_runner_cls = "vec_runner";

template <> void bind_runners<0>(py::module_ &m) {
  m.def("get_runner", [m](size_t N) -> py::object {
    std::string name = std::string(vec_runner_cls) + std::to_string(N);
    return m.attr(name.c_str());
  });
  m.def("get_vec_env", [m](size_t N) -> py::object {
    std::string name = std::string(vec_env_cls) + std::to_string(N);
    return m.attr(name.c_str());
  });
  m.def("get_vec_sampler", [m](size_t N) -> py::object {
    std::string name = std::string(vec_sampler_cls) + std::to_string(N);
    return m.attr(name.c_str());
  });
};
