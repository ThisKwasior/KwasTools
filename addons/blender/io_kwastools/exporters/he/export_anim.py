import bpy
#import sys
#import os
from bpy.props import *
from bpy_extras.io_utils import ImportHelper

from pathlib import Path
import xml.etree.ElementTree as ET

import io_kwastools.exporters.he.uv_anim as uv
import io_kwastools.exporters.he.cam_anim as cam

class ExportHedgehogUvAnimV3(bpy.types.Operator, ImportHelper):
	bl_idname = "custom_export.hedgehog_uv_anim_v3"
	bl_label = "Export"
	bl_options = {'PRESET', 'UNDO'}
	filename_ext = ".xml"
	filter_glob: StringProperty(default="*.xml", options={'HIDDEN'},)
	filepath: StringProperty(subtype='FILE_PATH',)
	files: CollectionProperty(type=bpy.types.PropertyGroup)
	
	def draw(self, context):
		pass
		
	def execute(self, context):
		uv.save_v3(self.filepath)
		return {'FINISHED'}

class ExportHedgehogCamAnim(bpy.types.Operator, ImportHelper):
	bl_idname = "custom_export.hedgehog_cam_anim"
	bl_label = "Export"
	bl_options = {'PRESET', 'UNDO'}
	filename_ext = ".xml"
	filter_glob: StringProperty(default="*.xml", options={'HIDDEN'},)
	filepath: StringProperty(subtype='FILE_PATH',)
	files: CollectionProperty(type=bpy.types.PropertyGroup)
	
	def draw(self, context):
		pass
		
	def execute(self, context):
		cam.save(self.filepath)
		return {'FINISHED'}
	