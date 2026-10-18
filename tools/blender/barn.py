# Builds the barn and writes it as a mesh blob grouped by material:
#   blender -b --factory-startup --python tools/blender/barn.py -- <out.mesh>
# Everything is written in the game's own space (x right, y up, the front
# facing -z) and turned into Blender's z-up space as it is made.

import math
import os
import random
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from meshkit import finish, solid  # noqa: E402

# Must match Lib/cowsinlove_Actors/Props/Barn.cpp.
HALF_WIDTH = 8.0
HALF_DEPTH = 11.0
WALL = 0.3
EAVES = 6.2
RIDGE = 11.5
DOOR_HALF = 2.5
DOOR_HEIGHT = 4.5
DOOR_THICKNESS = 0.15
TRIM_WIDTH = 0.25
TRIM_DEPTH = 0.06
DOOR_LEAF_WIDTH = DOOR_HALF + 0.1
BREAK_X = 4.8
BREAK_HEIGHT = 8.6
EAVES_OVERHANG = 0.4
GABLE_OVERHANG = 0.5
LOFT_HEIGHT = 5.0
LOFT_THICKNESS = 0.3
LOFT_DEPTH = 8.0
RAMP_WIDTH = 2.4
RAMP_FOOT_Z = 6.0
LOFT_POST_X = 3.0
BALE_WIDTH = 1.3
BALE_HEIGHT = 0.7
BALE_LENGTH = 2.5
CLIMB_FRONT = -10.0
CLIMB_RAMP_LENGTH = 5.0
CLIMB_RAMP_LAYERS = 3
CLIMB_LAYERS = [3, 5, 7, 8]
LOFT_STACK_LAYERS = 2
CUPOLA_Z = 7.5
CUPOLA_HALF = 0.7
CUPOLA_TOP = 13.2
HAY_DOOR_HALF = DOOR_HALF
HAY_DOOR_BOTTOM = LOFT_HEIGHT
HAY_DOOR_TOP = 8.4
HAY_LEAF_WIDTH = DOOR_HALF
STALL_ZS = [-8.5, -5.5, -2.5, 0.5]
STALL_DEPTH = 2.4
STALL_HEIGHT = 1.2
STALL_BOARD = 0.15
TROUGH_WIDTH = 0.8
TROUGH_LENGTH = 1.6
TROUGH_HEIGHT = 0.8

# The model's own details, which nothing in the game depends on.
ROOF_THICKNESS = 0.18
SEAM_HEIGHT = 0.035
SEAM_WIDTH = 0.06
SEAM_SPACING = 1.25
CORNER_BOARD = 0.35
SMALL_BEVEL = 0.02
BALE_BEVEL = 0.1
TWINE = 0.012
TWINE_WIDTH = 0.06
BATTEN_SPACING = 0.9
BATTEN_WIDTH = 0.12
BATTEN_DEPTH = 0.04
SHORTEST_BATTEN = 0.3
BEAM = 0.3
RAFTER_DEPTH = 0.22
RAFTER_WIDTH = 0.2
STRUCTURE_ZS = [-6.0, 0.0, 3.5, 7.5]
HOIST_BEAM_BOTTOM = HAY_DOOR_TOP + 0.6
WINDOW_HALF = 0.7
WINDOW_BOARD = 0.16
SIDE_WINDOWS = [-7.0, -2.5, 2.0, 6.5]
SIDE_WINDOW = (2.3, 3.8)
BACK_WINDOWS = [-5.0, 5.0]
BACK_WINDOW = (2.0, 3.5)
GABLE_WINDOW = (7.4, 8.8)
GABLE_WINDOW_HALF = 0.6
TWINE_BANDS = [0.28, 0.72]
CLIMB_BALE_SEGMENTS = 1
LOFT_BALE_SEGMENTS = 2
BALE_JITTER = 0.03
BALE_TWIST = math.radians(2.5)
STRAWS = 16
STRAW_LENGTH = 0.55
STRAW_THICKNESS = 0.025
SEED = 7
MOUND_SEGMENTS = 10
HAY_PILES = [(-6.4, -4.0, 1.0, 0.45), (-4.0, 6.5, 1.2, 0.5)]
LADDER_X = -4.6
LADDER_FOOT = 0.7
LANTERN_X = -2.5
LANTERN_Z = 3.5
FLOOR_HEIGHT = 0.03

MATERIALS = ["wall", "trim", "roof", "wood", "hay", "metal", "twine"]

INSIDE = HALF_WIDTH - WALL
BACK_INSIDE = HALF_DEPTH - WALL
LOFT_EDGE = -HALF_DEPTH + LOFT_DEPTH
FRONT = -HALF_DEPTH
ROOF_FRONT = -HALF_DEPTH - GABLE_OVERHANG
ROOF_BACK = HALF_DEPTH + GABLE_OVERHANG


def prism_z(mat, profile, z0, z1, bevel=0.0, segments=1):
    """A prism whose cross-section is `profile` (x, y), from z0 to z1."""
    count = len(profile)
    points = [(x, y, z0) for x, y in profile] + [(x, y, z1) for x, y in profile]
    faces = [list(range(count)), list(range(2 * count - 1, count - 1, -1))]
    faces += [[i, (i + 1) % count, count + (i + 1) % count, count + i]
              for i in range(count)]
    return solid(mat, points, faces, bevel, segments)


