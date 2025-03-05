cmake_minimum_required(VERSION 3.20)
execute_process(COMMAND python ${CMAKE_SOURCE_DIR}/sync_version.py)
project(CityofGold VERSION "0.0.1" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Handle multi-configuration generators
if (DEFINED CMAKE_CONFIGURATION_TYPES)
    if (DEFINED ENV{CIBUILDWHEEL})
        set(CMAKE_CONFIGURATION_TYPES "Release")
    else()
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile;Optimized")
    endif()
endif()

if (CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    set(CMAKE_DEFAULT_BUILD_TYPE "Release")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--param=destructive-interference-size=64)
    endif()
endif()

# compiler flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /GL /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
    set(CMAKE_CXX_FLAGS_PROFILE "/O2 /GL /Zi")
    set(CMAKE_CXX_FLAGS_OPTIMIZED "/O2 /GL /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -flto=auto -pg -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto=auto -DNDEBUG")
    set(CMAKE_CXX_FLAGS_PROFILE "-O2 -flto=auto -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_OPTIMIZED "-Ofast -flto=auto -march=native -mtune=native -DNDEBUG")
endif()

option(ENABLE_TESTS "Enable building and running tests" ON)
option(ENABLE_PYTHON_BINDINGS "Build Python bindings" ON)

if (DEFINED ENV{CIBUILDWHEEL})
    set(ENABLE_TESTS OFF)
endif()

include_directories(include)

# The main library
add_library(city_of_gold_env
    src/environment.cpp
    src/map.cpp
    src/player.cpp
    src/cards.cpp
    src/geometry.cpp
)

target_include_directories(city_of_gold_env PRIVATE include)

# Tests
if(ENABLE_TESTS)
    find_package(doctest REQUIRED)

    add_executable(test_environment src/tests/test_environment.cpp)
    target_link_libraries(test_environment PRIVATE city_of_gold_env doctest::doctest)

    include(CTest)
    enable_testing()
    add_test(NAME EnvironmentUnitTests COMMAND test_environment)
endif()

if(MSVC)
    target_compile_options(city_of_gold_env PRIVATE /W4)
    target_compile_options(city_of_gold_env PRIVATE
        "$<$<CONFIG:Debug>:
        /fsanitize=address
        >"
    )
    target_link_libraries(city_of_gold_env PRIVATE
        "$<$<CONFIG:Debug>:
        /fsanitize=address
        >"
    )
else()
    target_compile_options(city_of_gold_env PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -pedantic
    )
    target_compile_options(city_of_gold_env PRIVATE
        "$<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined;-fdiagnostics-color=always>"
    )
    target_link_libraries(city_of_gold_env PRIVATE
        "$<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>"
    )
    target_link_options(city_of_gold_env PRIVATE
        "$<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>"
    )
endif()

add_executable(profile_environment src/tests/profile.cpp)
target_link_libraries(profile_environment PRIVATE city_of_gold_env)

if(ENABLE_PYTHON_BINDINGS)

    project(
      ${SKBUILD_PROJECT_NAME}
      VERSION ${SKBUILD_PROJECT_VERSION}
      LANGUAGES CXX
    )

    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    find_package(pybind11 REQUIRED)

    add_library(bindings_ MODULE src/pybind/common.cpp src/pybind/single_env.cpp src/pybind/vectorized.cpp)

    # link gperftools deps conditionally
    target_link_libraries(bindings_ PRIVATE "$<$<CONFIG:Profile>:profiler;tcmalloc>")

    target_link_libraries(bindings_ PRIVATE city_of_gold_env pybind11::module)
    target_include_directories(bindings_ PRIVATE external)
    set_target_properties(bindings_ PROPERTIES PREFIX "" OUTPUT_NAME "city_of_gold")
    set_target_properties(bindings_ PROPERTIES SUFFIX "${CMAKE_SHARED_MODULE_SUFFIX}")

    install(TARGETS bindings_ LIBRARY DESTINATION .)

endif()

