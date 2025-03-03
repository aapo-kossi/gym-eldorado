#include "geometry.h"

point cube_to_xy(const cubepoint &uvw) {
  point ret{};
  ret.x = -4.0f / 3.0f * (uvw.v + 0.5f * uvw.u);
  ret.y = 4.0f / 3.0f * (uvw.u + 0.5f * uvw.v);
  return ret;
}

cubepoint xy_to_cube(const point &xy) {
  float halfx = xy.x / 2;
  float halfy = xy.y / 2;
  float u = halfx + xy.y;
  float v = -xy.x - halfy;
  float w = halfx - halfy;
  return {u, v, w};
}
