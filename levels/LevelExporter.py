# DEV4 Simple Level Exporter v1.0
# a simple export script based on this answer from Blender stack exchange:
# https://blender.stackexchange.com/a/146344

import bpy
import os
from bpy_extras.io_utils import axis_conversion
import mathutils
import math
import bl_math

def print_object_to_console(spaces, ob):
    if ob.type == 'LIGHT':
        print(spaces, ob.type)
    converted = mathutils.Matrix.Identity(4)
    if ob.type == 'LIGHT':
        print(spaces, ob.data.type)
        # write out the following information for the light
        # position
        # direction
        # attributes (inner, outer, radius)
        # color
        wm = ob.matrix_world.transposed()
        pos = mathutils.Vector((0,0,0,0))
        dir = mathutils.Vector((0,0,0,0))
        attrib = mathutils.Vector((-1,-1,-1,-1))
        
        if ob.data.type == 'POINT':
            pos = mathutils.Vector((wm[3][0], wm[3][1], wm[3][2], 1))
            dir = mathutils.Vector((-0,-0,-0))
            attrib = mathutils.Vector((-0,-0,ob.data.cutoff_distance, ob.data.energy))
        elif ob.data.type == 'SPOT':
            pos = mathutils.Vector((wm[3][0], wm[3][1], wm[3][2], 2))
            dir = mathutils.Vector((wm[2][0], wm[2][1], wm[2][2]))
            inner = math.cos((ob.data.spot_size - ob.data.spot_blend) * 0.5)
            outer = math.cos(ob.data.spot_size * 0.5)
            radius = ob.data.cutoff_distance
            power = ob.data.energy
            attrib = mathutils.Vector((inner,outer,radius,power))
            
        converted[0][0:4] = pos
        converted[1][0:3] = dir
        converted[2][0:4] = attrib
        converted[3][0:3] = ob.data.color
        print(spaces, converted)
    else:
        #print(spaces, ob.name)
        converted = ob.matrix_world.transposed()
        #print(spaces, converted)

# write the object to the file
def write_object_to_file(file, spaces, ob):
    # write the type 
    file.write(spaces + ob.type + "\n")
    # write the name
    file.write(spaces + ob.name + "\n")
    # matrix variable to write to file
    converted = mathutils.Matrix.Identity(4)
    # swap from blender space to vulkan/d3d 
    # { rx, ry, rz, 0 } to { rx, rz, ry, 0 }  
    # { ux, uy, uz, 0 }    { ux, uz, uy, 0 }
    # { lx, ly, lz, 0 }    { lx, lz, ly, 0 } 
    # { px, py, pz, 1 }    { px, pz, py, 1 }  
    row_world = ob.matrix_world.transposed()
    converted[0][0:3] = row_world[0][0], row_world[0][2], row_world[0][1]
    converted[1][0:3] = row_world[1][0], row_world[1][2], row_world[1][1] 
    converted[2][0:3] = row_world[2][0], row_world[2][2], row_world[2][1] 
    converted[3][0:3] = row_world[3][0], row_world[3][2], row_world[3][1]
    # flip the local Z axis for winding and transpose for export
    scaleZ = mathutils.Matrix.Scale(-1.0, 4, (0.0, 0.0, 1.0))
    converted = scaleZ.transposed() @ converted
    # if the type is a light
    if ob.type == 'LIGHT':
        # write out the following information for the light
        # position
        # direction
        # attributes (inner, outer, radius)
        # color
        row_world = converted
        col = ob.data.color
        pos = mathutils.Vector((0,0,0,0))
        dir = mathutils.Vector((0,0,0,0))
        attrib = mathutils.Vector((-1,-1,-1,-1))
        power = 0
        
        if ob.data.type == 'POINT':
            pos = mathutils.Vector((row_world[3][0], row_world[3][1], row_world[3][2], 1))
            dir = mathutils.Vector((-1,-1,-1))
            attrib = mathutils.Vector((-0,-0,ob.data.cutoff_distance, ob.data.energy))
        elif ob.data.type == 'SPOT':
            pos = mathutils.Vector((converted[3][0], converted[3][1], converted[3][2], 2))
            dir = mathutils.Vector((converted[2][0], converted[2][1], converted[2][2]))
            inner = math.cos((ob.data.spot_size - ob.data.spot_blend) * 0.5)
            outer = math.cos(ob.data.spot_size * 0.5)
            radius = ob.data.cutoff_distance
            power = ob.data.energy
            attrib = mathutils.Vector((inner,outer,radius,power))
            
        display_mat = mathutils.Matrix.Identity(4)
        display_mat[0][0:4] = pos
        display_mat[1][0:3] = dir
        display_mat[2][0:4] = attrib
        display_mat[3][0:3] = col
        file.write(spaces + str(display_mat) + "\n")
    # default case to write matrix data
    else:
        file.write(spaces + str(converted) + "\n")
# end def write_object_to_file(file, spaces, ob)
    
# print/write object information
def print_heir(ob, levels=10):
    # recursive function to print/write object information
    def recurse(ob, parent, depth):
        if depth > levels: 
            return
        
        # spacing to show hierarchy
        spaces = "  " * depth;        
        # print to system console for debugging
        print_object_to_console(spaces, ob)        
        # send to file
        write_object_to_file(file, spaces, ob)        
         
        # TODO: For a game ready exporter we would
        # probably want the delta(pivot) matrix, lights,
        # detailed mesh hierarchy information
        # and bounding box/collission data at minimum
        
        # Bounding Box information
        bbox_corners = [mathutils.Vector(corner) for corner in ob.bound_box]
        #for corner in bbox_corners:
            #print(spaces, corner)

        for child in ob.children:
            recurse(child, ob,  depth + 1)
    #end def recurse(ob, parent, depth)
    
    # call the recursive function inside print_heir 
    recurse(ob, ob.parent, 0)
#end def print_heir(ob, levels=10)

# Python script starts here
print("----------Begin Level Export----------")

filename = bpy.path.display_name(bpy.context.blend_data.filepath) + ".txt"
filename = "GameLevel.txt"
path = os.path.join(os.path.dirname(bpy.data.filepath), filename)
file = open(path,"w")
file.write("# Game Level Exporter v1.1\n")

scene = bpy.context.scene
root_obs = (o for o in scene.objects if not o.parent)
# Loop for all the objects 
for o in root_obs:
    # recursively print the heirarchy
    print_heir(o)
    
file.close()

print("----------End Level Export----------")

# check the blender python API docs under "Object(ID)"
# that is where I found the "type" and "matrix_world"
# there is so much more useful stuff for a game level!
