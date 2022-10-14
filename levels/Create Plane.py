import bpy
import bmesh

# TriangulateObject
def triangulate_object(obj):
    # Store the current data
    me = obj.data
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    # Finish up, write the bmesh back to the mesh
    bm.to_mesh(me)
    bm.free()


# CreatePlane
def CreatePlane(minRange, maxRange):
    # Store a spacing
    spacing = 2
    # Loop for the number of columns
    for col in range(minRange, maxRange):
        # Loop for the number of rows
        for row in range(minRange, maxRange):
            # Store the location
            loc = (row * spacing, col * spacing, 0)
            # Create the plane at the desired location
            bpy.ops.mesh.primitive_plane_add(size=2, enter_editmode=False, align='WORLD', location=loc, scale=(1, 1, 1))
            # Get the active object (could be any mesh object)
            triangulate_object(bpy.context.active_object)


CreatePlane(-10, 11)