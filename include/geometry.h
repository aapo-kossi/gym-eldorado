#pragma once

#include "constants.h"
#include <array>

typedef struct {
  float u;
  float v;
  float w;
} cubepoint;

typedef struct point {
  float x;
  float y;
  inline bool operator==(const point &) const = default;
  inline bool operator!=(const point &) const = default;
  inline bool operator<(const point &other) const {
    return (x < other.x) || (x == other.x && y < other.y);
  };
  inline point operator+(const point &other) const {
    return {x + other.x, y + other.y};
  };
  inline point operator-(const point &other) const {
    return {x - other.x, y - other.y};
  };
  inline point operator-() const { return {-x, -y}; };
} point;

point cube_to_xy(const cubepoint &);
cubepoint xy_to_cube(const point &);

enum Direction {
  NONE = 0,
  EAST,
  NORTHEAST,
  NORTHWEST,
  WEST,
  SOUTHWEST,
  SOUTHEAST
};

constexpr std::array<point, 7> DIRECTIONS = {{
    {0, 0},
    {1, 0},
    {0, 1},
    {-1, 1},
    {-1, 0},
    {0, -1},
    {1, -1},
}};
constexpr u_char N_DIRECTIONS = DIRECTIONS.size();
