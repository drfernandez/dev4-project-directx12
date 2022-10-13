import math
import bpy
import mathutils
import os
import bmesh

# path to the asset folder
asset_path = "C:\\Users\\drfernandez\\Desktop\\Assets\\Original"
# file list inside the directory
file_list = os.listdir(asset_path)
# number of files in the directory
file_total = len(file_list)

# function to load all obj files from within a directory
def import_obj_from_directory(path):    
    # create a list to store the .obj files
    model_list = []
    # variable to move the model into position
    position = mathutils.Vector((0.0, 0.0, 0.0))
    
    # separate the .mtl and .obj in the folder
    for item in file_list:
        # get the file name and file extension stored into a tuple
        file_tuple = os.path.splitext(item)
        # compare the extension to .mtl, we do not want to load these
        if file_tuple[1] == ".mtl":
            # skip over the current file in the directory
            continue
        # add the current item to the model_list
        model_list.append(item)

    # loop over all the objs and import them to the scene
    for item in model_list:
        # get the current model from the list
        current_model = item
        # create a path to the file so we can add it to blender
        target_path = path + "\\" + current_model
        # import the obj to the blender scene
        bpy.ops.import_scene.obj(filepath=target_path)
        # select the current object we just imported
        obj_object = bpy.context.selected_objects[0]
        # change the model's name in blender to match the file's name
        file_tuple = os.path.splitext(current_model)
        obj_object.name = file_tuple[0]
        # move the current object to a new location
        obj_object.matrix_world.translation = position
        # update the position
        position.x += 5.0
        if position.x >= 25.0:
            position.x = 0.0
            position.y += 5.0

# call the function to load the files from a directory
import_obj_from_directory(asset_path)
