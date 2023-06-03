from dataclasses import dataclass, asdict
import re
from enum import Enum, IntEnum, auto
import functools
import numpy as np

N_MAP_FEATURES = 7
MAX_CACHED_MAPS = 128

class Resource(IntEnum):
    MACHETE = 0
    PADDLE  = 1
    COIN    = 2
    USE     = 3
    REMOVE  = 4


class Direction(Enum):
    NONE      = (0,0)
    EAST      = (1,0)
    NORTHEAST = (0,1)
    NORTHWEST = (-1,1)
    WEST      = (-1,0)
    SOUTHWEST = (0,-1)
    SOUTHEAST = (1,-1)

    @classmethod
    def array(cls):
        return np.array(list(map(lambda c: c.value, cls)))

class Color(str, Enum):
    RESET   = "\x1b[0m"
    BLACK   = "\x1b[30m"
    RED     = "\x1b[31m"
    GREEN   = "\x1b[32m"
    YELLOW  = "\x1b[33m"
    BLUE    = "\x1b[34m"
    MAGENTA = "\x1b[35m"
    CYAN    = "\x1b[36m"
    WHITE   = "\x1b[37m"
    DEFAULT = "\x1b[39m"
    
    GRAY    = "\x1b[2m\x1b[37m"

    BYELLOW   = YELLOW[:-1] + ";1m"

    RED_BG       = "\x1b[101;30m"
    GREEN_BG     = "\x1b[102;30m"
    YELLOW_BG    = "\x1b[103;30m"
    BLUE_BG      = "\x1b[104;30m"
    MAGENTA_BG   = "\x1b[105;30m"
    CYAN_BG      = "\x1b[106;30m"
    WHITE_BG     = "\x1b[107;30m"

player_colors = {
    1: Color.RED_BG,
    2: Color.GREEN_BG,
    3: Color.YELLOW_BG,
    4: Color.BLUE_BG,
}

Difficulty = IntEnum('Difficulty', [
    "EASY",
    "MEDIUM",
    "HARD",
])

PieceType = IntEnum('PieceType', [
    "START",
    "TRAVEL",
    "END",
])


def get_n_copies(hex, n):
    return [Hex(**asdict(hex)) for _ in range(n)]

def get_overlap(xy1, xy2):
    xy1 = set([tuple(xy) for xy in xy1])
    xy2 = set([tuple(xy) for xy in xy2])
    return len(xy1.intersection(xy2)) > 0

def cube_to_xy(u,v,w):
    x = -4/3*(v + u/2)
    y =  4/3*(u + v/2)
    return x,y

def xy_to_cube(x,y):
    halfx = x/2
    halfy = y/2
    u = halfx + y
    v = -x - halfy
    w = halfx - halfy
    return u,v,w

def colored(s, col):
    return col + s + Color.RESET

RESOURCE_STRINGS = {
    Resource.MACHETE: colored('m', Color.GREEN),
    Resource.COIN: colored('c', Color.YELLOW),
    Resource.PADDLE: colored('p', Color.BLUE),
    Resource.USE: colored('u', Color.GRAY),
    Resource.REMOVE: colored('d', Color.RED),
}


@dataclass
class Hex:
    resource: Resource = None
    n_required: int = None
    is_end: int = 0
    player_start: int = 0
    _occupier: int = 0

    @property
    def occupier(self):
        return self._occupier

    @occupier.setter
    def occupier(self, n):
        self._occupier = n

    @property
    def is_passable(self):
        return self.resource is not None and not self.occupier

    def __str__(self):
        player_string = colored(f'{self.occupier}', player_colors[self.occupier]) + " " if self.occupier else "  "
        if self.player_start:
            s = f"S{self.player_start}"
        else:
            s = ""
            info_color = Color.YELLOW_BG if self.is_end else Color.RESET
            s += colored(
                str(self.n_required) +
                RESOURCE_STRINGS[self.resource],
                info_color
            )
        out = s + player_string
        return out
    
    @classmethod
    def strlen(cls):
        return 4
        

