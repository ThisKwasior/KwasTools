bl_info = {
	"name": "KwasTools for Blender 3.0/4.0",
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
from bpy.app.handlers import persistent

from io_kwastools.importers.he import import_anim as he_import
from io_kwastools.exporters.he import export_anim as he_export
from io_kwastools.common import he_common as he_common

class KwasToolsImportMenu(bpy.types.Menu):
	bl_label = "KwasTools"
	bl_idname = "KWASTOOLS_MT_IMPORT_MENU"

	def draw(self, context):
		layout = self.layout
		layout.operator(he_import.ImportHedgehogXMLAnim.bl_idname, text="Hedgehog Engine Anim XML (.xml)")
		
class KwasToolsExportMenu(bpy.types.Menu):
	bl_label = "KwasTools"
	bl_idname = "KWASTOOLS_MT_EXPORT_MENU"

	def draw(self, context):
		layout = self.layout
		layout.operator(he_export.ExportHedgehogUvAnimV3.bl_idname, text="Hedgehog Engine Uv-Anim V3 XML (.xml)")
		layout.operator(he_export.ExportHedgehogCamAnim.bl_idname, text="Hedgehog Engine Cam-Anim XML (.xml)")

classes = (
	KwasToolsImportMenu,
	KwasToolsExportMenu,
	he_import.ImportHedgehogXMLAnim,
	he_export.ExportHedgehogUvAnimV3,
	he_export.ExportHedgehogCamAnim,
	he_common.UVAnimatorCreateOperator,
	he_common.CamAnimCreateSetupOperator
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
	
	bpy.types.VIEW3D_MT_object.append(he_common.UVAnimatorCreateOperator.menu_func)
	bpy.types.VIEW3D_MT_object.append(he_common.CamAnimCreateSetupOperator.menu_func)

def unregister():
	bpy.types.TOPBAR_MT_file_import.remove(menu_import)
	bpy.types.TOPBAR_MT_file_export.remove(menu_export)
	
	bpy.types.VIEW3D_MT_object.append(he_common.UVAnimatorCreateOperator.menu_func)
	bpy.types.VIEW3D_MT_object.append(he_common.CamAnimCreateSetupOperator.menu_func)
	
	for cls in classes:
		bpy.utils.unregister_class(cls)
		
if __name__ == "__main__":
	register()