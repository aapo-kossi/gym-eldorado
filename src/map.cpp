#include "map.h"
#include <algorithm>

std::vector<Hex*> get_n_copies(
    Hex &hex, int n
) {
    std::vector<Hex*> ret(n);
    fill(ret.begin(), ret.end(), &hex);
    return ret;
};

std::vector<point> get_overlap(
    std::vector<point> p1,
    std::vector<point> p2
) {
    std::vector<point> intersect;
    std::sort(p1.begin(), p1.end());
    std::sort(p2.begin(), p2.end());
    std::set_intersection(
        p1.begin(), p1.end(),
        p2.begin(), p2.end(),
        back_inserter(intersect)
    );
    return intersect;
}

point cube_to_xy(cubepoint uvw) {
    point ret{};
    ret.x = -4/3*(uvw.v + uvw.u/2);
    ret.y =  4/3*(uvw.u + uvw.v/2);
    return ret;
}

cubepoint point_to_cube(point xy) {
    float halfx = static_cast< float >(xy.x)/2;
    float halfy = static_cast< float >(xy.y)/2;
    float u = halfx + xy.y;
    float v = -xy.x - halfy;
    float w = halfx - halfy;
    return cubepoint{u, v, w};
}

std::string colored(
    std::string to_color
    std::string color
) {
    return color + to_color + Color.RESET
}

Hex::Hex(
    Resources resource,
    int n_required,
    bool is_end,
    ushort player_start,
    ushort occupier
)
    : resource{resource}
    , n_required{n_required}
    , is_end{is_end}
    , player_start{player_start}
    , occupier{occupier}
{}

Hex::is_passable() {
    return (occupier == 0) & (resource != NULL);
}

Hex::to_string() {

}