def prism_x(mat, profile, x0, x1, bevel=0.0, segments=1):
    """A prism whose cross-section is `profile` (z, y), from x0 to x1."""
    count = len(profile)
    points = [(x0, y, z) for z, y in profile] + [(x1, y, z) for z, y in profile]
    faces = [list(range(count)), list(range(2 * count - 1, count - 1, -1))]
    faces += [[i, (i + 1) % count, count + (i + 1) % count, count + i]
              for i in range(count)]
    return solid(mat, points, faces, bevel, segments)


def box(mat, x0, x1, y0, y1, z0, z1, bevel=SMALL_BEVEL, segments=1):
    return prism_z(mat, [(x0, y0), (x1, y0), (x1, y1), (x0, y1)], z0, z1,
                   bevel, segments)


# The roof's line across the barn, and lines parallel to it.

def lower_slope():
    return (BREAK_HEIGHT - EAVES) / (HALF_WIDTH - BREAK_X)


def roof_height(x):
    """The walkable top of the roof over x."""
    x = abs(x)
    if x <= BREAK_X:
        return RIDGE - x * (RIDGE - BREAK_HEIGHT) / BREAK_X
    return BREAK_HEIGHT - (x - BREAK_X) * lower_slope()


def roof_line(reach):
    """The roof's top from -reach to reach, through the breaks and ridge."""
    return [(-reach, roof_height(reach)), (-BREAK_X, BREAK_HEIGHT), (0.0, RIDGE),
            (BREAK_X, BREAK_HEIGHT), (reach, roof_height(reach))]


def normalised(vector):
    length = math.hypot(vector[0], vector[1])
    return (vector[0] / length, vector[1] / length)


def offset(line, distance):
    """`line` moved `distance` along its upward normal, mitred at the corners;
    the ends cut square to their segments."""
    normals = []
    for a, b in zip(line, line[1:]):
        dx, dy = normalised((b[0] - a[0], b[1] - a[1]))
        normals.append((-dy, dx))

    moved = []
    for index, point in enumerate(line):
        if index == 0:
            n = normals[0]
            scale = distance
        elif index == len(line) - 1:
            n = normals[-1]
            scale = distance
        else:
            before, after = normals[index - 1], normals[index]
            n = normalised((before[0] + after[0], before[1] + after[1]))
            scale = distance / (n[0] * before[0] + n[1] * before[1])
        moved.append((point[0] + n[0] * scale, point[1] + n[1] * scale))
    return moved


def vertical_line(line, distance):
    """`line` moved `distance` straight up, keeping each point's x."""
    return [(x, y + distance) for x, y in line]


def thickness_below(x):
    """How far below the roof's top its underside is, straight down at x."""
    slope = lower_slope() if abs(x) > BREAK_X else (RIDGE - BREAK_HEIGHT) / BREAK_X
    return ROOF_THICKNESS * math.hypot(1.0, slope)


def strip(mat, top, bottom, z0, z1, bevel=0.0):
    for index in range(len(top) - 1):
        prism_z(mat, [bottom[index], bottom[index + 1], top[index + 1], top[index]],
                z0, z1, bevel)


def along(a, b, t):
    return (a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t)


# The parts.

def build_roof():
    outer = roof_line(HALF_WIDTH + EAVES_OVERHANG)
    strip("roof", outer, offset(outer, -ROOF_THICKNESS), ROOF_FRONT, ROOF_BACK)

    seams = int((ROOF_BACK - ROOF_FRONT) / SEAM_SPACING)
    for seam in range(1, seams):
        z = ROOF_FRONT + seam * (ROOF_BACK - ROOF_FRONT) / seams
        strip("roof", offset(outer, SEAM_HEIGHT), outer,
              z - SEAM_WIDTH / 2, z + SEAM_WIDTH / 2)

    cap = 0.35
    cap_line = [(-cap, roof_height(cap)), (0.0, RIDGE), (cap, roof_height(cap))]
    strip("metal", offset(cap_line, 0.06), offset(cap_line, -0.05),
          ROOF_FRONT - 0.03, ROOF_BACK + 0.03)

    rake_top = offset(outer, 0.05)
    rake_bottom = offset(outer, -ROOF_THICKNESS - 0.12)
    for z0, z1 in [(ROOF_FRONT - TRIM_DEPTH, ROOF_FRONT + 0.02),
                   (ROOF_BACK - 0.02, ROOF_BACK + TRIM_DEPTH)]:
        strip("trim", rake_top, rake_bottom, z0, z1, SMALL_BEVEL)

    for side in [-1.0, 1.0]:
        edge = HALF_WIDTH + EAVES_OVERHANG
        top = roof_height(edge)
        bottom = top - thickness_below(edge) - 0.1
        x0, x1 = sorted([side * (edge - 0.03), side * (edge + TRIM_DEPTH)])
        box("trim", x0, x1, bottom, top + 0.02, ROOF_FRONT - TRIM_DEPTH,
            ROOF_BACK + TRIM_DEPTH)

    for z in STRUCTURE_ZS + [LOFT_EDGE]:
        inner = offset(roof_line(INSIDE), -ROOF_THICKNESS)
        strip("wood", inner, offset(inner, -RAFTER_DEPTH),
              z - RAFTER_WIDTH / 2, z + RAFTER_WIDTH / 2, SMALL_BEVEL)

        collar = BREAK_HEIGHT - thickness_below(BREAK_X) - RAFTER_DEPTH
        box("wood", -BREAK_X, BREAK_X, collar - BEAM * 0.8, collar,
            z - RAFTER_WIDTH / 2, z + RAFTER_WIDTH / 2)


def wall_top(x):
    """Where the walls end: halfway into the roof's thickness, out of sight."""
    return roof_height(x) - thickness_below(x) / 2