@dataclass
class MapPiece:
    centerX: int
    centerY: int
    rotation: int

    def get_x_byidx(self, n):
        return self._x[n]
    
    def get_y_byidx(self, n):
        return self._y[n]

    @property
    def hex_coords(self):
        return self._x, self._y
    
    @hex_coords.setter
    def hex_coords(self, xy):
        #You better be sure you are only giving this function integers as inputs >:(
        self._x = xy[0].astype(int)
        self._y = xy[1].astype(int)

    @staticmethod
    def translate(x, y, a, b):
        return x + a, y + b

    # tail recursive piece rotation in increments of 60 degrees
    @staticmethod
    def rotate(x, y, times):
        
        def reduce(arg, mod):
            rem = arg % mod
            large = rem > (mod // 2)
            return rem - int(large)*mod

        def cube_rotate(u,v,w, times):
            dir = np.sign(times)

            if times == 1:
                return -v,-w,-u
            elif times == -1:
                return -w,-u,-v
            else:
                return cube_rotate(*cube_rotate(u,v,w, dir), times - dir)
            
        times = reduce(times, 6)

        if times == 0:
            return x, y
        else:
            # transform to cube coordinates
            u,v,w = xy_to_cube(x,y)

            #rotate
            U,V,W = cube_rotate(u,v,w, times)

            # transform back
            X,Y = cube_to_xy(U,V,W)

            return X,Y
        
    def connection_points(self, other):
        raise NotImplementedError


class MountainHex(Hex):

    def __init__(self):
        super().__init__()

    @Hex.occupier.setter
    def occupier(self, n):
        raise ValueError("Cannot move player onto an impassable mountain hex!")
    
    def __str__(self):
        return "^Ʌ^ "

class StartHex(Hex):
    def __init__(self, n):
        super().__init__(player_start = n)

@dataclass
class EndHex(Hex):
    is_end: int = 1

    @Hex.occupier.setter
    def occupier(self, n):
        return


#large map piece with 37 hexes
class LargePiece(MapPiece):

    starts = np.array([0,-1,-2,-3,-3,-3,-3])
    stops = np.array([3,3,3,3,2,1,0])
    rel_x = np.concatenate([np.arange(a,b+1) for a,b in zip(starts, stops)])
    rel_y = np.repeat(np.arange(-3,4), [4,5,6,7,6,5,4])
    del starts
    del stops

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        x, y = self.rotate(self.rel_x, self.rel_y, self.rotation)
        self.hex_coords = x + self.centerX, y + self.centerY

    def connection_points(self, other):
        if issubclass(other, LargePiece):
            xs = [np.array([4,3])]
            ys = [np.array([3,4])]
            for _ in range(1,6):
                new_x, new_y = MapPiece.rotate(xs[-1], ys[-1], 1)
                xs.append(new_x)
                ys.append(new_y)
            x, y = [x for l in xs for x in l], [y for l in ys for y in l]
            x = np.array(x) + self.centerX
            y = np.array(y) + self.centerY
            rotations = [range(6)] * len(x)
            return x, y, rotations
        
        elif issubclass(other, SmallPiece):
            xs = [np.array([1.5,2.5,3.5])]
            ys = [np.array([3.5,2.5,1.5])]
            current_rotations = np.array([[-1,2],[-1,2],[-1,2]])
            all_rotations = current_rotations
            for _ in range(1,6):
                new_x, new_y = MapPiece.rotate(xs[-1], ys[-1], 1)
                current_rotations = current_rotations + 1
                xs.append(new_x)
                ys.append(new_y)
                all_rotations = np.concatenate([all_rotations, current_rotations], axis=0)
            x, y = [x for l in xs for x in l], [y for l in ys for y in l]
            x = np.array(x) + self.centerX
            y = np.array(y) + self.centerY
            return x, y, all_rotations

        elif issubclass(other, EndPiece):
            x = [0]
            y = [4]
            rotation = [-3]
            for _ in range(1,6):
                new_x, new_y = MapPiece.rotate(x[-1], y[-1], 1)
                x.append(new_x)
                y.append(new_y)
                rotation.append(rotation[-1] + 1)
            x = np.array(x) + self.centerX
            y = np.array(y) + self.centerY
            
            return x, y, rotation
            
# medium sized map piece with 16 hexes
class SmallPiece(MapPiece):

    starts = np.array([-1.5,-2.5,-2.5])
    stops = np.array([2.5,2.5,1.5])
    rel_x = np.concatenate([np.arange(a,b+1) for a,b in zip(starts, stops)])
    rel_y = np.repeat(np.arange(-1,2), [5,6,5])
    del starts
    del stops

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        x, y = self.rotate(self.rel_x, self.rel_y, self.rotation)
        self.hex_coords = x + self.centerX, y + self.centerY

    def connection_points(self, other):
        rotations = [self.rotation, self.rotation + 3]
        if issubclass(other, LargePiece):
            x_base = np.array([-3.5,-2.5,-1.5])
            y_base = np.array([5,5,5])

            x = np.empty((len(rotations), len(x_base)))
            y = np.empty((len(rotations), len(x_base)))
            for n, rot in enumerate(rotations):
                new_x, new_y = MapPiece.rotate(x_base, y_base, rot)
                x[n] = new_x
                y[n] = new_y
            
            x = np.reshape(x, [-1])
            y = np.reshape(y, [-1])
            x = x + self.centerX
            y = y + self.centerY
            rotations = [range(6)] * len(x)

            return x, y, rotations
        
        elif issubclass(other, SmallPiece):
            raise ValueError("cannot use two small pieces in a row!")

        elif issubclass(other, EndPiece):
            raise ValueError("The ending piece must connect to a large piece!")

class StartPiece(LargePiece):

    def connection_points(self, other):
        if issubclass(other, EndPiece):
            raise ValueError("The map must have at least one travel piece!")
        x, y, rots = super().connection_points(other)
        n_per_side = len(x) // 6
        return x[:n_per_side], y[:n_per_side], rots[:n_per_side]

class Apiece1(StartPiece):

    type = PieceType.START
    difficulty = Difficulty.EASY

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hexes = [
            StartHex(1),
            StartHex(2),
            StartHex(3),
            StartHex(4),
        ] + get_n_copies(Hex(Resource.MACHETE, 1), 7) + [
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
            MountainHex(),
            Hex(Resource.COIN, 1),
        ] + get_n_copies(Hex(Resource.MACHETE, 1), 3) + [
            Hex(Resource.PADDLE, 1),
            MountainHex(),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.REMOVE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
        ]

class Cpiece1(LargePiece):

    type = PieceType.TRAVEL
    difficulty = Difficulty.EASY
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hexes = [
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            MountainHex(),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.USE, 1),
        ]

