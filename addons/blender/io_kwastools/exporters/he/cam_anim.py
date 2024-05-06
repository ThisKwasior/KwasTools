import bpy
import sys

import xml.etree.ElementTree as ET

from io_kwastools.common.common import *
from io_kwastools.common.he_common import *

'''
# Prepares the root of CAMAnimation XML
def get_xml_cam_root():
	camanim = ET.Element('CAMAnimation')
	camanim.set('root_node_type', '2')
	return camanim


# Creates the keyframe set xml element
def old_create_keyframe_set_xml_root(anim, type):
	kfs = ET.SubElement(anim, 'KeyframeSet')
	kfs.set('type', str(int(type)))
	kfs.set('flag2', str(0))
	kfs.set('interpolation', str(0))
	kfs.set('flag4', str(0))
	return kfs

def append_xml_keyframes(kfs_xml, data):
	for kf in iter(data):
		kf_elem = ET.SubElement(kfs_xml, 'Keyframe')
		kf_elem.set('index', str(int(kf[0])))
		kf_elem.set('value', str(float(kf[1])))
'''

# Camera position keyframes
def create_camera_pos_kfs(anim_xml_root, object):
	if bool(object.animation_data) == False: return
	if bool(object.animation_data.action) == False: return
	fc = object.animation_data.action.fcurves

	posx = bl_get_fcurve_data_by_path_id(fc, 'location', 0)
	posy = bl_get_fcurve_data_by_path_id(fc, 'location', 1)
	posz = bl_get_fcurve_data_by_path_id(fc, 'location', 2)

	# Camera X position
	if posx is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.cam_pos_x._value_)
		he_append_xml_keyframes(kfs, posx)

	# Camera Z position
	if posz is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.cam_pos_z._value_)
		he_append_xml_keyframes(kfs, posz)
		
	# Camera Y position
	if posy is not None:
		# Negate this mf
		for kf in iter(posy):
			kf[1] = -kf[1]

		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.cam_pos_y._value_)
		he_append_xml_keyframes(kfs, posy)

# Camera properties keyframes	
def create_camera_misc_kfs(anim_xml_root, object):
	if bool(object.data.animation_data) == False: return
	if bool(object.data.animation_data.action) == False: return
	fc = object.data.animation_data.action.fcurves

	start = bl_get_fcurve_data_by_path_id(fc, 'clip_start', 0)
	end = bl_get_fcurve_data_by_path_id(fc, 'clip_end', 0)
	lens = bl_get_fcurve_data_by_path_id(fc, 'lens', 0)
	
	# Camera clip start
	if start is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.z_near._value_)
		he_append_xml_keyframes(kfs, start)
		
	# Camera clip end
	if end is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.z_far._value_)
		he_append_xml_keyframes(kfs, end)
		
	# Camera clip start
	if lens is not None:
		# Convert to HE FOV
		for kf in iter(lens):
			bpy.context.scene.frame_set(frame=int(kf[0]))
			kf[1] = blender_angle_to_he_fov(object.data.angle, bl_get_current_aspect_ratio())
		
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.fov._value_)
		he_append_xml_keyframes(kfs, lens)

# Tracker position and twist keyframes
def create_tracker_pos_twist_kfs(anim_xml_root, object):
	if bool(object.animation_data) == False: return
	if bool(object.animation_data.action) == False: return
	fc = object.animation_data.action.fcurves

	posx = bl_get_fcurve_data_by_path_id(fc, 'location', 0)
	posy = bl_get_fcurve_data_by_path_id(fc, 'location', 1)
	posz = bl_get_fcurve_data_by_path_id(fc, 'location', 2)
	twist = bl_get_fcurve_data_by_path_id(fc, 'rotation_euler', 2)

	# Tracker X position
	if posx is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.aim_pos_x._value_)
		he_append_xml_keyframes(kfs, posx)

	# Tracker Z position
	if posz is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.aim_pos_z._value_)
		he_append_xml_keyframes(kfs, posz)
		
	# Tracker Y position
	if posy is not None:
		# Negate this mf
		for kf in iter(posy):
			kf[1] = -kf[1]

		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.aim_pos_y._value_)
		he_append_xml_keyframes(kfs, posy)
		
	# Tracker twist
	if twist is not None:
		kfs = he_create_keyframe_set_xml_root(anim_xml_root, CamAnimTypes.twist._value_)
		he_append_xml_keyframes(kfs, twist)