def gable_profile():
    return [(INSIDE, wall_top(INSIDE)), (BREAK_X, wall_top(BREAK_X) + 0.05),
            (0.0, wall_top(0.0) + 0.05), (-BREAK_X, wall_top(BREAK_X) + 0.05),
            (-INSIDE, wall_top(INSIDE))]


def gable_top(x):
    """The gable's top at x, along gable_profile."""
    points = sorted(gable_profile())
    x = abs(x)
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        if x0 <= x <= x1:
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0)
    return points[-1][1]


def build_walls():
    for side in [-1.0, 1.0]:
        inner, outer = side * INSIDE, side * HALF_WIDTH
        prism_z("wall", [(inner, 0.0), (outer, 0.0), (outer, wall_top(outer)),
                         (inner, wall_top(inner))], FRONT, HALF_DEPTH)

    gable = gable_profile()
    prism_z("wall", [(-INSIDE, 0.0), (INSIDE, 0.0)] + gable, BACK_INSIDE, HALF_DEPTH)
    for side in [-1.0, 1.0]:
        prism_z("wall", [(side * DOOR_HALF, 0.0), (side * INSIDE, 0.0),
                         (side * INSIDE, wall_top(INSIDE)),
                         (side * BREAK_X, gable_top(BREAK_X)),
                         (side * DOOR_HALF, gable_top(DOOR_HALF))],
                FRONT, FRONT + WALL)
    box("wall", -DOOR_HALF, DOOR_HALF, DOOR_HEIGHT, HAY_DOOR_BOTTOM, FRONT,
        FRONT + WALL, 0.0)
    prism_z("wall", [(-DOOR_HALF, HAY_DOOR_TOP), (DOOR_HALF, HAY_DOOR_TOP),
                     (DOOR_HALF, gable_top(DOOR_HALF)), (0.0, gable_top(0.0)),
                     (-DOOR_HALF, gable_top(DOOR_HALF))], FRONT, FRONT + WALL)


def open_spans(top, blocked):
    """What is left of [0, top] once the `blocked` (y0, y1) spans are cut out."""
    spans = [(0.0, top)]
    for y0, y1 in blocked:
        cut = []
        for a, b in spans:
            if y0 > a:
                cut.append((a, min(b, y0)))
            if y1 < b:
                cut.append((max(a, y1), b))
        spans = cut
    return [(a, b) for a, b in spans if b - a > SHORTEST_BATTEN]


def battens(face, side, start, end, top, holes):
    """Upright battens along a wall, `face` the axis the wall faces along and
    `side` which way. `top(u)` is how high they reach at u along the wall;
    `holes` are (u0, u1, y0, y1) they stop short of."""
    count = int((end - start) / BATTEN_SPACING)
    for index in range(count + 1):
        u = start + (end - start) * index / count
        u0, u1 = u - BATTEN_WIDTH / 2, u + BATTEN_WIDTH / 2
        blocked = [(y0, y1) for h0, h1, y0, y1 in holes if h0 < u1 and u0 < h1]
        for y0, y1 in open_spans(min(top(u0), top(u1)), blocked):
            if face == "x":
                base = side * HALF_WIDTH
                x0, x1 = sorted([base, base + side * BATTEN_DEPTH])
                box("wall", x0, x1, y0, y1, u0, u1, 0.0)
            else:
                base = side * HALF_DEPTH
                z0, z1 = sorted([base, base + side * BATTEN_DEPTH])
                box("wall", u0, u1, y0, y1, z0, z1, 0.0)


def under_roof(x):
    return roof_height(x) - thickness_below(x) - 0.02


def build_battens():
    margin = CORNER_BOARD + 0.1
    eave_top = wall_top(HALF_WIDTH) - thickness_below(HALF_WIDTH) / 2 - 0.05
    window_half = WINDOW_HALF + WINDOW_BOARD + 0.05
    for side in [-1.0, 1.0]:
        holes = []
        if side > 0:
            holes = [(z - window_half, z + window_half, 0.0, 99.0)
                     for z in SIDE_WINDOWS]
        battens("x", side, -HALF_DEPTH + margin, HALF_DEPTH - margin,
                lambda u: eave_top, holes)

    reach = HALF_WIDTH - margin
    back_holes = [(x - window_half, x + window_half, 0.0, 99.0) for x in BACK_WINDOWS]
    back_holes.append((-GABLE_WINDOW_HALF - WINDOW_BOARD - 0.05,
                       GABLE_WINDOW_HALF + WINDOW_BOARD + 0.05, GABLE_WINDOW[0] - 0.3,
                       GABLE_WINDOW[1] + 0.2))
    battens("z", 1.0, -reach, reach, under_roof, back_holes)

    door = DOOR_HALF + TRIM_WIDTH + 0.05
    hay = HAY_DOOR_HALF + TRIM_WIDTH + 0.05
    front_holes = [(-door, door, 0.0, DOOR_HEIGHT + TRIM_WIDTH + 0.02),
                   (-hay, hay, HAY_DOOR_BOTTOM - TRIM_WIDTH - 0.02,
                    HAY_DOOR_TOP + TRIM_WIDTH + 0.02)]
    battens("z", -1.0, -reach, reach, under_roof, front_holes)


def build_corner_boards():
    for sx in [-1.0, 1.0]:
        for sz in [-1.0, 1.0]:
            top = wall_top(HALF_WIDTH) - thickness_below(HALF_WIDTH) / 2 - 0.05
            face = sz * HALF_DEPTH
            x0, x1 = sorted([sx * (HALF_WIDTH - CORNER_BOARD),
                             sx * (HALF_WIDTH + TRIM_DEPTH)])
            z0, z1 = sorted([face, face + sz * TRIM_DEPTH])
            box("trim", x0, x1, 0.0, top, z0, z1)

            x0, x1 = sorted([sx * HALF_WIDTH, sx * (HALF_WIDTH + TRIM_DEPTH)])
            z0, z1 = sorted([face, face - sz * CORNER_BOARD])
            box("trim", x0, x1, 0.0, top, z0, z1)


