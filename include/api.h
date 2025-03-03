#pragma once

#include <array>
#include <cstddef>
#include <new>
#include <stdlib.h>

#include "constants.h"
#include "geometry.h"

#ifdef _WIN32
    #ifdef ELDORADO_LIBRARY_EXPORTS
        #define ELDORADO_API __declspec(dllexport)
    #else
        #define ELDORADO_API __declspec(dllimport)
    #endif
#else
    #define ELDORADO_API __attribute__((visibility("default")))
#endif

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_constructive_interference_size;
    using std::hardware_destructive_interference_size;
#else
    constexpr std::size_t hardware_constructive_interference_size =
        2 * sizeof(std::max_align_t);
    constexpr std::size_t hardware_destructive_interference_size =
        2 * sizeof(std::max_align_t);
#endif

template <typename T,
          std::size_t Alignment = hardware_destructive_interference_size>
struct AlignedAllocator {
  using value_type = T;

  T *allocate(std::size_t n) {
    if (n == 0)
      return nullptr;

    void *ptr = nullptr;
    if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
      throw std::bad_alloc();
    }

    return static_cast<T *>(ptr);
  }

  void deallocate(T *ptr, std::size_t) noexcept { free(ptr); }

  template <typename U> struct rebind {
    using other = AlignedAllocator<U, Alignment>;
  };
};

template <typename T, typename U, std::size_t A>
bool operator==(const AlignedAllocator<T, A> &,
                const AlignedAllocator<U, A> &) {
  return true;
}

template <typename T, typename U, std::size_t A>
bool operator!=(const AlignedAllocator<T, A> &,
                const AlignedAllocator<U, A> &) {
  return false;
}

struct ELDORADO_API DeckObs {
  std::array<u_char, N_CARDTYPES> draw;
  std::array<u_char, N_CARDTYPES> hand;
  std::array<u_char, N_CARDTYPES> active;
  std::array<u_char, N_CARDTYPES> played;
  std::array<u_char, N_CARDTYPES> discard;
  /*std::array<float, N_RESOURCETYPES> resources;*/

  void reset() {
    draw.fill(0);
    hand.fill(0);
    active.fill(0);
    discard.fill(0);
    /*resources.fill(.0f);*/
  }
};

typedef std::array<std::array<std::array<u_char, N_MAP_FEATURES>, GRIDSIZE>,
                   GRIDSIZE>
    MapObservation;

struct ELDORADO_API SharedObservation {
  MapObservation map;
  u_char phase;
  std::array<float, N_RESOURCETYPES> current_resources;
  std::array<u_char, N_BUYABLETYPES> shop;
};

struct ELDORADO_API alignas(hardware_destructive_interference_size) ActionMask {
  std::array<bool, N_CARDTYPES + 1> play;
  std::array<bool, N_CARDTYPES + 1> play_special;
  std::array<bool, N_CARDTYPES + 1> remove;
  std::array<bool, N_DIRECTIONS> move;
  std::array<bool, N_BUYABLETYPES + 1> get_from_shop;
  ActionMask() { // special card actions always valid
    reset();
  };
  void reset() {
    play[0] = true; // Null card playing action always valid
    std::fill(play.begin() + 1, play.end(), false);

    remove[0] = true;
    std::fill(remove.begin() + 1, remove.end(), false);

    play_special[0] = true;
    std::fill(play_special.begin() + 1, play_special.end(), false);

    move[0] = true; // Null movement (staying still) always valid,
    // other movement mask components filled by map

    get_from_shop[0] = true; // same for adding new cards
  }
};

struct ELDORADO_API PlayerData {
  DeckObs obs;
  ActionMask action_mask;
};

struct ELDORADO_API alignas(hardware_destructive_interference_size) ObsData {
  SharedObservation shared;
  std::array<PlayerData, MAX_N_PLAYERS> player_data;
};

struct ELDORADO_API alignas(hardware_destructive_interference_size) ActionData {
  u_char play;
  u_char play_special;
  u_char remove;
  u_char move;
  u_char get_from_shop;

  bool is_null() const { return !play && !move && !get_from_shop; }
  bool operator!=(ActionData &other) const {
    return ((play != other.play) || (play_special != other.play_special) ||
            (remove != other.remove) || (move != other.move) ||
            (get_from_shop != other.get_from_shop));
  };
};

struct ELDORADO_API AgentInfo {
  u_char steps_taken;
  float returns;
  unsigned int travelled_hexes;
  u_char cards_added;
  u_char cards_removed;
  unsigned int n_machete_uses;
  unsigned int n_paddle_uses;
  unsigned int n_coin_uses;
  unsigned int n_card_uses;
};

struct ELDORADO_API alignas(hardware_destructive_interference_size) Info {
  unsigned int total_length;
  std::array<AgentInfo, MAX_N_PLAYERS> agent_infos;
};
