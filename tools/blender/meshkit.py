import math
import struct

import bmesh
import bpy


def to_blender(point):
    x, y, z = point
    return (x, -z, y)


def material(name):
    found = bpy.data.materials.get(name)
    return found if found else bpy.data.materials.new(name)


def solid(mat, points, faces, bevel=0.0, segments=1, recalc=True, tidy=False):
    """Without `recalc`, faces must already wind counter-clockwise seen from outside."""
    mesh = bpy.data.meshes.new(mat)
    built = bmesh.new()
    verts = [built.verts.new(to_blender(point)) for point in points]

    for face in faces:
        built.faces.new([verts[index] for index in face])

    bmesh.ops.remove_doubles(built, verts=built.verts, dist=1e-6)
    if tidy:
        bmesh.ops.dissolve_degenerate(built, edges=built.edges, dist=1e-6)
    if recalc:
        bmesh.ops.recalc_face_normals(built, faces=built.faces)
    built.to_mesh(mesh)
    built.free()

    obj = bpy.data.objects.new(mat, mesh)
    obj.data.materials.append(material(mat))
    bpy.context.scene.collection.objects.link(obj)

    if bevel > 0.0:
        modifier = obj.modifiers.new("bevel", "BEVEL")
        modifier.width = bevel
        modifier.segments = segments
        modifier.limit_method = "ANGLE"
        modifier.angle_limit = math.radians(30.0)
        modifier.use_clamp_overlap = True
        modifier.harden_normals = False

    return obj


MAGIC = b"CMSH"
VERSION = 1
QUANTA = 65535
HALF_QUANTA = 32768
NORMAL_SCALE = 127


def to_game(point):
    x, y, z = point
    return (x, z, -y)


def group_triangles(obj):
    mesh = obj.data
    mesh.calc_loop_triangles()
    triangles = []
    for triangle in mesh.loop_triangles:
        normal = to_game(mesh.polygons[triangle.polygon_index].normal)
        corners = [to_game(mesh.vertices[index].co) for index in triangle.vertices]
        triangles.append((corners, normal))
    return triangles


def quantise(value, low, high):
    span = max(high - low, 1e-9)
    return round((value - low) / span * QUANTA) - HALF_QUANTA


def write_blob(path, groups):
    every = [corner for _, triangles in groups for corners, _ in triangles
             for corner in corners]
    low = [min(point[axis] for point in every) for axis in range(3)]
    high = [max(point[axis] for point in every) for axis in range(3)]

    out = bytearray(MAGIC)
    out += struct.pack("<HH", VERSION, len(groups))
    out += struct.pack("<3f", *low)
    out += struct.pack("<3f", *high)

    for name, triangles in groups:
        vertices = {}
        indices = []
        for corners, normal in triangles:
            packed_normal = tuple(round(axis * NORMAL_SCALE) for axis in normal)
            for corner in corners:
                packed = tuple(quantise(corner[axis], low[axis], high[axis])
                               for axis in range(3))
                key = (packed, packed_normal)
                if key not in vertices:
                    vertices[key] = len(vertices)
                indices.append(vertices[key])

        encoded = name.encode("ascii")
        out += struct.pack("<B", len(encoded)) + encoded
        out += struct.pack("<II", len(vertices), len(indices))
        for (position, normal), _ in sorted(vertices.items(), key=lambda item: item[1]):
            out += struct.pack("<3h", *position)
            out += struct.pack("<3b", *normal)
        wide = len(vertices) >= 65536
        out += struct.pack("<%d%s" % (len(indices), "I" if wide else "H"), *indices)

    with open(path, "wb") as blob:
        blob.write(out)
    return sum(len(triangles) for _, triangles in groups)


def finish(path, materials, name):
    bpy.ops.object.select_all(action="SELECT")
    bpy.context.view_layer.objects.active = bpy.context.scene.objects[0]
    bpy.ops.object.convert(target="MESH")

    groups = []
    for group in materials:
        objects = [obj for obj in bpy.context.scene.objects
                   if obj.data.materials and obj.data.materials[0].name == group]
        if not objects:
            continue
        bpy.ops.object.select_all(action="DESELECT")
        for obj in objects:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = objects[0]
        bpy.ops.object.join()
        groups.append((group, group_triangles(bpy.context.view_layer.objects.active)))

    triangles = write_blob(path, groups)
    print(f"{name}: {triangles} triangles")
