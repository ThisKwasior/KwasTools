bl_info = {
	"name": "KwasTools for Blender",
	"author": "Kwasior",
	"version": (0, 1, 0),
	"blender": (3, 0, 0),
	"location": "File > Import-Export",
	"description": "Collection of scripts for import/export of files from various games in XML format",
	"warning": "",
	"support": "COMMUNITY",
	"category": "Import-Export",
}

import bpy
import sys
import os
from bpy.props import *
from bpy_extras.io_utils import ImportHelper

from io_kwastools.importers.he import import_anim
from io_kwastools.exporters.he import export_anim as he_export

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
		import_anim.load(self.filepath)
		return {'FINISHED'}

class KwasToolsImportMenu(bpy.types.Menu):
	bl_label = "KwasTools"
	bl_idname = "KWASTOOLS_MT_IMPORT_MENU"

	def draw(self, context):
		layout = self.layout
		layout.operator(ImportHedgehogXMLAnim.bl_idname, text="Hedgehog Engine Anim XML (.xml)")
		
class KwasToolsExportMenu(bpy.types.Menu):
	bl_label = "KwasTools"
	bl_idname = "KWASTOOLS_MT_EXPORT_MENU"

	def draw(self, context):
		layout = self.layout
		#layout.operator(he_export.ExportHedgehogUvAnim.bl_idname, text="Hedgehog Engine Uv-Anim XML (.xml)")
		layout.operator(he_export.ExportHedgehogCamAnim.bl_idname, text="Hedgehog Engine Cam-Anim XML (.xml)")

classes = (
	ImportHedgehogXMLAnim,
	KwasToolsImportMenu,
	KwasToolsExportMenu,
	he_export.ExportHedgehogUvAnim,
	he_export.ExportHedgehogCamAnim,
)

def menu_import(self, context):
	self.layout.menu(KwasToolsImportMenu.bl_idname)
	
def menu_export(self, context):
	self.layout.menu(KwasToolsExportMenu.bl_idname)

def register():
	for cls in classes:
		bpy.utils.register_class(cls)
	
	bpy.types.TOPBAR_MT_file_import.append(menu_import)
	bpy.types.TOPBAR_MT_file_export.append(menu_export)

def unregister():
	bpy.types.TOPBAR_MT_file_import.remove(menu_import)
	bpy.types.TOPBAR_MT_file_export.append(menu_export)
	
	for cls in classes:
		bpy.utils.unregister_class(cls)
		
if __name__ == "__main__":
	register()