def braced_panel(mat, x0, x1, y0, y1, back, front, braces=1):
    """A board door: a panel framed in trim with a cross brace in each of
    `braces` stacked bays. Builds forward from `back` (z) to `front`."""
    sign = 1.0 if front > back else -1.0
    depth = abs(front - back)
    z_panel = sorted([back, back + sign * depth * 0.6])
    z_frame = sorted([back + sign * depth * 0.6, front])
    z_brace = sorted([back + sign * depth * 0.6, back + sign * depth * 0.8])
    z_over = sorted([back + sign * depth * 0.6, back + sign * depth * 0.95])

    box(mat, x0, x1, y0, y1, *z_panel)
    board = min(TRIM_WIDTH * 0.8, (x1 - x0) * 0.12)
    box("trim", x0, x0 + board, y0, y1, *z_frame)
    box("trim", x1 - board, x1, y0, y1, *z_frame)

    bay = (y1 - y0 - board) / braces
    for index in range(braces + 1):
        y = y0 + index * bay
        box("trim", x0 + board, x1 - board, y, y + board, *z_frame)

    for index in range(braces):
        bx0, bx1 = x0 + board, x1 - board
        by0 = y0 + index * bay + board
        by1 = by0 + bay - board
        cross_brace(bx0, bx1, by0, by1, board * 0.8, z_brace, z_over)


def cross_brace(x0, x1, y0, y1, width, z_low, z_high):
    angle = math.atan2(y1 - y0, x1 - x0)
    a = width / math.sin(angle)
    b = width / math.cos(angle)
    rising = [(x0, y0), (x0 + a, y0), (x1, y1 - b), (x1, y1), (x1 - a, y1),
              (x0, y0 + b)]
    falling = [(x, y0 + y1 - y) for x, y in reversed(rising)]
    prism_z("trim", rising, *z_low)
    prism_z("trim", falling, *z_high)


def build_doors():
    face = FRONT - TRIM_DEPTH
    for side in [-1.0, 1.0]:
        x0, x1 = sorted([side * DOOR_HALF, side * (DOOR_HALF + TRIM_WIDTH)])
        box("trim", x0, x1, 0.0, DOOR_HEIGHT, face, FRONT)
    box("trim", -DOOR_HALF - TRIM_WIDTH, DOOR_HALF + TRIM_WIDTH, DOOR_HEIGHT,
        DOOR_HEIGHT + TRIM_WIDTH, face, FRONT)

    leaf = DOOR_LEAF_WIDTH
    for side in [-1.0, 1.0]:
        x0, x1 = sorted([side * (DOOR_HALF + TRIM_WIDTH),
                         side * (DOOR_HALF + TRIM_WIDTH + leaf)])
        braced_panel("wall", x0, x1, 0.05, DOOR_HEIGHT, face - 0.01,
                     face - DOOR_THICKNESS, 2)
        hinge_x = side * (DOOR_HALF + TRIM_WIDTH + 0.1)
        for y in [0.6, DOOR_HEIGHT / 2, DOOR_HEIGHT - 0.6]:
            hx0, hx1 = sorted([hinge_x, hinge_x + side * 0.6])
            box("metal", hx0, hx1, y - 0.07, y + 0.07,
                face - DOOR_THICKNESS - 0.04, face - DOOR_THICKNESS + 0.02, 0.0)

    box("trim", -HAY_DOOR_HALF - TRIM_WIDTH, HAY_DOOR_HALF + TRIM_WIDTH,
        HAY_DOOR_TOP, HAY_DOOR_TOP + TRIM_WIDTH, face, FRONT)
    box("trim", -HAY_DOOR_HALF - TRIM_WIDTH, HAY_DOOR_HALF + TRIM_WIDTH,
        HAY_DOOR_BOTTOM - TRIM_WIDTH, HAY_DOOR_BOTTOM, face, FRONT)
    for side in [-1.0, 1.0]:
        x0, x1 = sorted([side * HAY_DOOR_HALF, side * (HAY_DOOR_HALF + TRIM_WIDTH)])
        box("trim", x0, x1, HAY_DOOR_BOTTOM, HAY_DOOR_TOP, face, FRONT)
        hay_leaf(side, face)

    beam_bottom = HOIST_BEAM_BOTTOM
    beam_top = beam_bottom + 0.28
    box("wood", -0.14, 0.14, beam_bottom, beam_top, ROOF_FRONT - 1.1, FRONT + WALL,
        SMALL_BEVEL)
    hook_z = ROOF_FRONT - 0.9
    box("metal", -0.12, 0.12, beam_bottom - 0.35, beam_bottom,
        hook_z - 0.12, hook_z + 0.12, 0.03, 2)
    box("wood", -0.025, 0.025, HAY_DOOR_TOP - 0.4, beam_bottom - 0.35,
        hook_z - 0.025, hook_z + 0.025, 0.0)
    box("metal", -0.09, 0.09, HAY_DOOR_TOP - 0.6, HAY_DOOR_TOP - 0.4,
        hook_z - 0.03, hook_z + 0.03, 0.0)


