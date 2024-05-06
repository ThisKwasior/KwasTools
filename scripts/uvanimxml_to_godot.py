# Requires python 3.10
# Usage:
# 	python uvanimxml_to_godot.py uvanim.xml

import sys
from pathlib import Path
import xml.etree.ElementTree as ET
from enum import Enum
from pathlib import Path

class UV_Anim_Types(Enum):
	u		= 0
	v		= 1
	unk1	= 2
	unk2	= 3

# Calculates steps for every range of keyframes
def calc_step(values, idxs):
	steps=[]
	for x in range(len(values)-1):
		dif=values[x+1]-values[x]
		kf_range=idxs[x+1]-idxs[x]
		steps.append(float(dif/kf_range))
	return steps

# Calculates a linear approximation of a value at given pos (keyframe)
def lerp_at_pos(pos, values, ids, steps):
	id = 0
	pos_case = 0;
	
	# Get the ID for the array of values, ids and steps
	for i in range(len(ids)):
		if ids[i] >= pos:
			id = i-1
			break

	# Keyframe position in individual range
	pos_case = pos-ids[id]
	
	# Calculate value
	value = values[id] + (steps[id]*pos_case)
	
	return value

argc = len(sys.argv)

if argc == 1:
	print("Usage:", sys.argv[0], "<Kwastools UVAnim XML>")
	sys.exit()

tree = ET.parse(sys.argv[1])
root = tree.getroot()

if root.tag != 'UVAnimation':
	print("This is not a valid UVAnim XML")
	sys.exit()
	
filename = Path(sys.argv[1]).stem

uv_x = []
id_x = []
step_x = []
uv_y = []
id_y = []
step_y = []

fps = 0

for anim in root.iter('Animation'):
	fps = float(anim.get('frame_rate'))

	for kfs in anim.iter('KeyframeSet'):
		anim_type = UV_Anim_Types(int(kfs.get('type')))
		keyframes = kfs.findall('Keyframe')
		
		match anim_type:
			case UV_Anim_Types.u:
				for kf in iter(keyframes):
					uv_x.append(float(kf.get('value')))
					idx = int(kf.get('index'))
					id_x.append(idx)
			case UV_Anim_Types.v:
				for kf in iter(keyframes):
					uv_y.append(float(kf.get('value')))
					idx = int(kf.get('index'))
					id_y.append(idx)
					
step_x = calc_step(uv_x, id_x)
step_y = calc_step(uv_y, id_y)

ids_combined = id_x + id_y
ids_combined.sort()
ids_nodups=list(dict.fromkeys(ids_combined))

# val x, val y, time
keyframes=[]

for i in iter(ids_nodups):
	kf = [lerp_at_pos(i, uv_x, id_x, step_x), lerp_at_pos(i, uv_y, id_y, step_y), float(i/fps)]
	keyframes.append(kf)

filename_godot=filename+".godot"

print("Saving to", filename_godot)

godotf = open(filename_godot, "w")

# Writing times
godotf.write('"times": PoolRealArray(')

for i in range(len(keyframes)-1):
	godotf.write(" " + str(keyframes[i][2]) + " ,")
	
godotf.write(" " + str(keyframes[len(keyframes)-1][2]) + "),\n")

# Writing transitions
godotf.write('"transitions": PoolRealArray(')

for i in range(len(keyframes)):
	godotf.write(" 1,")
	
godotf.write(" 1 ),\n")

# Writing update
godotf.write('"update": 0,\n')

# Writing values
godotf.write('"values": [ ')

for i in range(len(keyframes)-1):
	godotf.write("Vector3( " + str(keyframes[i][0]) + ", " + str(keyframes[i][1]) + ", 0 ) , ")
	
godotf.write(" Vector3( " + str(keyframes[len(keyframes)-1][0]) + ", " + str(keyframes[len(keyframes)-1][1]) + ", 0 ) ]")

godotf.close()