import bpy
import sys

import xml.etree.ElementTree as ET

from io_kwastools.common.common import *
from io_kwastools.common.he_common import *

'''
# Prepares the root of UVAnimation XML
def get_xml_uv_root(version):
	uvanim = ET.Element('UVAnimation')
	uvanim.set('root_node_type', str(version))
	return uvanim
'''

# Prepares the "Animation" sub-element with uv animation channels
def prep_animation_v3(xml_root, node, texture_name, fps, start, end):
	anim = ET.SubElement(xml_root, 'Animation')
	anim.set('name', str(texture_name))
	anim.set('frame_rate', str(fps))
	anim.set('start_frame', str(start))
	anim.set('end_frame', str(end))
	
	fc = node.id_data.animation_data.action.fcurves
	
	path_posx	= 'nodes["'+ node.name +'"].inputs[0].default_value'
	path_posy	= 'nodes["'+ node.name +'"].inputs[1].default_value'
	path_rot	= 'nodes["'+ node.name +'"].inputs[2].default_value'
	path_scalex	= 'nodes["'+ node.name +'"].inputs[3].default_value'
	path_scaley	= 'nodes["'+ node.name +'"].inputs[4].default_value'
	
	posx	= bl_get_fcurve_data_by_path_id(fc, path_posx, 0)
	posy	= bl_get_fcurve_data_by_path_id(fc, path_posy, 0)
	rot		= bl_get_fcurve_data_by_path_id(fc, path_rot, 0)
	scalex	= bl_get_fcurve_data_by_path_id(fc, path_scalex, 0)
	scaley	= bl_get_fcurve_data_by_path_id(fc, path_scaley, 0)
	
	# Position X
	if posx is not None:
		kfs = he_create_keyframe_set_xml_root(anim, UVAnimTypes.pos_x._value_)
		he_append_xml_keyframes(kfs, posx)
		
	# Position Y
	if posy is not None:
		kfs = he_create_keyframe_set_xml_root(anim, UVAnimTypes.pos_y._value_)
		he_append_xml_keyframes(kfs, posy)
		
	# Rotation
	if rot is not None:
		kfs = he_create_keyframe_set_xml_root(anim, UVAnimTypes.rot._value_)
		he_append_xml_keyframes(kfs, rot)

	# Scale X
	if scalex is not None:
		kfs = he_create_keyframe_set_xml_root(anim, UVAnimTypes.scale_x._value_)
		he_append_xml_keyframes(kfs, scalex)
		
	# Scale Y
	if scaley is not None:
		kfs = he_create_keyframe_set_xml_root(anim, UVAnimTypes.scale_y._value_)
		he_append_xml_keyframes(kfs, scaley)
		
	return anim

def prep_uvanimv3_xml(filepath):
	uv_xml = he_get_xml_he_anim_root('UVAnimation', 3)

	mat = bpy.context.object.active_material
	img_nodes = bl_get_nodes_by_type(mat.node_tree.nodes, "TEX_IMAGE")
	
	uv_xml.set('material_name', str(mat.name))
	uv_xml.set('texture_name', str('none'))
	
	fps = int(bl_get_current_scene_fps())
	start = int(bpy.context.scene.frame_start)
	end = int(bpy.context.scene.frame_end)
	
	for img in img_nodes:
		links = img.inputs['Vector'].links
		
		# Check if there are any links to Vector
		if(links != ()):
			group_node = links[0].from_node
			prep_animation_v3(uv_xml, group_node, img.image.name, fps, start, end)

	# Sort the Animations by texture name
	uv_xml[:] = sorted(uv_xml, key=lambda child: child.get('name'))

	return uv_xml

def save_v3(filepath):
	print("uv_anim.save_v3()")
	uv_xml = prep_uvanimv3_xml(filepath)
	print(uv_xml)
	print(filepath)
	save_pretty_xml(filepath, uv_xml)