def hay_leaf(side, face):
    """A hay door leaf swung open square to the gable beside the opening, its
    boards facing out across the front."""
    inner = side * (HAY_DOOR_HALF + TRIM_WIDTH)
    outer = inner + side * DOOR_THICKNESS
    x0, x1 = sorted([inner, outer])
    y0, y1 = HAY_DOOR_BOTTOM + 0.05, HAY_DOOR_TOP - 0.05
    z0, z1 = face - HAY_LEAF_WIDTH, face
    box("wall", x0, x1, y0, y1, z0, z1)

    proud = sorted([outer, outer + side * TRIM_DEPTH])
    board = TRIM_WIDTH * 0.8
    box("trim", *proud, y0, y1, z0, z0 + board)
    box("trim", *proud, y0, y1, z1 - board, z1)
    box("trim", *proud, y0, y0 + board, z0 + board, z1 - board)
    box("trim", *proud, y1 - board, y1, z0 + board, z1 - board)
    brace = board * 0.8
    lean = math.atan2(y1 - y0 - 2 * board, z1 - z0 - 2 * board)
    a = brace / math.sin(lean)
    b = brace / math.cos(lean)
    za, zb = z0 + board, z1 - board
    ya, yb = y0 + board, y1 - board
    prism_x("trim", [(za, ya), (za + a, ya), (zb, yb - b), (zb, yb), (zb - a, yb),
                     (za, ya + b)], *proud)
    for y in [y0 + 0.5, y1 - 0.5]:
        box("metal", *sorted([outer, outer + side * 0.04]), y - 0.07, y + 0.07,
            z1 - 0.6, z1, 0.0)


def window(face_axis, side, centre, bottom, top, half):
    """A four-pane window on a wall, `face_axis` the axis it faces along."""
    def place(u0, u1, y0, y1, d0, d1, mat, bevel=0.0):
        if face_axis == "x":
            base = side * HALF_WIDTH
            x0, x1 = sorted([base + side * d0, base + side * d1])
            box(mat, x0, x1, y0, y1, centre + u0, centre + u1, bevel)
        else:
            base = side * HALF_DEPTH
            z0, z1 = sorted([base + side * d0, base + side * d1])
            box(mat, centre + u0, centre + u1, y0, y1, z0, z1, bevel)

    board = WINDOW_BOARD
    place(-half, half, bottom, top, 0.0, 0.015, "metal", 0.0)
    place(-half - board, -half, bottom - board, top + board, 0.0, TRIM_DEPTH, "trim")
    place(half, half + board, bottom - board, top + board, 0.0, TRIM_DEPTH, "trim")
    place(-half, half, bottom - board, bottom, 0.0, TRIM_DEPTH, "trim")
    place(-half, half, top, top + board, 0.0, TRIM_DEPTH, "trim")
    place(-half, half, bottom - board - 0.06, bottom - board, 0.0, TRIM_DEPTH * 2,
          "trim")
    middle = (bottom + top) / 2
    place(-0.04, 0.04, bottom, top, 0.0, 0.04, "trim", 0.0)
    place(-half, -0.04, middle - 0.04, middle + 0.04, 0.0, 0.04, "trim", 0.0)
    place(0.04, half, middle - 0.04, middle + 0.04, 0.0, 0.04, "trim", 0.0)


def build_windows():
    for z in SIDE_WINDOWS:
        window("x", 1.0, z, *SIDE_WINDOW, WINDOW_HALF)
    for x in BACK_WINDOWS:
        window("z", 1.0, x, *BACK_WINDOW, WINDOW_HALF)
    window("z", 1.0, 0.0, *GABLE_WINDOW, GABLE_WINDOW_HALF)


def build_frame():
    for side in [-1.0, 1.0]:
        for z in STRUCTURE_ZS + [LOFT_EDGE]:
            x0, x1 = sorted([side * INSIDE, side * (INSIDE - BEAM)])
            box("wood", x0, x1, 0.0, wall_top(INSIDE) - 0.1, z - BEAM / 2,
                z + BEAM / 2)
        x0, x1 = sorted([side * INSIDE, side * (INSIDE - BEAM * 0.6)])
        box("wood", x0, x1, EAVES - 0.75, EAVES - 0.45, FRONT + WALL, BACK_INSIDE)

    loft_bottom = LOFT_HEIGHT - LOFT_THICKNESS
    box("wood", -INSIDE, INSIDE, loft_bottom, LOFT_HEIGHT, FRONT + WALL, LOFT_EDGE)
    box("wood", -INSIDE, INSIDE, loft_bottom - BEAM, loft_bottom,
        LOFT_EDGE - BEAM / 2, LOFT_EDGE + BEAM / 2)
    for x in [-LOFT_POST_X, LOFT_POST_X]:
        box("wood", x - BEAM / 2, x + BEAM / 2, 0.0, loft_bottom - BEAM,
            LOFT_EDGE - BEAM / 2, LOFT_EDGE + BEAM / 2)

    ramp_x0 = INSIDE - RAMP_WIDTH
    prism_x("wood", [(RAMP_FOOT_Z, 0.0), (LOFT_EDGE, 0.0),
                     (LOFT_EDGE, LOFT_HEIGHT)], ramp_x0, INSIDE, SMALL_BEVEL)
    cleat(ramp_x0, INSIDE, RAMP_FOOT_Z, LOFT_EDGE, LOFT_HEIGHT)

    rail_height = 0.9
    rail_start, rail_end = 2.0, ramp_run() - 0.3
    for along in [rail_start, (rail_start + rail_end) / 2, rail_end]:
        z, top = ramp_point(along)
        box("wood", ramp_x0 + 0.02, ramp_x0 + 0.14, top, top + rail_height,
            z - 0.06, z + 0.06)
    (za, ya), (zb, yb) = ramp_point(rail_start), ramp_point(rail_end)
    prism_x("wood", [(za, ya + rail_height - 0.12), (zb, yb + rail_height - 0.12),
                     (zb, yb + rail_height), (za, ya + rail_height)],
            ramp_x0 + 0.01, ramp_x0 + 0.15, SMALL_BEVEL)


