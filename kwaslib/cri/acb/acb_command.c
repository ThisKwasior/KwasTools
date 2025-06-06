#include "acb_command.h"

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/type_writers.h>

#include "acb_command_opcodes.h"

static const char* ACB_CMD_TYPE_STR[] =
{
    "NOVAL", "U8", "U16", "U32",
    "U64", "FLOAT", "DOUBLE", "U24",
    "U40", "U48", "U56"
};

ACB_COMMAND acb_cmd_alloc()
{
    return cvec_create(sizeof(ACB_COMMAND_OPCODE));
}

ACB_COMMAND acb_cmd_load_from_data(const uint8_t* data, const uint32_t size)
{
    ACB_COMMAND acbcmd = acb_cmd_alloc();
    
    uint32_t iter = 0;
    
    while(iter < size)
    {
        const ACB_COMMAND_OPCODE op = acb_cmd_parse_opcode(&data[iter]);
        acb_cmd_append_opcode(acbcmd, op);
        iter += 3;
        iter += op.size;
    }
    
    return acbcmd;
}

ACB_COMMAND_OPCODE acb_cmd_parse_opcode(const uint8_t* data)
{
    ACB_COMMAND_OPCODE cmd = {0};
    
    cmd.op = tr_read_u16be(&data[0]);
    cmd.size = data[2];
    
    /* For nonstandard ints */
    uint8_t buf[8] = {0};
    
    switch(cmd.size)
    {
        case 0:
            cmd.type = ACB_CMD_OPCODE_TYPE_NOVAL;
            break;
        case 1:
            cmd.type = ACB_CMD_OPCODE_TYPE_U8;
            cmd.data.u8 = data[3];
            break;
        case 2:
            cmd.type = ACB_CMD_OPCODE_TYPE_U16;
            cmd.data.u16 = tr_read_u16be(&data[3]);
            break;
        case 3:
            cmd.type = ACB_CMD_OPCODE_TYPE_U24;
            tr_read_array(&data[3], 3, &buf[1]);
            cmd.data.u32 = tr_read_u32be(&buf[0]);
            break;
        case 4:
            cmd.type = ACB_CMD_OPCODE_TYPE_U32;
            cmd.data.u32 = tr_read_u32be(&data[3]);
            break;
        case 5:
            cmd.type = ACB_CMD_OPCODE_TYPE_U40;
            tr_read_array(&data[3], 5, &buf[3]);
            cmd.data.u64 = tr_read_u64be(&buf[0]);
            break;
        case 6:
            cmd.type = ACB_CMD_OPCODE_TYPE_U48;
            tr_read_array(&data[3], 6, &buf[2]);
            cmd.data.u64 = tr_read_u64be(&buf[0]);
            break;
        case 7:
            cmd.type = ACB_CMD_OPCODE_TYPE_U56;
            tr_read_array(&data[3], 7, &buf[1]);
            cmd.data.u64 = tr_read_u64be(&buf[0]);
            break;
        case 8:
            cmd.type = ACB_CMD_OPCODE_TYPE_U64;
            cmd.data.u64 = tr_read_u64be(&data[3]);
            break;
    }
    
    /* Check for floats */
    switch(cmd.op)
    {
        case ACB_CMD_OP_POS_3D_DISTANCE_MIN:
        case ACB_CMD_OP_POS_3D_DISTANCE_MAX:
        case ACB_CMD_OP_VOLUME_CONTROL:
            if(cmd.size == 4)
                cmd.type = ACB_CMD_OPCODE_TYPE_F32;
            if(cmd.size == 8)
                cmd.type = ACB_CMD_OPCODE_TYPE_F64;
            break;
    }
    
    return cmd;
}

void acb_cmd_append_opcode(ACB_COMMAND acbcmd, ACB_COMMAND_OPCODE op)
{
    cvec_push_back(acbcmd, &op);
}

ACB_COMMAND_OPCODE* acb_cmd_get_opcode_by_id(ACB_COMMAND acbcmd, const uint32_t id)
{
    return (ACB_COMMAND_OPCODE*)cvec_at(acbcmd, id);
}

ACB_COMMAND acb_cmd_free(ACB_COMMAND acbcmd)
{
    return cvec_destroy(acbcmd);
}

