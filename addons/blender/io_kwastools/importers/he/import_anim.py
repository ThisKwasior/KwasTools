from pathlib import Path
import xml.etree.ElementTree as ET
import io_kwastools.importers.he.uv_anim as uv
import io_kwastools.importers.he.cam_anim as cam

def load(filepath):
	tree = ET.parse(filepath)
	root = tree.getroot()
	
	filename = Path(filepath).stem
	
	if root.tag == 'UVAnimation':
		uv.load(filename, root)
	elif root.tag == 'CAMAnimation':
		cam.load(filename, root)

	
	pass
	
	