class Gpiece2(LargePiece):

    type = PieceType.TRAVEL
    difficulty = Difficulty.HARD
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hexes = get_n_copies(Hex(Resource.MACHETE, 1),3) + [
            Hex(Resource.USE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.PADDLE, 1),
            MountainHex(),
            Hex(Resource.MACHETE, 2),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.COIN, 1),
            MountainHex(),
            MountainHex(),
            Hex(Resource.USE, 3),
            Hex(Resource.PADDLE, 1),
            Hex(Resource.PADDLE, 1),
            MountainHex(),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 3),
            MountainHex(),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 2),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 2),
            Hex(Resource.COIN, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.REMOVE, 1),
        ]

class Opiece(SmallPiece):
    
    type = PieceType.TRAVEL
    difficulty = Difficulty.MEDIUM
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hexes = [
            Hex(Resource.USE, 2),
            Hex(Resource.MACHETE, 2),
            Hex(Resource.USE, 1),
            Hex(Resource.COIN, 1),
            Hex(Resource.COIN, 2),
            Hex(Resource.USE, 1),
            MountainHex(),
            MountainHex(),
            Hex(Resource.PADDLE, 4),
            MountainHex(),
            Hex(Resource.COIN, 1),
            Hex(Resource.USE, 1),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.MACHETE, 2),
            Hex(Resource.MACHETE, 1),
            Hex(Resource.COIN, 1),
        ]

class EndPiece(MapPiece):

    type = PieceType.END
    difficulty = Difficulty.EASY
    rel_x = np.array([0, 1, -1])
    rel_y = np.array([0, 0, 1])

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        x, y = self.rotate(self.rel_x, self.rel_y, self.rotation)
        self.hex_coords = x + self.centerX, y + self.centerY
        self.hexes = get_n_copies(EndHex(Resource.PADDLE, 1), 3)

    # when a player arrives at an end hex, the piece moves into el dorado, not blocking following players.
    def set_occupier(self, n):
        pass

    # the map cannot continue after an end piece is placed
    def connection_points(self, other):
        return None

