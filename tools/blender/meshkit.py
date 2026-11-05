# What the prop scripts share: solids written in the game's space (x right,
# y up, -z forward) and turned into Blender's z-up space as they are made, one
# Blender material per group, and the export of one OBJ grouped by material.

import math

import bmesh
import bpy


def to_blender(point):
    x, y, z = point
    return (x, -z, y)


def material(name):
    found = bpy.data.materials.get(name)
    return found if found else bpy.data.materials.new(name)


def solid(mat, points, faces, bevel=0.0, segments=1, recalc=True, tidy=False):
    """A mesh of `faces` (lists of indices into `points`) in material `mat`.
    With `recalc`, Blender turns the faces outward; without, the faces must
    already wind counter-clockwise seen from outside. `tidy` drops the faces
    that merging points left with no area."""
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


def finish(path, materials, name):
    """Joins every object of each of `materials` into one flat-shaded object
    and exports the scene to `path`, one OBJ group per material."""
    bpy.ops.object.select_all(action="SELECT")
    bpy.context.view_layer.objects.active = bpy.context.scene.objects[0]
    bpy.ops.object.convert(target="MESH")

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
        joined = bpy.context.view_layer.objects.active
        joined.name = group
        joined.data.name = group
        for polygon in joined.data.polygons:
            polygon.use_smooth = False

    bpy.ops.wm.obj_export(filepath=path, export_selected_objects=False,
                          export_uv=False, export_normals=True,
                          export_materials=True, export_triangulated_mesh=True,
                          forward_axis="NEGATIVE_Z", up_axis="Y",
                          apply_modifiers=True)

    faces = sum(len(obj.data.polygons) for obj in bpy.context.scene.objects)
    print(f"{name}: {faces} faces before triangulating")
