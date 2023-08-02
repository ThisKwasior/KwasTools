import bpy as bpy
from enum import Enum

class UV_Anim_Types(Enum):
	u		= 0
	v		= 1
	unk1	= 2
	unk2	= 3

def process_animation(modifier, anim, kfs):
	anim_type = UV_Anim_Types(int(kfs.get('type')))
	start = int(anim.get('start_frame'))
	keyframes = kfs.findall('Keyframe')
	
	match anim_type:
		case UV_Anim_Types.u:
			for kf in iter(keyframes):
				modifier.offset[0] = float(kf.get('value'))
				modifier.keyframe_insert('offset', index = 0, frame = int(kf.get('index')))
		case UV_Anim_Types.v:
			for kf in iter(keyframes):
				modifier.offset[1] = -float(kf.get('value'))
				modifier.keyframe_insert('offset', index = 1, frame = int(kf.get('index')))

def load(filename, root):
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
			
			
			
			
			