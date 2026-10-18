# Builds the round hay bale and writes it as a mesh blob grouped by material:
#   blender -b --factory-startup --python tools/blender/bale.py -- <out.mesh>
# The bale stands in makeBarrel's frame, so the game lays it down with the
# same matrices: its axis along y from 0 to 1, half a unit in radius.

import math
import os
import random
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from meshkit import finish, solid  # noqa: E402

MATERIALS = ["straw", "twine"]

SEGMENTS = 20
# The drum, under the twine, which reaches the frame's half unit.
DRUM = 0.488
# Each end: the coiled face out to CAP, domed DOME out from its rim, then a
# rounded shoulder down onto the drum SHOULDER along the axis.
CAP = 0.4
DOME = 0.03
SHOULDER = 0.11
SHOULDER_STEPS = 2
# The coil: TURNS turns of a ridge RIDGE proud of the grooves beside it.
TURNS = 3.5
RIDGE = 0.012
# The twine: a cord of TWINE_WIDTH round the drum at each of BANDS, standing
# out to the frame's radius from a little under the drum.
BANDS = [0.25, 0.5, 0.75]
TWINE_WIDTH = 0.035
TWINE_OUT = 0.5
TWINE_IN = DRUM - 0.006
# Loose straws: TUFTS tufts of STRAWS spikes, lying along the surface.
SEED = 11
TUFTS = 5
STRAWS = 3
STRAW_LENGTH = (0.1, 0.17)
STRAW_THICKNESS = 0.016
STRAW_LIFT = 0.035


def around(index):
    return 2 * math.pi * index / SEGMENTS


def at(radius, angle, y):
    return (radius * math.cos(angle), y, radius * math.sin(angle))


def coil_fade(radius):
    """How much of the ridge shows at `radius`: none at the middle and at the
    cap's rim, so the coil meets the centre point and the shoulder."""
    pitch = CAP / TURNS
    return max(0.0, min(1.0, radius / pitch, (CAP - radius) / pitch))


def cap_depth(radius, ridge):
    """How far in from the end the coiled face lies at `radius`: domed, the
    ridges (`ridge` 1) on the dome and the grooves (-1) sunk below it."""
    return DOME * (radius / CAP) ** 2 + (1.0 - ridge) * RIDGE * coil_fade(radius)


def end_y(depth, end):
    return depth if end == 0 else 1.0 - depth


def build_end(end, points, faces):
    """One end's coiled face. Its rows are spirals: row k at angle j lies at
    k / 2 + j / SEGMENTS turns of the coil out, so a row comes round onto the
    row two beyond it, ridges on even rows and grooves on odd. Radii are held
    between the middle and the cap's rim, where the merged points close it."""
    pitch = CAP / TURNS
    first, last = -2, int(2 * TURNS) + 2
    index = {}

    def point(row, step):
        turns = row / 2 + step / SEGMENTS
        key = (round(turns * 2 * SEGMENTS), step % SEGMENTS)
        if key not in index:
            radius = max(0.0, min(CAP, turns * pitch))
            ridge = 1.0 if row % 2 == 0 else -1.0
            index[key] = len(points)
            points.append(at(radius, around(step), end_y(cap_depth(radius, ridge),
                                                         end)))
        return index[key]

    for row in range(first, last):
        for step in range(SEGMENTS):
            faces.append([point(row, step), point(row, step + 1),
                          point(row + 1, step + 1), point(row + 1, step)])


def build_body():
    """The drum, its two rounded shoulders and the coiled ends, one closed
    solid once the coincident points merge."""
    points, faces = [], []
    build_end(0, points, faces)
    build_end(1, points, faces)

    profile = []
    for step in range(SHOULDER_STEPS + 1):
        bend = 0.5 * math.pi * step / SHOULDER_STEPS
        radius = CAP + (DRUM - CAP) * math.sin(bend)
        depth = DOME + (SHOULDER - DOME) * (1.0 - math.cos(bend))
        profile.append((radius, depth))
    rings = [(radius, end_y(depth, 0)) for radius, depth in profile]
    rings += [(radius, end_y(depth, 1)) for radius, depth in reversed(profile)]

    first = len(points)
    for radius, y in rings:
        points.extend(at(radius, around(step), y) for step in range(SEGMENTS))
    for ring in range(len(rings) - 1):
        for step in range(SEGMENTS):
            here = first + ring * SEGMENTS
            there = here + SEGMENTS
            faces.append([here + step, here + (step + 1) % SEGMENTS,
                          there + (step + 1) % SEGMENTS, there + step])

    solid("straw", points, faces, recalc=True, tidy=True)


def build_band(middle):
    """A cord round the drum at `middle` along it: a low ridge of three faces,
    its foot just under the drum's surface."""
    section = [(TWINE_IN, middle - TWINE_WIDTH / 2), (TWINE_OUT, middle),
               (TWINE_IN, middle + TWINE_WIDTH / 2)]
    points = [at(radius, around(step), y) for step in range(SEGMENTS)
              for radius, y in section]
    count = len(section)
    faces = []
    for step in range(SEGMENTS):
        here, there = step * count, ((step + 1) % SEGMENTS) * count
        for corner in range(count):
            following = (corner + 1) % count
            faces.append([here + corner, there + corner, there + following,
                          here + following])
    solid("twine", points, faces, recalc=True)


def straw(base, along, out, length):
    """A thin spike from `base`, lying along `along` and lifting off the
    surface along `out`."""
    side = normalised(cross(along, out))
    half = STRAW_THICKNESS / 2
    tip = tuple(b + a * length + o * STRAW_LIFT for b, a, o in zip(base, along, out))
    foot = [tuple(b + s * half * sign for b, s in zip(base, side)) for sign in (-1, 1)]
    top = tuple(b + o * STRAW_THICKNESS for b, o in zip(base, out))
    solid("straw", [foot[0], foot[1], top, tip],
          [[0, 1, 2], [0, 3, 1], [1, 3, 2], [2, 3, 0]], recalc=True)


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0])


def normalised(vector):
    length = math.sqrt(sum(value * value for value in vector))
    return tuple(value / length for value in vector)


def build_tufts(rng):
    """Tufts on the drum near the shoulders, straws fanned along the drum's
    length and a little round it."""
    for tuft in range(TUFTS):
        angle = 2 * math.pi * (tuft + rng.uniform(0.2, 0.8)) / TUFTS
        end = tuft % 2
        y = end_y(rng.uniform(SHOULDER + 0.02, SHOULDER + 0.12), end)
        out = (math.cos(angle), 0.0, math.sin(angle))
        round_drum = (-math.sin(angle), 0.0, math.cos(angle))
        towards_end = -1.0 if end == 0 else 1.0
        for _ in range(STRAWS):
            spread = rng.uniform(-0.6, 0.6)
            along = normalised(tuple(r * math.sin(spread)
                                     for r in round_drum))
            along = normalised((along[0], towards_end * math.cos(spread), along[2]))
            base = at(DRUM - STRAW_THICKNESS / 2, angle + rng.uniform(-0.05, 0.05),
                      y)
            straw(base, along, out, rng.uniform(*STRAW_LENGTH))


def main():
    path = sys.argv[sys.argv.index("--") + 1]
    bpy.ops.wm.read_factory_settings(use_empty=True)
    build_body()
    for middle in BANDS:
        build_band(middle)
    build_tufts(random.Random(SEED))
    finish(path, MATERIALS, "bale")


main()