def ramp_run():
    return abs(LOFT_EDGE - RAMP_FOOT_Z)


def ramp_point(along):
    """The ramp's top `along` from its foot: (z, height)."""
    toward = math.copysign(1.0, LOFT_EDGE - RAMP_FOOT_Z)
    return RAMP_FOOT_Z + toward * along, LOFT_HEIGHT * along / ramp_run()


def cleat(x0, x1, z0, z1, top):
    """Slats across a ramp from z0 (the ground) up to `top` at z1."""
    run = z1 - z0
    count = int(abs(run) / 0.8)
    for index in range(1, count):
        t0 = index / count
        t1 = t0 + 0.08 / abs(run)
        a = (z0 + run * t0, top * t0)
        b = (z0 + run * t1, top * t1)
        prism_x("wood", [a, b, (b[0], b[1] + 0.05), (a[0], a[1] + 0.05)],
                x0 + 0.1, x1 - 0.1)


def turned_box(mat, centre, size, yaw, bevel=0.0, segments=1, tilt=0.0):
    """A box `size` big about `centre`, turned `yaw` about y and tipped `tilt`
    about x; its bottom face at centre.y - size.y / 2 before tipping."""
    cx, cy, cz = centre
    sx, sy, sz = (value / 2 for value in size)
    corners = [(x, y, z) for x in (-sx, sx) for y in (-sy, sy) for z in (-sz, sz)]
    points = []
    for x, y, z in corners:
        y, z = (y * math.cos(tilt) - z * math.sin(tilt),
                y * math.sin(tilt) + z * math.cos(tilt))
        x, z = (x * math.cos(yaw) + z * math.sin(yaw),
                -x * math.sin(yaw) + z * math.cos(yaw))
        points.append((cx + x, cy + y, cz + z))
    faces = [[0, 1, 3, 2], [4, 6, 7, 5], [0, 4, 5, 1], [2, 3, 7, 6], [0, 2, 6, 4],
             [1, 5, 7, 3]]
    return solid(mat, points, faces, bevel, segments)


def bale_at(centre, width, length, height, yaw, rng, segments):
    """A bale about `centre` (its top at centre.y + height / 2), its length
    along z turned by `yaw`, a little smaller and a little more turned than
    asked so a stack looks stacked by hand."""
    width -= rng.uniform(0.0, 2 * BALE_JITTER)
    deep = length - rng.uniform(0.0, 2 * BALE_JITTER)
    yaw += rng.uniform(-BALE_TWIST, BALE_TWIST)
    turned_box("hay", centre, (width, height, deep), yaw, BALE_BEVEL, segments)
    for band in TWINE_BANDS:
        along = (band - 0.5) * deep
        z = along * math.cos(yaw)
        x = along * math.sin(yaw)
        turned_box("twine", (centre[0] + x, centre[1] + TWINE / 2, centre[2] + z),
                   (width + 2 * TWINE, height + TWINE, TWINE_WIDTH), yaw)


def bale(x0, y0, z0, rng, segments):
    """A bale filling the cell from (x0, y0, z0), its length along z."""
    centre = (x0 + BALE_WIDTH / 2, y0 + BALE_HEIGHT / 2, z0 + BALE_LENGTH / 2)
    bale_at(centre, BALE_WIDTH, BALE_LENGTH, BALE_HEIGHT, 0.0, rng, segments)


def stack(x0, z0, base, layers, rng, segments, across=2):
    for layer in range(layers):
        for column in range(across):
            bale(x0 + column * BALE_WIDTH, base + layer * BALE_HEIGHT, z0, rng,
                 segments)


def cross_layer(x0, z0, y0, rng):
    """Two bales across the layer under them, length along x, filling the same
    two-wide, one-long footprint."""
    for row in range(2):
        centre = (x0 + BALE_WIDTH, y0 + BALE_HEIGHT / 2,
                  z0 + BALE_LENGTH * (row + 0.5) / 2)
        bale_at(centre, BALE_LENGTH / 2, 2 * BALE_WIDTH - 2 * BALE_JITTER,
                BALE_HEIGHT, math.pi / 2, rng, LOFT_BALE_SEGMENTS)


def straws(rng, x0, x1, z0, z1, floor):
    for _ in range(STRAWS):
        centre = (rng.uniform(x0, x1), floor + STRAW_THICKNESS / 2,
                  rng.uniform(z0, z1))
        turned_box("hay", centre, (STRAW_THICKNESS, STRAW_THICKNESS, STRAW_LENGTH),
                   rng.uniform(0.0, math.pi))


