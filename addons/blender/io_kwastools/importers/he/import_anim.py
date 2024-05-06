import bpy
from bpy.props import *
from bpy_extras.io_utils import ImportHelper

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

class ImportHedgehogXMLAnim(bpy.types.Operator, ImportHelper):
	bl_idname = "custom_import.hedgehog_xml_anim"
	bl_label = "Import"
	bl_options = {'PRESET', 'UNDO'}
	filename_ext = ".xml"
	filter_glob: StringProperty(default="*.xml", options={'HIDDEN'},)
	filepath: StringProperty(subtype='FILE_PATH',)
	files: CollectionProperty(type=bpy.types.PropertyGroup)
	
	def draw(self, context):
		pass
		
	def execute(self, context):
		load(self.filepath)
		return {'FINISHED'}
	