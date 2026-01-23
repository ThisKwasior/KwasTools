#pragma once

#include <stdint.h>

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/type_writers.h>

#include "acb_command.h"
#include "acb_command_opcodes.h"

/*
    Generic writers
*/
static inline void acb_cmd_write_generic_noval(ACB_COMMAND_OPCODE* op, const uint16_t opid)
{
    op->op = opid;
    op->type = ACB_CMD_OPCODE_TYPE_NOVAL;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
}

static inline void acb_cmd_write_generic_u8(ACB_COMMAND_OPCODE* op, const uint16_t opid, const uint8_t value)
{
    op->op = opid;
    op->type = ACB_CMD_OPCODE_TYPE_U8;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
    op->data.u8 = value;
}

static inline void acb_cmd_write_generic_u16(ACB_COMMAND_OPCODE* op, const uint16_t opid, const uint16_t value)
{
    op->op = opid;
    op->type = ACB_CMD_OPCODE_TYPE_U16;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
    op->data.u16 = value;
}

static inline void acb_cmd_write_generic_u32(ACB_COMMAND_OPCODE* op, const uint16_t opid, const uint32_t value)
{
    op->op = opid;
    op->type = ACB_CMD_OPCODE_TYPE_U32;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
    op->data.u32 = value;
}

static inline void acb_cmd_write_generic_f32(ACB_COMMAND_OPCODE* op, const uint16_t opid, const float value)
{
    op->op = opid;
    op->type = ACB_CMD_OPCODE_TYPE_F32;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
    op->data.f32 = value;
}

/*
    NOP (0) has no value and is of size 0.
*/
static inline void acb_cmd_write_nop(ACB_COMMAND_OPCODE* op)
{
    acb_cmd_write_generic_noval(op, ACB_CMD_OP_NOP);
}

/*
    BIQUAD (31) - enigma. In Crossworlds reports size 0x0b, yet data is only 0x07 bytes.
*/
static inline void acb_cmd_read_biquad(ACB_COMMAND_OPCODE* op, uint8_t* type, uint16_t* cof,
                                       uint16_t* gain, uint16_t* qf)
{
   *type = op->data.vl[0];
   *cof = tr_read_u16be(&op->data.vl[1]);
   *gain = tr_read_u16be(&op->data.vl[3]);
   *qf = tr_read_u16be(&op->data.vl[5]);
}

static inline void acb_cmd_write_biquad(ACB_COMMAND_OPCODE* op, const uint8_t type,
                                        const uint16_t cof, const uint16_t gain, const uint16_t qf)
{
    op->op = ACB_CMD_OP_BIQUAD;
    op->type = ACB_CMD_OPCODE_TYPE_VL;
    op->size = 0x0b;
    op->real_size = 0x07;
    op->data.vl[0] = type;
    tw_write_u16be(cof, &op->data.vl[1]);
    tw_write_u16be(gain, &op->data.vl[3]);
    tw_write_u16be(qf, &op->data.vl[5]);
}

/*
    MUTE (33) - u8. I guess it just mutes the audio if true.
*/
static inline void acb_cmd_write_mute(ACB_COMMAND_OPCODE* op, const uint8_t is_muted)
{
    acb_cmd_write_generic_u8(op, ACB_CMD_OP_MUTE, is_muted);
}

/*
    CATEGORY (65) no idea, but it's u32 - 
    EDIT: nope, it's variable length
    TODO: Make it variable length
*/
static inline void acb_cmd_write_category(ACB_COMMAND_OPCODE* op, const uint32_t category)
{
    acb_cmd_write_generic_u32(op, ACB_CMD_OP_CATEGORY, category);
}

/*
    POS_3D_DISTANCE_MIN (68) is a float min range for a sound to play in 3d space.
*/
static inline void acb_cmd_write_pos_3d_distance_min(ACB_COMMAND_OPCODE* op, const float min)
{
    acb_cmd_write_generic_f32(op, ACB_CMD_OP_POS_3D_DISTANCE_MIN, min);
}

/*
    POS_3D_DISTANCE_MAX (69) is a float max range for a sound to play in 3d space.
*/
static inline void acb_cmd_write_pos_3d_distance_max(ACB_COMMAND_OPCODE* op, const uint32_t max)
{
    acb_cmd_write_generic_f32(op, ACB_CMD_OP_POS_3D_DISTANCE_MAX, max);
}

/*
    VOLUME_GAIN_RESOLUTION100 (87) no idea, but it's u16
*/
static inline void acb_cmd_write_volume_gain_resolution100(ACB_COMMAND_OPCODE* op, const uint32_t gain)
{
    acb_cmd_write_generic_u16(op, ACB_CMD_OP_VOLUME_GAIN_RESOLUTION100, gain);
}

/*
    VOLUME_CONTROL (146) - A float value that controls the volume for the track.
*/
static inline void acb_cmd_write_volume_control(ACB_COMMAND_OPCODE* op, const float volume)
{
    acb_cmd_write_generic_f32(op, ACB_CMD_OP_VOLUME_CONTROL, volume);
}

/*
    NOTE_ON (2000) has two 16bit values encoded as a 32bit integer.
    These are type (2 - SYNTH, 3 - SEQUENCE) and index in Synth or Sequence tables.
*/
static inline void acb_cmd_read_note_on(ACB_COMMAND_OPCODE* op, uint16_t* type, uint16_t* index)
{
    *type = tr_read_u16le(&op->data.vl[2]);
    *index = tr_read_u16le(&op->data.vl[0]);
}

static inline void acb_cmd_write_note_on(ACB_COMMAND_OPCODE* op, const uint16_t type, const uint16_t index)
{
    op->op = ACB_CMD_OP_NOTE_ON;
    op->type = ACB_CMD_OPCODE_TYPE_U32;
    op->size = acb_cmd_size_by_type(op->type);
    op->real_size = acb_cmd_size_by_type(op->type);
    
    tw_write_u16le(type, &op->data.vl[2]);
    tw_write_u16le(index, &op->data.vl[0]);
}

/*
    DELAY (2001) no idea, u32. In Sonic Frontiers it's used for boss music.
    The value is the waveform length in milliseconds; maybe it just delays execution
    of further commands by the time in ms?
*/
static inline void acb_cmd_write_delay(ACB_COMMAND_OPCODE* op, const uint32_t delay)
{
    acb_cmd_write_generic_u32(op, ACB_CMD_OP_DELAY, delay);
}

/*
    SEQUENCE_END (4000) denotes the end in command sequence in TrackEvent
    (like I know what that means).
    Like NOP, it's of size 0.
*/
static inline void acb_cmd_write_sequence_end(ACB_COMMAND_OPCODE* op)
{
    acb_cmd_write_generic_noval(op, ACB_CMD_OP_SEQUENCE_END);
}

/*
    BLOCK_END (4050) denotes the end in command block in TrackEvent
    (like I know what that means).
    Like NOP, it's of size 0.
*/
static inline void acb_cmd_write_block_end(ACB_COMMAND_OPCODE* op)
{
    acb_cmd_write_generic_noval(op, ACB_CMD_OP_BLOCK_END);
}

/*
    START_ACTION (7100) - noval. Nuff said
*/
static inline void acb_cmd_write_start_action(ACB_COMMAND_OPCODE* op)
{
    acb_cmd_write_generic_noval(op, ACB_CMD_OP_START_ACTION);
}

/*
    STOP_ACTION (7101) - noval. Nuff said
*/
static inline void acb_cmd_write_stop_action(ACB_COMMAND_OPCODE* op)
{
    acb_cmd_write_generic_noval(op, ACB_CMD_OP_STOP_ACTION);
}