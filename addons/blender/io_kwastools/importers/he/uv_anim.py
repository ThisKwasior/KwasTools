import bpy as bpy
from enum import Enum
import math

from io_kwastools.common.common import *
from io_kwastools.common.he_common import *

'''
def process_animation(modifier, anim, kfs):
	anim_type = UVAnimTypes(int(kfs.get('type')))
	start = int(anim.get('start_frame'))
	keyframes = kfs.findall('Keyframe')
	
	match anim_type:
		case UVAnimTypes.pos_x:
			for kf in iter(keyframes):
				modifier.offset[0] = float(kf.get('value'))
				modifier.keyframe_insert('offset', index = 0, frame = int(kf.get('index')))
		case UVAnimTypes.pos_y:
			for kf in iter(keyframes):
				modifier.offset[1] = -float(kf.get('value'))
				modifier.keyframe_insert('offset', index = 1, frame = int(kf.get('index')))

def load_old(filename, root):
	scn = bpy.context.scene
	selected = bpy.context.selected_objects
	
	print(selected)

	for mesh in iter(selected):
		for anim in root.iter('Animation'):
			name = anim.get('name')
			modifier = mesh.modifiers.new(name, 'UV_WARP')
			
			# Load keyframes from keyframe sets
			for kfs in anim.iter('KeyframeSet'):
				process_animation(modifier, anim, kfs)
'''

# Adds UVAnimator node group to the material
def add_uvanimator_to_material(material):
	# Check if the node group with shader exists
	# If it does, use it, otherwise create it
	uvanimator_shader = None
	for group in bpy.data.node_groups:
		if group.name == 'UVAnimatorGroup':
			uvanimator_shader = group
			break
	
	if uvanimator_shader == None:
		uvanimator_shader = he_create_uv_animator_nodegroup()
	
	shadernodegroup = material.node_tree.nodes.new('ShaderNodeGroup')
	shadernodegroup.label = 'UV Animator'
	shadernodegroup.node_tree = uvanimator_shader
	
	return shadernodegroup

def check_add_uvanimator_to_image(material, texture_node):
	# Find if the image node has the Node Group already
	group_node = None

	for node_link in texture_node.inputs['Vector'].links:
		if node_link.from_node.type == 'GROUP':
			group_node = node_link.from_node

	if group_node == None:
		group_node = add_uvanimator_to_material(material)
		group_node.location.x = texture_node.location.x - group_node.width - 40
		group_node.location.y = texture_node.location.y
		material.node_tree.links.new(group_node.outputs['Output Vector'], texture_node.inputs['Vector'])
		
	return group_node

# Inserts keyframes to the uv animator by type
def apply_animation_to_uvanimator(xmlanim, uvanmtr, material):
	# Start over, new animation incoming
	he_remove_animation_data_from_uv_animator(uvanmtr)

	for kfs in xmlanim.iter('KeyframeSet'):
		type = int(kfs.get('type'))
		anim_type = UVAnimTypes(type)

		for kf in kfs.iter('Keyframe'):
			value = float(kf.get('value'))
			index = float(kf.get('index'))
			
			data_path = 'nodes["'+ uvanmtr.name +'"].inputs[' + str(type) + '].default_value'
			
			match anim_type:
				case UVAnimTypes.pos_x:
					uvanmtr.inputs['Pos X'].default_value = value
					material.node_tree.keyframe_insert(data_path, frame = index)
				case UVAnimTypes.pos_y:
					uvanmtr.inputs['Pos Y'].default_value = value
					material.node_tree.keyframe_insert(data_path, frame = index)
				case UVAnimTypes.rot:
					uvanmtr.inputs['Rotation'].default_value = value
					material.node_tree.keyframe_insert(data_path, frame = index)
				case UVAnimTypes.scale_x:
					uvanmtr.inputs['Scale X'].default_value = value
					material.node_tree.keyframe_insert(data_path, frame = index)
				case UVAnimTypes.scale_y:
					uvanmtr.inputs['Scale Y'].default_value = value
					material.node_tree.keyframe_insert(data_path, frame = index)

	bl_set_fcurve_to_linear(uvanmtr.id_data.animation_data.action.fcurves)
			
# uv-anim with root_node_type 3
# Hedgehog Engine 2
# Can animate multiple textures in one material
def process_uvanim_v3(xmlroot):
	target_mat_name = str(xmlroot.get('material_name'))
	print('Target material:', target_mat_name)
	
	material = bl_get_material_by_name(target_mat_name)
	
	if material == None:
		return
	
	mat_nodetree = material.node_tree
	mat_nodes = mat_nodetree.nodes
	
	print(material)
	print(mat_nodes)
	
	for anim in xmlroot.iter('Animation'):
		texture_names = str(anim.get('name')).split('/')
		print(texture_names);
		
		for texture_name in texture_names:
			print('Now processing:', texture_name)
			texture_node = bl_get_tex_image_node(mat_nodes, texture_name)
			print(texture_node)
			
			if texture_node:
				group_node = check_add_uvanimator_to_image(material, texture_node)
				apply_animation_to_uvanimator(anim, group_node, material)
			else:
				print('Texture', texture_name, 'does not exist in this material. Skipping...')
			

def load(filename, root):
	root_node_type = int(root.get('root_node_type'))
	
	if root_node_type == 2: # HE1 uv-anim with only one texture animated
		print('root_node_type', root_node_type)
	elif root_node_type == 3: # HE2 uv-anim with multiple textures animated
		process_uvanim_v3(root)
	
	pass