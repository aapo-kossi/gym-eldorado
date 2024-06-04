#pragma once

#include <vector>
#include <random>
#include <string>
#include "environment.h"

const ushort N_MAP_FEATURES = 7;
const ushort MAX_CACHED_MAPS = 128;
const ushort GRIDSIZE = 48;

enum Resources {
    MACHETE,
    PADDLE,
    COIN,
    USE,
    REMOVE,
    MAX
};

typedef struct {
    short u;
    short v;
    short w;
} cubepoint;

typedef struct {
    short x;
    short y;
} point;

typedef struct {
    union {
        struct {
            point NONE;
            point EAST;
            point NORTHEAST;
            point NORTHWEST;
            point WEST;
            point SOUTHWEST;
            point SOUTHEAST;
        };
        struct {
            short array[7][2];
        };
    };
} Dir;

const Dir DIRECTIONS = {
    .NONE =      (point) {0,0},
    .EAST =      (point) {1,0},
    .NORTHEAST = (point) {0,1},
    .NORTHWEST = (point) {-1,1},
    .WEST =      (point) {-1,0},
    .SOUTHWEST = (point) {0,-1},
    .SOUTHEAST = (point) {1,-1},
}

typedef struct {
    Resources resource,
    ushort n_required,
    bool is_end
} MovementInfo;

typedef struct {
    point coords,
    std::vector<int>* rotations // Careful with memory here!
} ConnectionInfo;

enum Color {
    RESET,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT,

    GRAY,

    BYELLOW,

    RED_BG,
    GREEN_BG,
    YELLOW_BG,
    BLUE_BG,
    MAGENTA_BG,
    CYAN_BG,
    WHITE_BG,
    MAX
};

const char* COLORCODES[Color.MAX] = {
    "\x1b[0m",
    "\x1b[30m",
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[37m",
    "\x1b[39m",

    "\x1b[2m\x1b[37m",

    "\x1b[33;1m",

    "\x1b[101;30m",
    "\x1b[102;30m",
    "\x1b[103;30m",
    "\x1b[104;30m",
    "\x1b[105;30m",
    "\x1b[106;30m",
    "\x1b[107;30m"
};

const char* player_colors[N_PLAYERS] = {
    COLORCODES[Color.RED_BG],
    COLORCODES[Color.GREEN_BG],
    COLORCODES[Color.YELLOW_BG],
    COLORCODES[Color.BLUE_BG]
}

enum Difficulty {
    EASY,
    MEDIUM,
    HARD
};

enum PieceType {
    START,
    TRAVEL,
    END
};
std::string* colored(std::string* to_color, std::string* color);

const char* RESOSURCE_STRINGS[Resources.MAX] = {
    colored('m', Color.GREEN),
    colored('p', Color.BLUE),
    colored('c', Color.YELLOW),
    colored('u', Color.GRAY),
    colored('d', Color.RED)
}

// Declarations for functions
Hex* get_n_copies(Hex hex, ushort n);
Hex* get_overlap(Hex hex, ushort n);
point cube_to_xy(cubepoint);
hexpoint xy_to_cube(point);

// Declarations for classes
class Hex {
public:
    Hex(
        Resources resource = NULL,
        int n_required = NULL,
        int is_end = 0,
        int player_start = 0,
        int occupier = 0
    ); // constructor
    ~Hex(); // destructor

    Resources resource;
    int n_required;
    int is_end;
    int player_start;
    int occupier;
    int strlen;

    bool is_passable();
    char* to_string();
};

class MapPiece {
public:
    MapPiece(
        std::vector<Hex> hexes,
        std::vector<point> hex_coords,
        point center,
        int rotation
    ); // constructor
    ~MapPiece(); // destructor

    int centerX;
    int centerY;
    int rotation;
    std::vector<point> xy_array;

    static std::vector<point> translate(
        std::vector<point> xy_array, int a, int b
    );
    static std::vector<point> rotate(
        std::vector<point> xy_array, int times
    );

    virtual ConnectionInfo connection_points(MapPiece& other);
};

class LargePiece : public MapPiece {
    ConnectionInfo connection_points(MapPiece& other) final;
};

class SmallPiece : public MapPiece {
    ConnectionInfo connection_points(MapPiece& other) final;
};

class EndPiece : public MapPiece {
    ConnectionInfo (MapPiece& other) final;
};


class EndPiece : public MapPiece {
    EndPiece(); // constructor

    virtual void connection_points(MapPiece& other);
};

class Map {
public:
    Map(std::vector<MapPiece> pieces); // constructor
    ~Map(); // destructor

    std::vector<MapPiece> pieces;
    std::vector<std::vector<Hex>> hexes;
    std::vector<int> hex_index;
    std::vector<point> player_locations;
    ushort n_players;
    std::vector<std::vector<ushort>> observation(
        ushort player_id, ushort size
    );
    std::string draw();
    std::vector<ushort> movement_mask(
        ushort player_id, Resources resources[Resources.MAX]
    );
    MovementInfo move_in_direction(
        ushort player_id, point direction
    );
    MovementInfo move_to_point(
        ushort player_id, point point
    );
    std::vector<point> add_players(
        ushort player_count
    );
    std::vector<point> get_open_connections(
        MapPiece& other
    );
    template < class RNG >
    Map add_piece(
        MapPiece new_piece,
        point new_point,
        int rotation,
        RNG &rng = std::default_random_engine()
    );

private:
    MovementInfo _move(
        point from, point to
    );
};


template < class RNG >
Map generate(
    int n_pieces,
    Difficulty difficulty,
    int failures,
    int max_failures,
    RNG rng = std::default_random_engine()
);
