import bpy as bpy
from enum import Enum
import math

from io_kwastools.common.common import *

class CamAnimTypes(Enum):
	cam_pos_x	= 0
	cam_pos_z	= 1
	cam_pos_y	= 2
	cam_rot_x	= 3
	cam_rot_z	= 4
	cam_rot_y	= 5
	aim_pos_x	= 6
	aim_pos_z	= 7
	aim_pos_y	= 8
	twist		= 9
	z_near		= 10
	z_far		= 11
	fov			= 12
	aspect		= 13
	
class UVAnimTypes(Enum):
	pos_x		= 0
	pos_y		= 1
	rot			= 2
	scale_x		= 3
	scale_y		= 4

# I hate python
class UVAnimatorCreateOperator(bpy.types.Operator):
	bl_idname = "object.kwastools_uv_animator_create"
	bl_label = "Create UV Animator Node Group"

	def execute(self, context):
		# TODO: Check if we have the node group already, otherwise don't add a new one
		he_create_uv_animator_nodegroup()
		return {'FINISHED'}
		
	def menu_func(self, context):
		self.layout.operator(self.bl_idname, text=self.bl_label)
		
# I hate python part 2
class CamAnimCreateSetupOperator(bpy.types.Operator):
	bl_idname = "object.kwastools_camanim_create_setup"
	bl_label = "Create Camera and Tracker setup for cam-anim"

	# TODO: Make it SHORTER JEEZ IT'S LITERALLY COPY AND PASTE FROM importers.cam_anim
	def execute(self, context):
		scn = bpy.context.scene
		
		# Check if we even need to create the collection
		cam_collection = None
		cols = [col for col in bpy.data.collections if col.name=='KwasTools Cameras']
		
		if(len(cols) == 0):
			cam_collection = bl_create_collection("KwasTools Cameras")
		else:
			cam_collection = cols[0]
	
		# Create new camera and its tracker
		camera_data = bpy.data.cameras.new("KwasTools Camera Data")
		camera_obj = bpy.data.objects.new("KwasTools Camera", camera_data)
		camera_tracker = bpy.data.objects.new("KwasTools Tracker", None)
		
		# Create the marker on the timeline for current camera
		marker = scn.timeline_markers.new("KwasTools Camera", frame = 0)
		marker.camera = camera_obj
		
		# Link camera and tracker to event's collection
		cam_collection.objects.link(camera_obj)
		cam_collection.objects.link(camera_tracker)
		
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
		
		# Set location for both
		cursor3d_pos = bpy.context.scene.cursor.location
		camera_obj.location = [cursor3d_pos.x + 3, cursor3d_pos.y + 3, cursor3d_pos.z + 3]
		camera_tracker.location = cursor3d_pos
		
		return {'FINISHED'}
		
	def menu_func(self, context):
		self.layout.operator(self.bl_idname, text=self.bl_label)


# AMAZING work done by @ik-01 on figuring out the formula here! Thank you so much
# 2*atan(tan(fov/2)*aspect_ratio)
def he_fov_to_blender_angle(fov, aspect_ratio):
	return 2*math.atan(math.tan(fov/2)*aspect_ratio)
	
def blender_angle_to_he_fov(angle, aspect_ratio):
	return 2*math.atan(math.tan(angle/2)/aspect_ratio)
	
# Counting the range of frames for an animation
# Universal across all *-anim types
def he_get_frames_range(xml_root):
	# Don't ask
	range = [4294967295, -4294967295]
	
	# Get the range
	for anim in xml_root.iter('Animation'):
		start = int(anim.get('start_frame')) 
		end = int(anim.get('end_frame'))

		if range[0] > start:
			range[0] = start
			
		if range[1] < end: 
			range[1] = end

	return range

# Prepares the root of Animation XML
def he_get_xml_he_anim_root(type, version):
	anim = ET.Element(str(type))
	anim.set('root_node_type', str(version))
	return anim

# Creates the keyframe set xml element
def he_create_keyframe_set_xml_root(anim, type):
	kfs = ET.SubElement(anim, 'KeyframeSet')
	kfs.set('type', str(int(type)))
	kfs.set('flag2', str(0))
	kfs.set('interpolation', str(0))
	kfs.set('flag4', str(0))
	return kfs

# 'data' is from bl_get_fcurve_data_by_path_id()
def he_append_xml_keyframes(kfs_xml, data):
	for kf in iter(data):
		kf_elem = ET.SubElement(kfs_xml, 'Keyframe')
		kf_elem.set('index', str(int(kf[0])))
		kf_elem.set('value', str(float(kf[1])))

