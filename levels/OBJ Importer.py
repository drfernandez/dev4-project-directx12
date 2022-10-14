import math
import bpy
import mathutils
import os
import bmesh

# path to the asset folder
asset_path = "D:\\Code\\Full Sail\\DEV4\\Assets\\Updated Modular Dungeon - May 2019\\OBJ"
# file list inside the directory
file_list = os.listdir(asset_path)

# function to import an .obj to the scene
def import_obj(model_name):
    # create a path to the file so we can add it to blender
    target_path = asset_path + "\\" + model_name
    # import the obj to the blender scene
    bpy.ops.import_scene.obj(
        filepath=target_path, 
        filter_glob='*.obj;*.mtl', 
        use_edges=True, 
        use_smooth_groups=True, 
        use_split_objects=False, 
        use_split_groups=False, 
        use_groups_as_vgroups=False, 
        use_image_search=True, 
        split_mode='ON', 
        global_clamp_size=0.0, 
        axis_forward='-Z', 
        axis_up='Y')     
    # change the origin to center of mass (surface)
    bpy.ops.object.origin_set(
        type='ORIGIN_CENTER_OF_MASS',
        center='MEDIAN')
            

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
        # import the obj to the scene
        import_obj(item)
        # select the current object we just imported
        obj_object = bpy.context.selected_objects[0]
        # change the model's name in blender to match the file's name
        file_tuple = os.path.splitext(item)
        obj_object.name = file_tuple[0]
        # move the current object to a new location
        obj_object.matrix_world.translation = position

# call the function to load the files from a directory
import_obj_from_directory(asset_path)
