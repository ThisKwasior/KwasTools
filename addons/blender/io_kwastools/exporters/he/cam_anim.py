import bpy
import sys

import xml.etree.ElementTree as ET

def getXMLCamRoot():
    camanim = ET.Element('CAMAnimation')
    camanim.set('root_node_type', '2')
    return camanim

# cam-anim keyframe sets
# camera 'location' is 0, 1, 2 (position)
# camera 'rotation_euler' is 3, 4, 5 (rotation)
# target 'location' is 6, 7, 8 (position)
# target 'rotation_euler' is 9 (only Z rotation)
# camera 'clip_start' is 10 (z_near)
# camera 'clip_end' is 11 (z_far)
# camera 'angle_y' is 12 (fov)
# camera 'res_x/res_y' is 13 (aspect ratio)
def getTypeByDataPath(data_path, index, is_target):
    if data_path == 'location':
        if is_target:
            if index == 1: # Y
                return 8 # Z
            if index == 2: # Z
                return 7 # Y
                
            return 6 # X
        else:
            if index == 1: # Y
                return 2 # Z
            if index == 2: # Z
                return 1 # Y
                
            return 0 # X
    elif data_path == 'rotation_euler':
        if is_target and index == 2:
            return 9
        #elif not is_target: # dunno if rotation even works
    elif data_path == 'clip_start':
        return 10
    elif data_path == 'clip_end':
        return 11
    elif data_path == 'lens':
        return 12
    elif data_path == 'aspect':
        return 13
    
    return -1

def appendXMLKeyframes(kf_points, kfs_xml, type, obj_data):
    for kf in iter(kf_points):
        kf_elem = ET.SubElement(kfs_xml, 'Keyframe')
        kf_elem.set('index', str(int(kf.co.x)))
        
        if type == 2 or type == 8:
            kf_elem.set('value', str(float(-kf.co.y)))
        elif type == 12:
            # I have no idea how to get angle_y any other way
            bpy.context.scene.frame_set(frame=int(kf.co.x))
            aspect_ratio = bpy.context.scene.render.resolution_x/bpy.context.scene.render.resolution_y
            kf_elem.set('value', str(float(obj_data.angle_y/aspect_ratio)))
        else:
            kf_elem.set('value', str(float(kf.co.y)))
        
def createKeyframeSetXMLElem(anim_elem, cam_type):
    kfset_elem = ET.SubElement(anim_elem, 'KeyframeSet')
    kfset_elem.set('type', str(cam_type))
    kfset_elem.set('flag2', str(0))
    kfset_elem.set('interpolation', str(0))
    kfset_elem.set('flag4', str(0))
    
    return kfset_elem

def prepXMLKeyframeSets(camera, target, anim_elem):
    fcurves = 0
    fcurves_data = 0
    fcurves_target = 0

    if camera.animation_data is not None:
        fcurves = camera.animation_data.action.fcurves
    if camera.data.animation_data is not None:
        fcurves_data = camera.data.animation_data.action.fcurves
    
    if target != 0:
        if target.animation_data is not None:
            fcurves_target = target.animation_data.action.fcurves
    
    # Keyframes from the camera animation
    if fcurves != 0:
        for fc in iter(fcurves):
            path_name = fc.data_path
            index = fc.array_index
            type = getTypeByDataPath(path_name, index, False)
            print(path_name, index, type)
            if type != -1:
                kfset_elem = createKeyframeSetXMLElem(anim_elem, type)
                appendXMLKeyframes(fc.keyframe_points, kfset_elem, type, camera)

    # Keyframes from the target animation
    if fcurves_target != 0:    
        for fc in iter(fcurves_target):
            path_name = fc.data_path
            index = fc.array_index
            type = getTypeByDataPath(path_name, index, True)
            print(path_name, index, type)
            if type != -1:
                kfset_elem = createKeyframeSetXMLElem(anim_elem, type)
                appendXMLKeyframes(fc.keyframe_points, kfset_elem, type, target)

    # Keyframes from camera properties
    if fcurves_data != 0:
        for fc in iter(fcurves_data):
            path_name = fc.data_path
            index = fc.array_index
            type = getTypeByDataPath(path_name, index, False)
            print(path_name, index, type)
            if type != -1:
                kfset_elem = createKeyframeSetXMLElem(anim_elem, type)
                appendXMLKeyframes(fc.keyframe_points, kfset_elem, type, camera.data)

