#pragma once

#include <string.h>

#include <kwaslib/core/data/text/sexml.h>
#include <kwaslib/cri/acb/acb_command.h>
#include <kwaslib/cri/acb/acb_command_opcodes.h>
#include <kwaslib/cri/acb/acb_command_readers.h>

/*
    Defines
*/
#define XML_ACBCMD_NAME         (const char*)"ACBCMD"
#define XML_ACBCMD_NAME_SIZE    6

/*
	Unpacking
*/
inline static void cri_acb_cmd_to_xml(ACB_COMMAND acbcmd, SEXML_ELEMENT* root);

/*
	Packing
*/
inline static ACB_COMMAND cri_acb_cmd_xml_to_acbcmd(SEXML_ELEMENT* acbcmd_xml);

/*
    Misc
*/
inline static void cri_acb_cmd_print_acbcmd(ACB_COMMAND acbcmd);

/*
    Implementation
*/

/*
	Unpacking
*/
inline static void cri_acb_cmd_to_xml(ACB_COMMAND acbcmd, SEXML_ELEMENT* root)
{
    const uint32_t acbcmd_count = acb_cmd_get_opcode_count(acbcmd);
    
    SEXML_ELEMENT* acbcmd_node = sexml_append_element(root, XML_ACBCMD_NAME);
    
    for(uint32_t i = 0; i != acbcmd_count; ++i)
    {
        ACB_COMMAND_OPCODE* op = acb_cmd_get_opcode_by_id(acbcmd, i);
        SEXML_ELEMENT* cmd_xml = NULL;
        
        /* First we try to parse known opcodes to more familiar structure */
        switch(op->op)
        {
            /* 0 */
            case ACB_CMD_OP_NOP:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_NOP);
                continue;
            /* 33 - u8 */
            case ACB_CMD_OP_MUTE:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_MUTE);
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u8);
                continue;
            /* 65 - u32 */
            case ACB_CMD_OP_CATEGORY:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_CATEGORY);
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u32);
                continue;
            /* 68 - float */
            case ACB_CMD_OP_POS_3D_DISTANCE_MIN:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_POS_3D_DISTANCE_MIN);
                sexml_append_attribute_double(cmd_xml, "value", op->data.f32, 8);
                continue;
            /* 69 - float */
            case ACB_CMD_OP_POS_3D_DISTANCE_MAX:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_POS_3D_DISTANCE_MAX);
                sexml_append_attribute_double(cmd_xml, "value", op->data.f32, 8);
                continue;
            /* 87 - u16 */
            case ACB_CMD_OP_VOLUME_GAIN_RESOLUTION100:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_VOLUME_GAIN_RESOLUTION100);
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u16);
                continue;
            /* 146 - float*/
            case ACB_CMD_OP_VOLUME_CONTROL:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_VOLUME_CONTROL);
                sexml_append_attribute_double(cmd_xml, "value", op->data.f32, 8);
                continue;
            /* 2000 */
            case ACB_CMD_OP_NOTE_ON:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_NOTE_ON);
                uint16_t type, index;
                acb_cmd_read_note_on(op, &type, &index);
                
                switch(type)
                {
                    case ACB_CMD_NOTE_ON_TYPE_SYNTH:
                        sexml_append_attribute(cmd_xml, "type", ACB_CMD_NOTE_ON_TYPE_SYNTH_NAME);
                        break;
                    case ACB_CMD_NOTE_ON_TYPE_SEQUENCE:
                        sexml_append_attribute(cmd_xml, "type", ACB_CMD_NOTE_ON_TYPE_SEQUENCE_NAME);
                        break;
                    default:
                        sexml_append_attribute(cmd_xml, "type", ACB_CMD_NOTE_ON_TYPE_UNK_NAME);
                }
                
                sexml_append_attribute_uint(cmd_xml, "index", index);
                continue;
            /* 2001 - u32 */
            case ACB_CMD_OP_DELAY:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_DELAY);
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u32);
                continue;
            /* 4000 */
            case ACB_CMD_OP_SEQUENCE_END:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_SEQUENCE_END);
                continue;
            /* 4050 */
            case ACB_CMD_OP_BLOCK_END:
                cmd_xml = sexml_append_element(acbcmd_node, ACB_CMD_NAME_BLOCK_END);
                continue;
        }
        
        /* It's an unknown opcode, do the usual */
        cmd_xml = sexml_append_element(acbcmd_node, "cmd");
		sexml_append_attribute_uint(cmd_xml, "op", op->op);
		sexml_append_attribute(cmd_xml, "type", acb_cmd_type_to_str(op->type));
        
        switch(op->type)
        {
            case ACB_CMD_OPCODE_TYPE_U8:
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u8);
                break;
            case ACB_CMD_OPCODE_TYPE_U16:
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u16);
                break;
            case ACB_CMD_OPCODE_TYPE_U24:
            case ACB_CMD_OPCODE_TYPE_U32:
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u32);
                break;
            case ACB_CMD_OPCODE_TYPE_U40:
            case ACB_CMD_OPCODE_TYPE_U48:
            case ACB_CMD_OPCODE_TYPE_U56:
            case ACB_CMD_OPCODE_TYPE_U64:
                sexml_append_attribute_uint(cmd_xml, "value", op->data.u64);
                break;
            case ACB_CMD_OPCODE_TYPE_F32:
                sexml_append_attribute_double(cmd_xml, "value", op->data.f32, 8);
                break;
            case ACB_CMD_OPCODE_TYPE_F64:
                sexml_append_attribute_double(cmd_xml, "value", op->data.f64, 16);
                break;
            case ACB_CMD_OPCODE_TYPE_VL:
                sexml_append_attribute_vl(cmd_xml, "value", (const char*)op->data.vl, op->size);
                break;
        }
    }
}

