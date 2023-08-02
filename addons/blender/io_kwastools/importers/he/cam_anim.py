import bpy as bpy
from enum import Enum

class Cam_Anim_Types(Enum):
	cam_pos_x	= 0
	cam_pos_z	= 1
	cam_pos_y	= 2
	cam_rot_x	= 3
	cam_rot_z	= 4
	cam_rot_y	= 5
	aim_pos_x	= 6
	aim_pos_z	= 7
	aim_pos_y	= 8
	aim_rot_z	= 9
	z_near		= 10
	z_far		= 11
	fov			= 12
	aspect		= 13

def create_collection(filename):
    cam_collection = bpy.data.collections.new(filename + " Cameras")
    bpy.context.scene.collection.children.link(cam_collection)
    
    return cam_collection

def init_camera_tracker_values(cam_data, cam_obj, tracker, anim):
	# Set initial values for the camera
	cam_obj.location = [float(anim.get('cam_pos_x')),
						-float(anim.get('cam_pos_y')),
						float(anim.get('cam_pos_z'))]
						
	cam_data.angle_y = float(anim.get('fov'))
	cam_data.keyframe_insert('lens', frame = int(anim.get('start_frame')))
    
	cam_obj.keyframe_insert('location', frame = int(anim.get('start_frame')))
	cam_data.keyframe_insert('clip_start', frame = int(anim.get('start_frame')))
	cam_data.keyframe_insert('clip_end', frame = int(anim.get('start_frame')))
	
	# Set initial values for the camera
	tracker.location = [float(anim.get('aim_pos_x')),
						-float(anim.get('aim_pos_y')),
						float(anim.get('aim_pos_z'))]

	tracker.rotation_euler.z = float(anim.get('aim_z_rotation'))
	tracker.keyframe_insert('location', frame = int(anim.get('start_frame')))
	tracker.keyframe_insert('rotation_euler', frame = int(anim.get('start_frame')))

def process_animation(cam_data, cam_obj, tracker, anim, kfs):
	anim_type = Cam_Anim_Types(int(kfs.get('type')))
	start = int(anim.get('start_frame'))
	keyframes = kfs.findall('Keyframe')
	
	match anim_type:
		case Cam_Anim_Types.cam_pos_x:
			for kf in iter(keyframes):
				cam_obj.location.x = float(kf.get('value'))
				cam_obj.keyframe_insert('location', index = 0, frame = int(kf.get('index')))
		case Cam_Anim_Types.cam_pos_y:
			for kf in iter(keyframes):
				cam_obj.location.y = -float(kf.get('value'))
				cam_obj.keyframe_insert('location', index = 1, frame = int(kf.get('index')))
		case Cam_Anim_Types.cam_pos_z:
			for kf in iter(keyframes):
				cam_obj.location.z = float(kf.get('value'))
				cam_obj.keyframe_insert('location', index = 2, frame = int(kf.get('index')))
				
		case Cam_Anim_Types.aim_pos_x:
			for kf in iter(keyframes):
				tracker.location.x = float(kf.get('value'))
				tracker.keyframe_insert('location', index = 0, frame = int(kf.get('index')))
		case Cam_Anim_Types.aim_pos_y:
			for kf in iter(keyframes):
				tracker.location.y = -float(kf.get('value'))
				tracker.keyframe_insert('location', index = 1, frame = int(kf.get('index')))
		case Cam_Anim_Types.aim_pos_z:
			for kf in iter(keyframes):
				tracker.location.z = float(kf.get('value'))
				tracker.keyframe_insert('location', index = 2, frame = int(kf.get('index')))
				
		case Cam_Anim_Types.aim_rot_z:
			for kf in iter(keyframes):
				tracker.rotation_euler.z = float(kf.get('value'))
				tracker.keyframe_insert('rotation_euler', index = 2, frame = int(kf.get('index')))
		case Cam_Anim_Types.fov:
			for kf in iter(keyframes):
				cam_data.angle_y = float(kf.get('value'))*float(anim.get('aspect_ratio'))
				cam_data.keyframe_insert('lens', frame = int(kf.get('index')))

    # Set all keyframes to linear interpolation
	fcurves_obj = cam_obj.animation_data.action.fcurves
	for fcurve in fcurves_obj:
		for kf in fcurve.keyframe_points:
			kf.interpolation = 'LINEAR'
            
	fcurves_data = cam_data.animation_data.action.fcurves
	for fcurve in fcurves_data:
		for kf in fcurve.keyframe_points:
			kf.interpolation = 'LINEAR'
    
	fcurves_tracker = tracker.animation_data.action.fcurves
	for fcurve in fcurves_tracker:
		for kf in fcurve.keyframe_points:
			kf.interpolation = 'LINEAR'		

def load(filename, root):
	scn = bpy.context.scene

	cam_collection = create_collection(filename)

	for anim in root.iter('Animation'):
		cam_name = anim.get('name')
		
		# Create new camera and its tracker
		camera_data = bpy.data.cameras.new(cam_name + " Data")
		camera_obj = bpy.data.objects.new(cam_name, camera_data)
		camera_tracker = bpy.data.objects.new(cam_name + " Tracker", None)
		
		# Set up camera
		marker = scn.timeline_markers.new(cam_name, frame = int(anim.get('start_frame')))
		marker.camera = camera_obj
		
		track_to_constraint = camera_obj.constraints.new(type='TRACK_TO')  
		copy_rot_constraint = camera_obj.constraints.new(type='COPY_ROTATION')
		copy_rot_constraint.mix_mode = 'AFTER'
		copy_rot_constraint.use_x = False
		copy_rot_constraint.use_y = False
		track_to_constraint.target = camera_tracker
		copy_rot_constraint.target = camera_tracker
		
		# Init camera and tracker values
		init_camera_tracker_values(camera_data, camera_obj, camera_tracker, anim)
		
		# Load keyframes from keyframe sets
		for kfs in anim.iter('KeyframeSet'):
			process_animation(camera_data, camera_obj, camera_tracker, anim, kfs)
		
		# Link camera and tracker to event's collection
		cam_collection.objects.link(camera_obj)
		cam_collection.objects.link(camera_tracker)