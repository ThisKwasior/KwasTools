import bpy as bpy
import xml.etree.ElementTree as ET

def bl_get_current_aspect_ratio():
	brs = bpy.context.scene.render
	return brs.resolution_x/brs.resolution_y
	
def bl_get_current_scene_fps():
	return bpy.context.scene.render.fps
	
def bl_set_scene_settings(fps, frame_start, frame_end):
	bpy.context.scene.render.fps = int(fps)
	bpy.context.scene.frame_start = frame_start
	bpy.context.scene.frame_end = frame_end

# Creates collection and links it to scene
def bl_create_collection(name):
	collection = bpy.data.collections.new(name)
	bpy.context.scene.collection.children.link(collection)
	
	return collection

# Returns material by name
# otherwise type is None 
def bl_get_material_by_name(mat_name):
	for mat in iter(bpy.data.materials):
		if mat.name == mat_name:
			return mat

	return None

# Returns node_tree of the material requested by name
# otherwise type is None 
def bl_get_material_node_tree(mat_name):
	mat = bl_get_material_by_name(mat_name)
	return mat.node_tree

# Returns an array of nodes with requested type (like BSDF_PRINCIPLED, MAPPING or TEX_IMAGE)
# If none are found, returns None
def bl_get_nodes_by_type(nodes, type):
	type_nodes = []
	
	for node in iter(nodes):
		if node.type == type:
			type_nodes.append(node)
			
	if len(type_nodes) == 0:
		return None
    
	return type_nodes

# Returns the TEX_IMAGE node with specified image name
# Otherwise None
def bl_get_tex_image_node(nodes, image_name):
    for node in iter(nodes):
        if node.type == "TEX_IMAGE":
            # Name of the image as opposed to the label of the node.
            # Node labels from imported modelfbx files are shown
            # in the node editor but are empty in `node.label`.
            # TODO: Don't forget to mention it on github wiki page
            if node.image.name == image_name:
                return node
    
    return None

# LEGACY, REPLACED BY he_create_uv_animator_nodegroup()
# Attaches MAPPING and TEX_COORD nodes to specified TEX_IMAGE node.
# It won't attach anything if the link exists at 'Vector' 
def bl_tex_image_attach_mapping_texcoord(node_tree, image_node):
    # Check if the image node already has a node
    # If so - return, we're doing nothing here
    if image_node.inputs['Vector'].is_linked == True:
        return
    
    # Create nodes
    mapping_node = node_tree.nodes.new('ShaderNodeMapping')
    tex_node = node_tree.nodes.new('ShaderNodeTexCoord')
    
    # Set position of new nodes to be on the left of the image node
    mapping_node.location = (image_node.location.x - mapping_node.width - 40), image_node.location.y
    tex_node.location = (mapping_node.location.x - tex_node.width - 40), mapping_node.location.y
    
    # Link nodes
    # Tex['UV'] <-> Mapping['Vector'] <-> Image['Vector']
    node_tree.links.new(tex_node.outputs['UV'], mapping_node.inputs['Vector'])
    node_tree.links.new(mapping_node.outputs['Vector'], image_node.inputs['Vector'])

# Returns an array of keyframes in pairs [index, value]
# from fcurves of an object and specified data_path (like location)
# and array_id (like 0, which would be location.x)
def bl_get_fcurve_data_by_path_id(fcurves, data_path, array_id):
	if fcurves is None:
		# Nothing to be done, return
		return None

	for fc in iter(fcurves):
		if data_path == fc.data_path and array_id == fc.array_index:
			data = []
			
			for kf in iter(fc.keyframe_points):
				kfdata = [int(kf.co.x), float(kf.co.y)]
				data.append(kfdata)
			
			return data

	# We somehow got here
	return None

# Changes all available keyframes in the fcurves to linear interp
def bl_set_fcurve_to_linear(fcurves):
	for fcurve in fcurves:
		for kf in fcurve.keyframe_points:
			kf.interpolation = 'LINEAR'

# Removes fcurve in fcurves by data_path
def bl_remove_fcurve_by_data_path(fcurves, data_path):
    fc = fcurves.find(data_path)
    if bool(fc):
        fcurves.remove(fc)

# Makes the resulting XML not be one-liner
def save_pretty_xml(file_path, et_element):
	tree = ET.ElementTree(et_element)
	ET.indent(tree, space="\t", level=0)
	tree.write(file_path)