cmake_minimum_required(VERSION 3.20)
execute_process(COMMAND python ${CMAKE_SOURCE_DIR}/sync_version.py)
project(EldoradoEnv VERSION "0.0.1" LANGUAGES CXX)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_PROGRAM_PATH: ${CMAKE_PROGRAM_PATH}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -flto=auto -pg -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto=auto -march=native -mtune=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_PROFILE "-O2 -flto=auto -march=native -mtune=native -g -DNDEBUG")

option(ENABLE_TESTS "Enable building and running tests" ON)
option(ENABLE_PYTHON_BINDINGS "Build Python bindings" ON)

include_directories(include)

# The main library
add_library(eldorado_env
    src/environment.cpp
    src/map.cpp
    src/player.cpp
    src/cards.cpp
    src/geometry.cpp
)

target_include_directories(eldorado_env PRIVATE include)

if (MSVC)
    target_compile_options(eldorado_env PRIVATE /W4)
else()
    target_compile_options(eldorado_env PRIVATE -Wall -Wextra -Wshadow -Wconversion -pedantic)
endif()

if(ENABLE_TESTS)
    find_package(doctest REQUIRED)

    add_executable(test_environment src/tests/test_environment.cpp)
    target_link_libraries(test_environment PRIVATE eldorado_env doctest::doctest)

    include(CTest)
    enable_testing()
    add_test(NAME EnvironmentUnitTests COMMAND test_environment)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(eldorado_env PRIVATE -fsanitize=address -fsanitize=undefined -fdiagnostics-color=always)
    target_link_libraries(eldorado_env PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(eldorado_env PRIVATE -fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE MATCHES Profile)
    add_executable(profile_environment src/tests/profile.cpp)
    target_link_libraries(profile_environment PRIVATE eldorado_env)
endif()


if(ENABLE_PYTHON_BINDINGS)

    project(
      ${SKBUILD_PROJECT_NAME}
      VERSION ${SKBUILD_PROJECT_VERSION}
      LANGUAGES CXX
    )

    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 REQUIRED)

    add_library(bindings_ MODULE src/pybind/common.cpp src/pybind/single_env.cpp src/pybind/vectorized.cpp)

    if(CMAKE_BUILD_TYPE MATCHES Profile)
        target_link_libraries(bindings_ PRIVATE profiler tcmalloc)
    endif()

    target_link_libraries(bindings_ PRIVATE eldorado_env pybind11::module)
    target_include_directories(bindings_ PRIVATE external)
    set_target_properties(bindings_ PROPERTIES PREFIX "")
    set_target_properties(bindings_ PROPERTIES SUFFIX ".so")
    set_target_properties(bindings_ PROPERTIES OUTPUT_NAME "eldorado_py")
    if(MSVC)
        set_target_properties(bindings_ PROPERTIES SUFFIX ".pyd")
    endif()

    install(TARGETS bindings_ LIBRARY DESTINATION .)

endif()