/*
	Packing
*/
inline static ACB_COMMAND cri_acb_cmd_xml_to_acbcmd(SEXML_ELEMENT* acbcmd_xml)
{
    const uint32_t cmd_count = cvec_size(acbcmd_xml->elements);
    ACB_COMMAND acbcmd = acb_cmd_alloc();
 
    for(uint32_t i = 0; i != cmd_count; ++i)
    {
        ACB_COMMAND_OPCODE op = {0};
        SEXML_ELEMENT* cmd_xml = sexml_get_element_by_id(acbcmd_xml, i);
        
        /* First we try to parse known opcodes */
        
        /* NOP - 0 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_NOP, strlen(ACB_CMD_NAME_NOP)) == 0)
        {
            acb_cmd_write_nop(&op);
            goto append_command;
        }
        
        /* MUTE - 33 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_MUTE, strlen(ACB_CMD_NAME_MUTE)) == 0)
        {
            SEXML_ATTRIBUTE* mute_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const uint8_t is_muted = sexml_get_attribute_uint(mute_xml);
            acb_cmd_write_mute(&op, is_muted);
            goto append_command;
        }
        
        /* CATEGORY - 65 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_CATEGORY, strlen(ACB_CMD_NAME_CATEGORY)) == 0)
        {
            SEXML_ATTRIBUTE* category_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const uint32_t category = sexml_get_attribute_uint(category_xml);
            acb_cmd_write_category(&op, category);
            goto append_command;
        }

        /* POS_3D_DISTANCE_MIN - 68 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_POS_3D_DISTANCE_MIN, strlen(ACB_CMD_NAME_POS_3D_DISTANCE_MIN)) == 0)
        {
            SEXML_ATTRIBUTE* pos_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const float pos = sexml_get_attribute_double(pos_xml);
            acb_cmd_write_pos_3d_distance_min(&op, pos);
            goto append_command;
        }

        /* POS_3D_DISTANCE_MAX - 69 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_POS_3D_DISTANCE_MAX, strlen(ACB_CMD_NAME_POS_3D_DISTANCE_MAX)) == 0)
        {
            SEXML_ATTRIBUTE* pos_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const float pos = sexml_get_attribute_double(pos_xml);
            acb_cmd_write_pos_3d_distance_max(&op, pos);
            goto append_command;
        }
        
        /* VOLUME_GAIN_RESOLUTION100 - 87 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_VOLUME_GAIN_RESOLUTION100, strlen(ACB_CMD_NAME_VOLUME_GAIN_RESOLUTION100)) == 0)
        {
            SEXML_ATTRIBUTE* gain_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const uint16_t gain = sexml_get_attribute_uint(gain_xml);
            acb_cmd_write_volume_gain_resolution100(&op, gain);
            goto append_command;
        }
        
        /* VOLUME_CONTROL - 146 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_VOLUME_CONTROL, strlen(ACB_CMD_NAME_VOLUME_CONTROL)) == 0)
        {
            SEXML_ATTRIBUTE* volume_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const float volume = sexml_get_attribute_double(volume_xml);
            acb_cmd_write_volume_control(&op, volume);
            goto append_command;
        }
        
        /* NOTE_ON - 2000 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_NOTE_ON, strlen(ACB_CMD_NAME_NOTE_ON)) == 0)
        {
            SEXML_ATTRIBUTE* type_xml = sexml_get_attribute_by_name(cmd_xml, "type");
            SEXML_ATTRIBUTE* index_xml = sexml_get_attribute_by_name(cmd_xml, "index");
            uint16_t type = 0;
            const uint16_t index = sexml_get_attribute_uint(index_xml);
            
            /* Synth */
            if(strncmp(type_xml->value->ptr,
                       ACB_CMD_NOTE_ON_TYPE_SYNTH_NAME,
                       strlen(ACB_CMD_NOTE_ON_TYPE_SYNTH_NAME)) == 0)
            {
                type = ACB_CMD_NOTE_ON_TYPE_SYNTH;
            }
            /* Sequence */
            else if(strncmp(type_xml->value->ptr,
                            ACB_CMD_NOTE_ON_TYPE_SEQUENCE_NAME,
                            strlen(ACB_CMD_NOTE_ON_TYPE_SEQUENCE_NAME)) == 0)
            {
                type = ACB_CMD_NOTE_ON_TYPE_SEQUENCE;
            }
            
            acb_cmd_write_note_on(&op, type, index);
            goto append_command;
        }
        
        /* DELAY - 2001 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_DELAY, strlen(ACB_CMD_NAME_DELAY)) == 0)
        {
            SEXML_ATTRIBUTE* delay_xml = sexml_get_attribute_by_name(cmd_xml, "value");
            const uint32_t delay = sexml_get_attribute_uint(delay_xml);
            acb_cmd_write_delay(&op, delay);
            goto append_command;
        }
        
        /* SEQUENCE_END - 4000 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_SEQUENCE_END, strlen(ACB_CMD_NAME_SEQUENCE_END)) == 0)
        {
            acb_cmd_write_sequence_end(&op);
            goto append_command;
        }
        
        /* BLOCK_END - 4050 */
        if(strncmp(cmd_xml->name->ptr, ACB_CMD_NAME_BLOCK_END, strlen(ACB_CMD_NAME_BLOCK_END)) == 0)
        {
            acb_cmd_write_block_end(&op);
            goto append_command;
        }
        
        /*
            It's an unknown opcode, do the usual
        */
        SEXML_ATTRIBUTE* type_attr = sexml_get_attribute_by_name(cmd_xml, "type");
        SEXML_ATTRIBUTE* val_attr = sexml_get_attribute_by_name(cmd_xml, "value");
        
        op.op = sexml_get_attribute_uint_by_name(cmd_xml, "op");
        op.type = acb_cmd_str_to_type(type_attr->value->ptr, type_attr->value->size);
        op.size = acb_cmd_size_by_type(op.type);
        
        switch(op.type)
        {
            case ACB_CMD_OPCODE_TYPE_U8:
                op.data.u8 = sexml_get_attribute_uint(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_U16:
                op.data.u16 = sexml_get_attribute_uint(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_U24:
            case ACB_CMD_OPCODE_TYPE_U32:
                op.data.u32 = sexml_get_attribute_uint(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_U40:
            case ACB_CMD_OPCODE_TYPE_U48:
            case ACB_CMD_OPCODE_TYPE_U56:
            case ACB_CMD_OPCODE_TYPE_U64:
                op.data.u64 = sexml_get_attribute_uint(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_F32:
                op.data.f32 = sexml_get_attribute_double(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_F64:
                op.data.f64 = sexml_get_attribute_double(val_attr);
                break;
            case ACB_CMD_OPCODE_TYPE_VL:
                SU_STRING* vl = sexml_get_attribute_vl(val_attr);
                op.size = vl->size;
                memcpy(op.data.vl, (uint8_t*)vl->ptr, op.size);
                su_free(vl);
        }

append_command:
        acb_cmd_append_opcode(acbcmd, op);
    }
    
    return acbcmd;
}

/*
    Misc
*/
inline static void cri_acb_cmd_print_acbcmd(ACB_COMMAND acbcmd)
{
    printf("### ACB Command ###\n");
    printf("|Opcode|Size| Type |     Value     |\n");
    
    for(uint32_t i = 0; i != cvec_size(acbcmd); ++i)
    {
        ACB_COMMAND_OPCODE* op = acb_cmd_get_opcode_by_id(acbcmd, i);
        printf("|%6u", op->op);
        printf("|%4u", op->size);
        printf("|%6s", acb_cmd_type_to_str(op->type));

        switch(op->type)
        {
            case ACB_CMD_OPCODE_TYPE_U8:
                printf("|%15u|", op->data.u8);
                break;
            case ACB_CMD_OPCODE_TYPE_U16:
                printf("|%15u|", op->data.u16);
                break;
            case ACB_CMD_OPCODE_TYPE_U24:
            case ACB_CMD_OPCODE_TYPE_U32:
                printf("|%15u|", op->data.u32);
                break;
            case ACB_CMD_OPCODE_TYPE_U40:
            case ACB_CMD_OPCODE_TYPE_U48:
            case ACB_CMD_OPCODE_TYPE_U56:
            case ACB_CMD_OPCODE_TYPE_U64:
                printf("|%15llu|", op->data.u64);
                break;
            case ACB_CMD_OPCODE_TYPE_F32:
                printf("|%15f|", op->data.f32);
                break;
            case ACB_CMD_OPCODE_TYPE_F64:
                printf("|%15lf|", op->data.f64);
                break;
            case ACB_CMD_OPCODE_TYPE_VL:
                printf("|%15s|", "Variable Length");
                break;
            default:
                printf("|    UNKNOWN    |");
        }
        
        printf("\n");
    }
    
    printf("### ACB Command END ###\n");
}