SU_STRING* acb_cmd_to_data(ACB_COMMAND acbcmd)
{
    const uint32_t output_size = acb_cmd_calc_buffer_size(acbcmd);
    SU_STRING* output = su_create_string(NULL, output_size);
    uint8_t* ptr = (uint8_t*)&output->ptr[0];
    
    for(uint32_t i = 0; i != cvec_size(acbcmd); ++i)
    {
        /* For nonstandard ints */
        uint8_t buf[8] = {0};
        
        ACB_COMMAND_OPCODE* op = acb_cmd_get_opcode_by_id(acbcmd, i);
        
        tw_write_u16be(op->op, ptr);
        ptr[2] = op->size;
        ptr += 3;
        
        switch(op->type)
        {
            case ACB_CMD_OPCODE_TYPE_U8:
                ptr[0] = op->data.u8;
                break;
            case ACB_CMD_OPCODE_TYPE_U16:
                tw_write_u16be(op->data.u16, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U24:
                tw_write_u32be(op->data.u32, buf);
                tw_write_array(&buf[1], 3, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U32:
                tw_write_u32be(op->data.u32, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U40:
                tw_write_u64be(op->data.u64, buf);
                tw_write_array(&buf[3], 5, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U48:
                tw_write_u64be(op->data.u64, buf);
                tw_write_array(&buf[2], 6, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U56:
                tw_write_u64be(op->data.u64, buf);
                tw_write_array(&buf[1], 7, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_U64:
                tw_write_u64be(op->data.u64, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_F32:
                tw_write_f32be(op->data.f32, ptr);
                break;
            case ACB_CMD_OPCODE_TYPE_F64:
                tw_write_f32be(op->data.f64, ptr);
                break;
        }
        
        ptr += op->size;
    }
    
    return output;
}

uint32_t acb_cmd_calc_buffer_size(ACB_COMMAND acbcmd)
{
    uint32_t size = 0;
    
    for(uint32_t i = 0; i != cvec_size(acbcmd); ++i)
    {
        ACB_COMMAND_OPCODE* op = acb_cmd_get_opcode_by_id(acbcmd, i);
        size += 3;
        size += op->size;
    }
    
    return size;
}

const uint32_t acb_cmd_get_opcode_count(ACB_COMMAND acbcmd)
{
    return cvec_size(acbcmd);
}

const char* acb_cmd_type_to_str(const uint8_t type)
{
    return ACB_CMD_TYPE_STR[type];
}

const uint8_t acb_cmd_str_to_type(const char* str, const uint8_t size)
{
    uint8_t type = -1;
    
    if(su_cmp_char(str, size, "NOVAL", 5) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_NOVAL;
    if(su_cmp_char(str, size, "U8", 2) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U8;
    if(su_cmp_char(str, size, "U16", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U16;
    if(su_cmp_char(str, size, "U32", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U32;
    if(su_cmp_char(str, size, "U64", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U64;
    if(su_cmp_char(str, size, "FLOAT", 5) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_F32;
    if(su_cmp_char(str, size, "DOUBLE", 6) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_F64;
    if(su_cmp_char(str, size, "U24", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U24;
    if(su_cmp_char(str, size, "U40", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U40;
    if(su_cmp_char(str, size, "U48", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U48;
    if(su_cmp_char(str, size, "U56", 3) == SU_STRINGS_MATCH)
        type = ACB_CMD_OPCODE_TYPE_U56;
    
    return type;
}

const uint8_t acb_cmd_size_by_type(const uint8_t type)
{
    switch(type)
    {
        case ACB_CMD_OPCODE_TYPE_NOVAL: return 0;
        case ACB_CMD_OPCODE_TYPE_U8:    return 1;
        case ACB_CMD_OPCODE_TYPE_U16:   return 2;
        case ACB_CMD_OPCODE_TYPE_U32:   return 4;
        case ACB_CMD_OPCODE_TYPE_U64:   return 8;
        case ACB_CMD_OPCODE_TYPE_F32:   return 4;
        case ACB_CMD_OPCODE_TYPE_F64:   return 8;
        case ACB_CMD_OPCODE_TYPE_U24:   return 3;
        case ACB_CMD_OPCODE_TYPE_U40:   return 5;
        case ACB_CMD_OPCODE_TYPE_U48:   return 6;
        case ACB_CMD_OPCODE_TYPE_U56:   return 7;
    }
    
    return 0;
}