def build_climb():
    climb_x0 = -HALF_WIDTH - 2 * BALE_WIDTH
    ramp_top = CLIMB_RAMP_LAYERS * BALE_HEIGHT
    ramp_back = CLIMB_FRONT + CLIMB_RAMP_LENGTH
    prism_x("wood", [(CLIMB_FRONT, 0.0), (ramp_back, 0.0), (ramp_back, ramp_top)],
            climb_x0, -HALF_WIDTH, SMALL_BEVEL)
    cleat(climb_x0, -HALF_WIDTH, CLIMB_FRONT, ramp_back, ramp_top)

    rng = random.Random(SEED)
    for index, layers in enumerate(CLIMB_LAYERS):
        stack(climb_x0, ramp_back + index * BALE_LENGTH, 0.0, layers, rng,
              CLIMB_BALE_SEGMENTS)

    loft_front = FRONT + WALL
    stack(-INSIDE, loft_front, LOFT_HEIGHT, LOFT_STACK_LAYERS - 1, rng,
          LOFT_BALE_SEGMENTS)
    cross_layer(-INSIDE, loft_front,
                LOFT_HEIGHT + (LOFT_STACK_LAYERS - 1) * BALE_HEIGHT, rng)
    stack(INSIDE - BALE_WIDTH, loft_front, LOFT_HEIGHT, 1, rng, LOFT_BALE_SEGMENTS,
          1)
    straws(rng, -INSIDE + 0.5, INSIDE - 0.5, loft_front + 0.4, LOFT_EDGE - 0.3,
           LOFT_HEIGHT)
    straws(rng, -HALF_WIDTH - 3.5, -HALF_WIDTH - 0.3, CLIMB_FRONT - 1.0,
           ramp_back + len(CLIMB_LAYERS) * BALE_LENGTH + 1.0, 0.0)


def lathe(mat, centre, profile, segments):
    """A surface of revolution about the upright through `centre`, `profile`
    (radius, height) running from the top on the axis down to the ground."""
    cx, cy, cz = centre
    points = []
    rings = []
    for radius, height in profile:
        ring = []
        steps = 1 if radius == 0.0 else segments
        for step in range(steps):
            angle = 2.0 * math.pi * step / steps
            ring.append(len(points))
            points.append((cx + radius * math.cos(angle), cy + height,
                           cz + radius * math.sin(angle)))
        rings.append(ring)

    faces = []
    for upper, lower in zip(rings, rings[1:]):
        for step in range(segments):
            after = (step + 1) % segments
            if len(upper) == 1:
                faces.append([upper[0], lower[step], lower[after]])
            else:
                faces.append([upper[step], lower[step], lower[after], upper[after]])
    faces.append(list(reversed(rings[-1])))
    return solid(mat, points, faces)


def build_floor():
    """Boards over the ground inside, in the twine's darker brown."""
    box("twine", -INSIDE, INSIDE, 0.0, FLOOR_HEIGHT, FRONT + WALL, BACK_INSIDE, 0.0)
    box("twine", -DOOR_HALF, DOOR_HALF, 0.0, FLOOR_HEIGHT, FRONT, FRONT + WALL, 0.0)


def build_stalls():
    for z in STALL_ZS:
        x0, x1 = -INSIDE, -INSIDE + STALL_DEPTH
        box("wood", x1 - BEAM, x1, 0.0, STALL_HEIGHT + 0.15, z - BEAM / 2, z + BEAM / 2)
        for rail in range(3):
            y = 0.25 + rail * (STALL_HEIGHT - 0.35) / 2
            box("wood", x0, x1 - BEAM, y, y + 0.22, z - STALL_BOARD / 2,
                z + STALL_BOARD / 2)
        box("wood", x0, x1 - BEAM, STALL_HEIGHT - 0.1, STALL_HEIGHT,
            z - STALL_BOARD / 2 - 0.03, z + STALL_BOARD / 2 + 0.03)

    middle = (STALL_ZS[0] + STALL_ZS[1]) / 2
    z0, z1 = middle - TROUGH_LENGTH / 2, middle + TROUGH_LENGTH / 2
    x0, x1 = -INSIDE, -INSIDE + TROUGH_WIDTH
    side = 0.08
    box("wood", x0, x1, TROUGH_HEIGHT - 0.35, TROUGH_HEIGHT - 0.27, z0, z1)
    box("wood", x0, x0 + side, TROUGH_HEIGHT - 0.35, TROUGH_HEIGHT, z0, z1)
    box("wood", x1 - side, x1, TROUGH_HEIGHT - 0.35, TROUGH_HEIGHT, z0, z1)
    for z in [z0, z1 - side]:
        box("wood", x0 + side, x1 - side, TROUGH_HEIGHT - 0.35, TROUGH_HEIGHT, z,
            z + side)
    for z in [z0 + 0.1, z1 - 0.2]:
        box("wood", x0 + 0.1, x0 + 0.2, 0.0, TROUGH_HEIGHT - 0.35, z, z + 0.1, 0.0)
        box("wood", x1 - 0.2, x1 - 0.1, 0.0, TROUGH_HEIGHT - 0.35, z, z + 0.1, 0.0)
    box("hay", x0 + side, x1 - side, TROUGH_HEIGHT - 0.27, TROUGH_HEIGHT - 0.06,
        z0 + side, z1 - side, 0.0)


def build_hay_piles(rng):
    for x, z, radius, height in HAY_PILES:
        lathe("hay", (x, 0.0, z),
              [(0.0, height), (0.45 * radius, 0.85 * height),
               (0.8 * radius, 0.5 * height), (radius, 0.1 * height),
               (1.05 * radius, 0.0)], MOUND_SEGMENTS)
    straws(rng, -INSIDE + 0.5, INSIDE - 3.0, FRONT + 1.0, BACK_INSIDE - 1.0, 0.0)


def build_ladder():
    foot_z = LOFT_EDGE + LADDER_FOOT
    top = LOFT_HEIGHT + 0.9
    half = 0.3
    rail = 0.07
    for side in [-1.0, 1.0]:
        x = LADDER_X + side * half
        prism_x("wood", [(foot_z - rail, 0.0), (foot_z + rail, 0.0),
                         (LOFT_EDGE + BEAM / 2 + rail, top),
                         (LOFT_EDGE + BEAM / 2 - rail, top)],
                x - rail / 2, x + rail / 2)
    rungs = 7
    for rung in range(1, rungs):
        t = rung / rungs
        y = top * t
        z = foot_z + (LOFT_EDGE + BEAM / 2 - foot_z) * t
        box("wood", LADDER_X - half, LADDER_X + half, y - 0.03, y + 0.03,
            z - 0.03, z + 0.03, 0.0)