# Creates the entirety of UVAnimator NodeGroup
# Returns the created node group
def he_create_uv_animator_nodegroup():
    nodegroup = bpy.data.node_groups.new('UVAnimatorGroup', type='ShaderNodeTree')
    nginput = nodegroup.nodes.new('NodeGroupInput')
    ngoutput = nodegroup.nodes.new('NodeGroupOutput')
    nginput.location = (-677, 92)
    ngoutput.location = (728, 5)
    
    # Create input and output sockets
    nodegroup.interface.new_socket('Pos X', in_out='INPUT', socket_type='NodeSocketFloat')
    nodegroup.interface.new_socket('Pos Y', in_out='INPUT', socket_type='NodeSocketFloat')
    nodegroup.interface.new_socket('Rotation', description='In radians', in_out='INPUT', socket_type='NodeSocketFloat')
    nodegroup.interface.new_socket('Scale X', in_out='INPUT', socket_type='NodeSocketFloat')
    nodegroup.interface.new_socket('Scale Y', in_out='INPUT', socket_type='NodeSocketFloat')
    nodegroup.interface.new_socket('Output Vector', in_out='OUTPUT', socket_type='NodeSocketVector')
    
    nodegroup.interface.items_tree['Scale X'].default_value = 1.0
    nodegroup.interface.items_tree['Scale Y'].default_value = 1.0
    
    # Output Vector that mixes everything together
    finalvec = nodegroup.nodes.new('ShaderNodeMapping')
    finalvec.label = 'Final Vector'
    finalvec.location = (516, 90)
    nodegroup.links.new(finalvec.outputs['Vector'], ngoutput.inputs['Output Vector'])
    
    # Rotation node setup
    texcoord = nodegroup.nodes.new('ShaderNodeTexCoord')
    rotresult = nodegroup.nodes.new('ShaderNodeVectorRotate')
    
    texcoord.label = 'Tex Coord'
    rotresult.label = 'Resulting Rotation'
    
    texcoord.location = (118, 344)
    rotresult.location = (278, 306)
    
    rotresult.inputs['Center'].default_value = (0.5, 0.5, 0.5)
    rotresult.rotation_type = 'Z_AXIS'
    
    nodegroup.links.new(texcoord.outputs['UV'], rotresult.inputs['Vector'])
    nodegroup.links.new(nginput.outputs['Rotation'], rotresult.inputs['Angle'])
    nodegroup.links.new(rotresult.outputs['Vector'], finalvec.inputs['Vector'])
    
    # Scale node setup
    scaleresult = nodegroup.nodes.new('ShaderNodeCombineXYZ')
    
    scaleresult.label = 'Resulting Scale'
    scaleresult.location = (107, -360)
    
    nodegroup.links.new(nginput.outputs['Scale X'], scaleresult.inputs['X'])
    nodegroup.links.new(nginput.outputs['Scale Y'], scaleresult.inputs['Y'])
    nodegroup.links.new(scaleresult.outputs['Vector'], finalvec.inputs['Scale'])
    
    # Position node setup
    posytimesminusone = nodegroup.nodes.new('ShaderNodeMath')
    oneminusscaley = nodegroup.nodes.new('ShaderNodeMath')
    posyresult = nodegroup.nodes.new('ShaderNodeMath')
    posxyresult = nodegroup.nodes.new('ShaderNodeCombineXYZ')
    
    posytimesminusone.label = 'PosY times -1'
    oneminusscaley.label = '1 minus ScaleY' 
    posyresult.label = 'Resulting PosY'
    posxyresult.label = 'Resulting PosXY'
    
    posytimesminusone.location = (-62, -17)
    oneminusscaley.location = (-55, -185)
    posyresult.location = (108, -50)
    posxyresult.location = (278, 58)
    
    posytimesminusone.operation = 'MULTIPLY'
    posytimesminusone.inputs[0].default_value = -1.0
    oneminusscaley.operation = 'SUBTRACT'
    oneminusscaley.inputs[0].default_value = 1.0
    posyresult.operation = 'ADD'
    
    nodegroup.links.new(nginput.outputs['Pos X'], posxyresult.inputs['X'])
    nodegroup.links.new(nginput.outputs['Pos Y'], posytimesminusone.inputs[1])
    nodegroup.links.new(nginput.outputs['Scale Y'], oneminusscaley.inputs[1])
    nodegroup.links.new(posytimesminusone.outputs['Value'], posyresult.inputs[0])
    nodegroup.links.new(oneminusscaley.outputs['Value'], posyresult.inputs[1])
    nodegroup.links.new(posyresult.outputs['Value'], posxyresult.inputs['Y'])
    nodegroup.links.new(posxyresult.outputs['Vector'], finalvec.inputs['Location'])
    
    # Return the node group
    return nodegroup
	
# Resets the values of the UVAnimator NodeGroup
def he_reset_uv_animator_nodegroup(nodegroup):
	nodegroup.inputs['Pos X'].default_value = 0
	nodegroup.inputs['Pos Y'].default_value = 0
	nodegroup.inputs['Rotation'].default_value = 0
	nodegroup.inputs['Scale X'].default_value = 1
	nodegroup.inputs['Scale Y'].default_value = 1
	
def he_remove_animation_data_from_uv_animator(nodegroup):
	# Clear the data
	# animation_data_clear() clears EVERYTHING, because it's all in one action
	# nodegroup.id_data.animation_data_clear()
	if bool(nodegroup.id_data.animation_data.action):
		fcurves = nodegroup.id_data.animation_data.action.fcurves
		bl_remove_fcurve_by_data_path(fcurves, str('nodes["' + nodegroup.name + '"].inputs[0].default_value'))
		bl_remove_fcurve_by_data_path(fcurves, str('nodes["' + nodegroup.name + '"].inputs[1].default_value'))
		bl_remove_fcurve_by_data_path(fcurves, str('nodes["' + nodegroup.name + '"].inputs[2].default_value'))
		bl_remove_fcurve_by_data_path(fcurves, str('nodes["' + nodegroup.name + '"].inputs[3].default_value'))
		bl_remove_fcurve_by_data_path(fcurves, str('nodes["' + nodegroup.name + '"].inputs[4].default_value'))
	
	# Reset values to the default ones
	he_reset_uv_animator_nodegroup(nodegroup)