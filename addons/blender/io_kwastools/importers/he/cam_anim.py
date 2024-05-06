import bpy as bpy
from enum import Enum
import math

from io_kwastools.common.common import *
from io_kwastools.common.he_common import *

# Does simple division and round up on X resolution
# All calculations assume 1920px width
# It doesn't really matter for the game, but for blender preview? Certainly.
def apply_aspect_to_resy(aspect_ratio):
	render = bpy.context.scene.render
	res_y = round(float(1920)/float(aspect_ratio))
	render.resolution_y = res_y

# Reads Animation params and sets camera+tracker initial settings
# TODO: add rot_or_aim handler
#		defaults to aim for now
def init_camera_tracker_values(cam_data, cam_obj, tracker, anim):

	start_frame = int(anim.get('start_frame'))

	# Set initial values for the camera
	cam_obj.location = [float(anim.get('cam_pos_x')),
						-float(anim.get('cam_pos_y')),
						float(anim.get('cam_pos_z'))]

	cam_obj.rotation_euler = [float(anim.get('cam_rot_x')),
							  float(anim.get('cam_rot_y')),
							  float(anim.get('cam_rot_z'))]
							  
	cam_data.clip_start = float(anim.get('z_near'))
	cam_data.clip_end = float(anim.get('z_far'))

	# Converting HE FOV to Blender's
	cam_data.angle = he_fov_to_blender_angle(float(anim.get('fov')), float(anim.get('aspect_ratio')))
	
	# Inserting keyframe values for the camera at start_frame
	cam_obj.keyframe_insert('location', frame = start_frame)
	cam_obj.keyframe_insert('rotation_euler', frame = start_frame)
	cam_data.keyframe_insert('lens', frame = start_frame)
	cam_data.keyframe_insert('clip_start', frame = start_frame)
	cam_data.keyframe_insert('clip_end', frame = start_frame)
	
	# Set initial values for the tracer
	tracker.location = [float(anim.get('aim_pos_x')),
						-float(anim.get('aim_pos_y')),
						float(anim.get('aim_pos_z'))]
	
	# Inserting keyframe values for the trancer at start_frame
	tracker.keyframe_insert('location', frame = start_frame)
	tracker.keyframe_insert('rotation_euler', index = 2, frame = start_frame)
	tracker.rotation_euler.z = float(anim.get('twist'))

def process_animation(cam_data, cam_obj, tracker, anim, kfs):
	anim_type = CamAnimTypes(int(kfs.get('type')))
	start = int(anim.get('start_frame'))
	keyframes = kfs.findall('Keyframe')

	for kf in iter(keyframes):
		value = float(kf.get('value'))
		index = float(kf.get('index'))
		
		match anim_type:
			case CamAnimTypes.cam_pos_x:
				cam_obj.location.x = value
				cam_obj.keyframe_insert('location', index = 0, frame = index)
			case CamAnimTypes.cam_pos_y:
				cam_obj.location.y = -value
				cam_obj.keyframe_insert('location', index = 1, frame = index)
			case CamAnimTypes.cam_pos_z:
				cam_obj.location.z = value
				cam_obj.keyframe_insert('location', index = 2, frame = index)
			#case CamAnimTypes.cam_rot_x:
			#	cam_obj.rotation_euler.x = value
			#	cam_obj.keyframe_insert('rotation_euler', index = 0, frame = index)
			#case CamAnimTypes.cam_rot_y:
			#	cam_obj.rotation_euler.y = value
			#	cam_obj.keyframe_insert('rotation_euler', index = 1, frame = index)
			#case CamAnimTypes.cam_rot_z:
			#	cam_obj.rotation_euler.z = value
			#	cam_obj.keyframe_insert('rotation_euler', index = 2, frame = index)
					
			case CamAnimTypes.aim_pos_x:
				tracker.location.x = value
				tracker.keyframe_insert('location', index = 0, frame = index)
			case CamAnimTypes.aim_pos_y:
				tracker.location.y = -value
				tracker.keyframe_insert('location', index = 1, frame = index)
			case CamAnimTypes.aim_pos_z:
				tracker.location.z = value
				tracker.keyframe_insert('location', index = 2, frame = index)
			case CamAnimTypes.twist:
				tracker.rotation_euler.z = value
				tracker.keyframe_insert('rotation_euler', index = 2, frame = index)

			case CamAnimTypes.z_near:
				cam_data.clip_start = value
				cam_data.keyframe_insert('clip_start', frame = index)
			case CamAnimTypes.z_far:
				cam_data.clip_end = value
				cam_data.keyframe_insert('clip_end', frame = index)

			case CamAnimTypes.fov:
				cam_data.angle = he_fov_to_blender_angle(value, bl_get_current_aspect_ratio())
				cam_data.keyframe_insert('lens', frame = index)

	# Set all keyframes to linear interpolation
	fcurves_obj = cam_obj.animation_data.action.fcurves
	fcurves_data = cam_data.animation_data.action.fcurves
	fcurves_tracker = tracker.animation_data.action.fcurves
	bl_set_fcurve_to_linear(fcurves_obj)
	bl_set_fcurve_to_linear(fcurves_data)
	bl_set_fcurve_to_linear(fcurves_tracker)

def load(filename, root):
	scn = bpy.context.scene
	cam_collection = bl_create_collection(str(filename + " Cameras"))
	
	# Will be used later
	# Each Animation has its own fps, but mostly it's the same
	fps = 0
	
	for anim in root.iter('Animation'):
		cam_name = anim.get('name')
		fps = int(anim.get('frame_rate'))
		apply_aspect_to_resy(anim.get('aspect_ratio'))
		
		# Create new camera and its tracker
		camera_data = bpy.data.cameras.new(cam_name + " Data")
		camera_obj = bpy.data.objects.new(cam_name, camera_data)
		camera_tracker = bpy.data.objects.new(cam_name + " Tracker", None)
		
		# Create the marker on the timeline for current camera
		marker = scn.timeline_markers.new(cam_name, frame = int(anim.get('start_frame')))
		marker.camera = camera_obj
		
		# Set up camera constraints
		# First we stretch to the position of the tracker
		# Second we use Damped Track to make sure the camera won't do 180 (tracking the -Z)
		stretch_to_constraint = camera_obj.constraints.new(type='STRETCH_TO')
		stretch_to_constraint.target = camera_tracker
		stretch_to_constraint.volume = 'NO_VOLUME'
		stretch_to_constraint.keep_axis = 'PLANE_Z'
		
		copy_rot_constraint = camera_obj.constraints.new(type='COPY_ROTATION')
		copy_rot_constraint.mix_mode = 'AFTER'
		copy_rot_constraint.use_x = False
		copy_rot_constraint.use_y = False
		copy_rot_constraint.target = camera_tracker

		damped_constraint = camera_obj.constraints.new(type='DAMPED_TRACK')
		damped_constraint.target = camera_tracker
		damped_constraint.track_axis = 'TRACK_NEGATIVE_Z'
		
		# Init camera and tracker values
		init_camera_tracker_values(camera_data, camera_obj, camera_tracker, anim)
		
		# Load keyframes from keyframe sets
		for kfs in anim.iter('KeyframeSet'):
			process_animation(camera_data, camera_obj, camera_tracker, anim, kfs)
		
		# Link camera and tracker to event's collection
		cam_collection.objects.link(camera_obj)
		cam_collection.objects.link(camera_tracker)
		
	# Set scene settings
	frames_range = he_get_frames_range(root)
	bl_set_scene_settings(fps, frames_range[0], frames_range[1])