def build_lantern():
    collar = BREAK_HEIGHT - thickness_below(BREAK_X) - RAFTER_DEPTH - BEAM * 0.8
    lamp_top = collar - 1.4
    box("metal", LANTERN_X - 0.012, LANTERN_X + 0.012, lamp_top, collar, LANTERN_Z - 0.012,
        LANTERN_Z + 0.012, 0.0)
    box("metal", LANTERN_X - 0.16, LANTERN_X + 0.16, lamp_top - 0.06, lamp_top, LANTERN_Z - 0.16,
        LANTERN_Z + 0.16, 0.02)
    box("hay", LANTERN_X - 0.12, LANTERN_X + 0.12, lamp_top - 0.36, lamp_top - 0.06, LANTERN_Z - 0.12,
        LANTERN_Z + 0.12, 0.0)
    box("metal", LANTERN_X - 0.15, LANTERN_X + 0.15, lamp_top - 0.42, lamp_top - 0.36, LANTERN_Z - 0.15,
        LANTERN_Z + 0.15, 0.02)


def build_cupola():
    half = CUPOLA_HALF
    z0, z1 = CUPOLA_Z - half, CUPOLA_Z + half
    walls_top = CUPOLA_TOP - 0.75
    base = [(-half, wall_top(half)), (0.0, wall_top(0.0)), (half, wall_top(half)),
            (half, walls_top), (-half, walls_top)]
    prism_z("wall", base, z0, z1)

    for sx in [-1.0, 1.0]:
        for sz in [-1.0, 1.0]:
            x0, x1 = sorted([sx * (half - 0.12), sx * (half + 0.03)])
            zz0, zz1 = sorted([CUPOLA_Z + sz * (half - 0.12),
                               CUPOLA_Z + sz * (half + 0.03)])
            box("trim", x0, x1, roof_height(half), walls_top, zz0, zz1)

    louvre_bottom = roof_height(half) + 0.15
    louvre_top = walls_top - 0.12
    count = 4
    for slat in range(count):
        y = louvre_bottom + (louvre_top - louvre_bottom) * (slat + 0.5) / count
        for sx in [-1.0, 1.0]:
            x0, x1 = sorted([sx * half, sx * (half + 0.025)])
            box("trim", x0, x1, y - 0.04, y + 0.04, z0 + 0.12, z1 - 0.12, 0.0)
        for sz in [-1.0, 1.0]:
            zz0, zz1 = sorted([CUPOLA_Z + sz * half, CUPOLA_Z + sz * (half + 0.025)])
            box("trim", -half + 0.12, half - 0.12, y - 0.04, y + 0.04, zz0, zz1, 0.0)

    eave = half + 0.2
    roof_base = walls_top
    apex = CUPOLA_TOP
    points = [(-eave, roof_base, z0 - 0.2), (eave, roof_base, z0 - 0.2),
              (eave, roof_base, z1 + 0.2), (-eave, roof_base, z1 + 0.2),
              (-eave, roof_base + 0.1, z0 - 0.2), (eave, roof_base + 0.1, z0 - 0.2),
              (eave, roof_base + 0.1, z1 + 0.2), (-eave, roof_base + 0.1, z1 + 0.2),
              (0.0, apex, CUPOLA_Z)]
    faces = [[0, 1, 2, 3], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7],
             [4, 5, 8], [5, 6, 8], [6, 7, 8], [7, 4, 8]]
    solid("roof", points, faces)

    pole_top = apex + 1.0
    box("metal", -0.03, 0.03, apex - 0.1, pole_top, CUPOLA_Z - 0.03,
        CUPOLA_Z + 0.03, 0.0)
    arm = 0.45
    arm_y = apex + 0.45
    box("metal", -arm, arm, arm_y - 0.02, arm_y + 0.02, CUPOLA_Z - 0.02,
        CUPOLA_Z + 0.02, 0.0)
    box("metal", -0.02, 0.02, arm_y - 0.02, arm_y + 0.02, CUPOLA_Z - arm,
        CUPOLA_Z + arm, 0.0)
    arrow_y = pole_top - 0.15
    box("metal", -0.02, 0.02, arrow_y - 0.025, arrow_y + 0.025, CUPOLA_Z - 0.7,
        CUPOLA_Z + 0.5, 0.0)
    prism_x("metal", [(CUPOLA_Z - 0.95, arrow_y), (CUPOLA_Z - 0.7, arrow_y + 0.14),
                      (CUPOLA_Z - 0.7, arrow_y - 0.14)], -0.02, 0.02)
    prism_x("metal", [(CUPOLA_Z + 0.35, arrow_y), (CUPOLA_Z + 0.7, arrow_y + 0.22),
                      (CUPOLA_Z + 0.7, arrow_y - 0.06)], -0.02, 0.02)


def main():
    path = sys.argv[sys.argv.index("--") + 1]
    bpy.ops.wm.read_factory_settings(use_empty=True)
    build_walls()
    build_corner_boards()
    build_battens()
    build_doors()
    build_windows()
    build_frame()
    build_roof()
    build_climb()
    build_floor()
    build_stalls()
    build_hay_piles(random.Random(SEED + 1))
    build_ladder()
    build_lantern()
    build_cupola()
    finish(path, MATERIALS, "barn")


main()