# Fills the anim_xml_root with all keyframe sets that are available
def prep_keyframe_sets(anim_xml_root, object, target):
	# Camera position
	create_camera_pos_kfs(anim_xml_root, object)
	
	# Camera clip and lens
	create_camera_misc_kfs(anim_xml_root, object)
	
	# Tracker pos and twist
	if target is not None:
		create_tracker_pos_twist_kfs(anim_xml_root, target)
	pass

# Prepares the xml sub-element with animation of a camera
# TODO: add rot_or_aim handler
#		defaults to aim for now
def prep_animation(xml_root, object):
	anim = ET.SubElement(xml_root, 'Animation')
	
	# Camera default params
	name = object.name
	frame_rate = bl_get_current_scene_fps()
	
	start_frame = 0
	end_frame = 0
	if object.animation_data is None:
		start_frame = bpy.context.scene.frame_start
		end_frame = bpy.context.scene.frame_end
	elif object.animation_data.action is None:
		start_frame = bpy.context.scene.frame_start
		end_frame = bpy.context.scene.frame_end
	else:
		start_frame = object.animation_data.action.frame_range.x
		end_frame = object.animation_data.action.frame_range.y

	cam_pos_x = object.location.x
	cam_pos_y = -object.location.y
	cam_pos_z = object.location.z
	cam_rot_x = object.rotation_euler.x
	cam_rot_y = object.rotation_euler.y
	cam_rot_z = object.rotation_euler.z
	aim_pos_x = 0.0
	aim_pos_y = 0.0
	aim_pos_z = 0.0
	twist = 0.0
	z_near = object.data.clip_start
	z_far = object.data.clip_end
	fov = blender_angle_to_he_fov(object.data.angle, bl_get_current_aspect_ratio())
	aspect_ratio = bl_get_current_aspect_ratio()
	
	# target of STRETCH_TO constraint
	target = None
	
	# Check for STRETCH_TO constraint and if found
	# fill the default tracker pos and twist params
	for cons in iter(object.constraints):
		if(cons.type == "STRETCH_TO"):
			target = cons.target
			aim_pos_x = target.location.x
			aim_pos_y = -target.location.y
			aim_pos_z = target.location.z
			twist = target.rotation_euler.z

	anim.set('name', str(name))
	anim.set('rot_or_aim', str(int(1)))
	anim.set('flag2', str(int(0)))
	anim.set('flag3', str(int(0)))
	anim.set('flag4', str(int(0)))
	anim.set('frame_rate', str(int(frame_rate)))
	anim.set('start_frame', str(int(start_frame)))
	anim.set('end_frame', str(int(end_frame)))
	anim.set('cam_pos_x', str(float(cam_pos_x)))
	anim.set('cam_pos_y', str(float(cam_pos_y)))
	anim.set('cam_pos_z', str(float(cam_pos_z)))
	anim.set('cam_rot_x', str(float(cam_rot_x)))
	anim.set('cam_rot_y', str(float(cam_rot_y)))
	anim.set('cam_rot_z', str(float(cam_rot_z)))
	anim.set('aim_pos_x', str(float(aim_pos_x)))
	anim.set('aim_pos_y', str(float(aim_pos_y)))
	anim.set('aim_pos_z', str(float(aim_pos_z)))
	anim.set('twist', str(float(twist)))
	anim.set('z_near', str(float(z_near)))
	anim.set('z_far', str(float(z_far)))
	anim.set('fov', str(float(fov)))
	anim.set('aspect_ratio', str(float(aspect_ratio)))

	prep_keyframe_sets(anim, object, target)

	return anim

def prep_camanim_xml(filepath):
	#cam_xml = get_xml_cam_root()
	cam_xml = he_get_xml_he_anim_root('CAMAnimation', 2)
	
	# Store the current frame position
	frame_current = bpy.context.scene.frame_current

	# Set the currect scene frame to 0
	bpy.context.scene.frame_set(frame=0) 

	objects = bpy.context.scene.objects

	for object in iter(objects):
		if(object.type == "CAMERA"):
			if object.animation_data is not None:
				prep_animation(cam_xml, object)

	# Restore the frame position
	bpy.context.scene.frame_set(frame_current)

	return cam_xml

def save(filepath):
	print("cam_anim.save()")
	cam_xml = prep_camanim_xml(filepath)
	save_pretty_xml(filepath, cam_xml)