def prepAndSaveCamAnim(filepath):
    cam_xml = getXMLCamRoot()

    render =  bpy.context.scene.render
    objects = bpy.context.scene.objects

    # Set the currect scene frame to 0
    bpy.context.scene.frame_set(frame=0) 

    # Common for Animation
    flag1 = 1
    flag2 = 0
    flag3 = 0
    flag4 = 0
    frame_rate = bpy.context.scene.render.fps
    aspect_ratio = render.resolution_x/render.resolution_y

    for object in iter(objects):
        if(object.type == "CAMERA"):
            if object.animation_data is not None:
                anim_elem = ET.SubElement(cam_xml, 'Animation')
                
                trg = 0 # target of TRACK_TO constraint
                
                name = object.name
                start_frame = object.animation_data.action.frame_range.x
                end_frame = object.animation_data.action.frame_range.y
                cam_pos_x = object.location.x
                cam_pos_y = object.location.y
                cam_pos_z = object.location.z
                #cam_rot_x = object.rotation_euler.x
                #cam_rot_y = object.rotation_euler.y
                #cam_rot_z = object.rotation_euler.z
                cam_rot_x = 0.0
                cam_rot_y = 0.0
                cam_rot_z = 0.0
                aim_pos_x = 0.0
                aim_pos_y = 0.0
                aim_pos_z = 0.0
                aim_z_rotation = 0.0
                z_near = object.data.clip_start
                z_far = object.data.clip_end
                fov = object.data.angle_y
                
                for cons in iter(object.constraints):
                    if(cons.type == "TRACK_TO"):
                        trg = cons.target
                        aim_pos_x = trg.location.x
                        aim_pos_y = trg.location.y
                        aim_pos_z = trg.location.z
                        aim_z_rotation = trg.rotation_euler.z
                        
                print("Target:", trg)
                        
                print("Name:", name)
                print("Framerate:", frame_rate)
                print("Frame range:", start_frame, end_frame)
                print("Flags:", flag1, flag2, flag3, flag4)
                print("Cam pos:", cam_pos_x, cam_pos_y, cam_pos_z)
                print("Cam rot:", cam_rot_x, cam_rot_y, cam_rot_z)
                print("Aim pos/rotz:", aim_pos_x, aim_pos_y, aim_pos_z, aim_z_rotation)
                print("near/far/fov/ar:", z_near, z_far, fov, aspect_ratio)
                
                anim_elem.set('name', str(name))
                anim_elem.set('flag1', str(int(flag1)))
                anim_elem.set('flag2', str(int(flag2)))
                anim_elem.set('flag3', str(int(flag3)))
                anim_elem.set('flag4', str(int(flag4)))
                anim_elem.set('frame_rate', str(int(frame_rate)))
                anim_elem.set('start_frame', str(int(start_frame)))
                anim_elem.set('end_frame', str(int(end_frame)))
                anim_elem.set('cam_pos_x', str(float(cam_pos_x)))
                anim_elem.set('cam_pos_y', str(float(cam_pos_y)))
                anim_elem.set('cam_pos_z', str(float(cam_pos_z)))
                anim_elem.set('cam_rot_x', str(float(cam_rot_x)))
                anim_elem.set('cam_rot_y', str(float(cam_rot_y)))
                anim_elem.set('cam_rot_z', str(float(cam_rot_z)))
                anim_elem.set('aim_pos_x', str(float(aim_pos_x)))
                anim_elem.set('aim_pos_y', str(float(aim_pos_y)))
                anim_elem.set('aim_pos_z', str(float(aim_pos_z)))
                anim_elem.set('aim_z_rotation', str(float(aim_z_rotation)))
                anim_elem.set('z_near', str(float(z_near)))
                anim_elem.set('z_far', str(float(z_far)))
                anim_elem.set('fov', str(float(fov)))
                anim_elem.set('aspect_ratio', str(float(aspect_ratio)))

                prepXMLKeyframeSets(object, trg, anim_elem)
         
    tree = ET.ElementTree(cam_xml)
    tree.write(filepath)

def save(filepath):
    print("cam_anim.save()")
    prepAndSaveCamAnim(filepath)