class EndPiece2(EndPiece):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hexes = get_n_copies(EndHex(Resource.MACHETE, 1), 3)


class Map:

    all_pieces = [
        Apiece1,
        Cpiece1,
        Gpiece2,
        Opiece,
        EndPiece,
        EndPiece2,
    ]

    def __init__(self, pieces):
        self.pieces = pieces
        self._hexes = [hex for piece in self.pieces for hex in piece.hexes]

        xy = np.zeros((2,0))
        idx = xy
        hex_arr = np.empty((0,0))
        coords = [piece.hex_coords for piece in self.pieces]
        if len(coords):
            xs, ys = tuple(zip(*coords))
            xy = np.stack([np.concatenate(a, 0) for a in [xs,ys]],0)
            mins = np.amin(xy, 1).astype(int)
            dims = np.amax(xy, 1).astype(int) - mins + 1
            idx = xy - mins[:,np.newaxis]
            hex_arr = np.empty(dims + 1, object)
            hex_arr[tuple(idx.astype(int))] = self._hexes

        self._hex_array = hex_arr
        self._hex_coords =  xy
        self._hex_idx = idx
        self._player_locations = []

    @classmethod
    def generate(cls, n_pieces = 5, difficulty = Difficulty.EASY, failures=0, max_failures=5, rng=np.random.default_rng()):

        if failures >= max_failures: raise ValueError("failed to generate a new map within specified attempts.")

        def try_add(map, piece):
            try:
                return map.add_piece(piece, rng=rng)
            except ValueError as e:
                print(e)
                return None

        starting_pieces = list(filter(lambda p: p.type == PieceType.START, cls.all_pieces))
        start = rng.choice(starting_pieces)(centerX = 0, centerY = 0, rotation = 0)
        map = cls([start])
        for n in range(n_pieces):
            _cond = lambda p: p.difficulty <= difficulty and p.type == PieceType.TRAVEL
            if issubclass(type(map.pieces[-1]), SmallPiece) or (n == n_pieces-1):
                cond = lambda p: _cond(p) and not issubclass(p, SmallPiece)
            else: cond = _cond
            pool = list(filter(cond, cls.all_pieces))
            map = try_add(map, rng.choice(pool))
            if map is None:
                return cls.generate(n_pieces, difficulty, failures=failures+1, max_failures=max_failures)

        end_cond = lambda p: p.type == PieceType.END
        pool = list(filter(end_cond, cls.all_pieces))
        map  = try_add(map, rng.choice(pool))
        if map is None:
            return cls.generate(n_pieces, difficulty, failures=failures+1, max_failures=max_failures)
        return map
        
    def add_piece(self, piece_cls, x=None, y=None, rotation=None, rng=np.random.default_rng()):

        if x is None and y is None and rotation is None:
            valid_locs = self.get_open_connections(piece_cls)
            if not valid_locs:
                raise ValueError("No valid locations on the map for the specified piece!")
            n = len(valid_locs)
            i = rng.choice(n-1)
            x, y, rot = valid_locs[i]
            if hasattr(rot, '__len__'):
                rot = rng.choice(rot)
            new_piece = piece_cls(x, y, rot)
            return Map(self.pieces + [new_piece])
        elif x is not None and y is not None and rotation is not None:
            new_piece = piece_cls(x, y, rotation)
            return Map(self.pieces + [new_piece])
        else:
            raise ValueError("If one of x, y, rotation is specified, they must all be specified.")


    def get_open_connections(self, piece_cls):
        used_xy = Map(self.pieces[:-1]).hex_coords
        used_xy = np.transpose(used_xy)
        blocks = np.array([[1,0],[0,1],[-1,1],[-1,0],[0,-1],[1,-1]])
        blocked_xy = np.tile(used_xy, [len(blocks),1]) + np.repeat(blocks, len(used_xy), axis=0)

        candidate_locs = self.pieces[-1].connection_points(piece_cls)
        locs = []
        for center_x, center_y, rot in zip(*candidate_locs):
            if isinstance(rot, int):
                sample_rot = rot
            else:
                sample_rot = rot[0]
            rel_x, rel_y = MapPiece.rotate(piece_cls.rel_x, piece_cls.rel_y, sample_rot)
            x = center_x + rel_x
            y = center_y + rel_y
            xy = list(zip(x, y))
            if not get_overlap(blocked_xy, xy):
                locs.append((center_x, center_y, rot))
        return locs

    def add_players(self, n_players):
        assert n_players <= 4, "Maximum number of players in a game is 4."
        self.n_players = n_players
        start_hexes = self.hexes[:n_players] # map building always starts from the starting piece,
                                     # and the starting pieces always start with the starting hexes

        locs = self.hex_index[:,:n_players]

        for h in start_hexes:
            h.occupier = h.player_start

        self._player_locations = [tuple(locs[:,i]) for i in range(n_players)]
        return self._player_locations

    def _move(self, original_loc, target_loc):
        occupier = self.hex_array[original_loc].occupier
        if occupier:
            self.hex_array[target_loc].occupier = occupier
            self.hex_array[original_loc].occupier = 0
            player_id = occupier - 1
            self._player_locations[player_id] = target_loc
            target_hex = self.hex_array[target_loc]
            return target_hex.resource, target_hex.n_required, target_hex.is_end
        else:
            raise ValueError(f"No player located at {original_loc}, cannot move!")

    def move_to_coords(self, player, xy):
        original_loc = self._player_locations[player.id]
        return self._move(original_loc, xy)

    def move_in_direction(self, player, direction):
        original_loc = self._player_locations[player.id]
        new_loc = tuple(np.array(original_loc) + direction)
        return self._move(original_loc, new_loc)

    def movement_mask(self, player_id, resources):
        def mask_hex(hex):
            if hex is not None and hex.resource is not None:
                required_resource = hex.resource
                num_required = hex.n_required
                num_available = resources[required_resource]
                return int(num_available >= num_required and not hex.occupier)
            return 0

        location = self.player_locations[player_id]
        directions = Direction.array()
        adjacents = [self.hex_array[tuple(dir + location)] for dir in directions]
        return np.array([mask_hex(h) for h in adjacents], dtype=np.int8)

    def draw(self):
        xy = self.hex_coords
        hexes = self.hexes
        reprs = [str(h) for h in hexes]
        maxs = np.amax(xy, 1)
        mins = np.amin(xy,1)
        dims = maxs-mins + 1
        chars = np.full(dims, " " * Hex.strlen(), dtype=object)
        idx = xy - mins[:,np.newaxis]
        chars[tuple(idx)] = reprs

        rows = [''.join(x.tolist()) for x in chars]
        m = len(rows)
        rows = [(" "*n*(Hex.strlen() // 2) + row).rstrip() for n, row in enumerate(rows)]
        raw_rows = [re.sub('(\x9B|\x1B\[)[0-?]*[ -\/]*[@-~]', '', row) for row in rows]
        row_lengths = [len(row) for row in raw_rows]
        max_length = max(row_lengths)
        rows = [row + " "*(max_length - rowlen) for row, rowlen in zip(rows, row_lengths)]
        s = '\n'.join(rows)

        return s

    @property
    def hex_coords(self):
        return self._hex_coords
    
    @property
    def hexes(self):
        return self._hexes
    
    @property
    def hex_array(self):
        return self._hex_array

    @property
    def hex_index(self):
        return self._hex_idx
    
    @property
    def player_locations(self):
        return self._player_locations
    
    def observation(self, player_id, sz):
        obs = self._obs_base(sz)

        player_idx = tuple(zip(*self.player_locations))
        relative_player_ids = ((np.arange(len(self.player_locations)) - player_id) % self.n_players) + 1
        obs[player_idx + (0,)] = relative_player_ids

        return obs

    @functools.lru_cache(maxsize=MAX_CACHED_MAPS)
    def _obs_base(self, sz):

        def get_features(hex):
            features = np.zeros(N_MAP_FEATURES)
            if hex.resource is not None:
                resource_idx = hex.resource + 1
                features[resource_idx] = hex.n_required
                features[-1] = hex.is_end
            return features

        base = np.zeros(sz + (N_MAP_FEATURES,), dtype=np.int64)
        features = list(map(get_features, self.hexes))
        base[tuple(self.hex_index)] = features
        return base
