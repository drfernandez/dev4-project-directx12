import math
import bpy
import mathutils
import os
import bmesh

# path to the asset folder
export_path = "C:\\Users\\dfern\\Desktop\\Assets\\Exported"

# function to export all the MESH type in the scene to an .obj and .mtl
def export_obj(object):
    path = export_path + "\\" + object.name + ".obj"
    bpy.ops.export_scene.obj(
        filepath=path, 
        check_existing=True, 
        filter_glob='*.obj;*.mtl', 
        use_selection=True, 
        use_animation=False, 
        use_mesh_modifiers=True, 
        use_edges=True, 
        use_smooth_groups=False, 
        use_smooth_groups_bitflags=False, 
        use_normals=True, 
        use_uvs=True, 
        use_materials=True, 
        use_triangles=True, 
        use_nurbs=False, 
        use_vertex_groups=False, 
        use_blen_objects=True, 
        group_by_object=False, 
        group_by_material=False, 
        keep_vertex_order=False, 
        global_scale=1.0, 
        path_mode='STRIP', 
        axis_forward='-Z', 
        axis_up='Y')

# found at: https://www.versluis.com/2021/12/how-to-export-multiple-objects-from-blender-as-obj-or-fbx/
def export_obj_from_scene(path):
    selection = bpy.context.selectable_objects
    bpy.ops.object.select_all(action='DESELECT')
    for obj in selection:
        if obj.type == 'MESH':
            obj.select_set(True)
            bpy.context.view_layer.objects.active = obj
            #name = bpy.path.clean_name(obj.name)
            export_obj(obj)
            obj.select_set(False)

export_obj_from_scene(export_path)