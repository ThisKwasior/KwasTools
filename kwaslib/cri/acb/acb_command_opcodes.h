#pragma once

#define ACB_CMD_OP_NOP                                      0
#define ACB_CMD_OP_MUTE                                     33
#define ACB_CMD_OP_CATEGORY                                 65
#define ACB_CMD_OP_VOLUME_GAIN_RESOLUTION100                87
#define ACB_CMD_OP_UNK_006F                                 111
#define ACB_CMD_OP_STOP_AT_LOOP_END                         124
#define ACB_CMD_OP_SEQUENCE_START_RANDOM                    998
#define ACB_CMD_OP_SEQUENCE_START                           999
#define ACB_CMD_OP_NOTE_OFF                                 1000
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_ID                1251
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_STRING            1252
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_ID_AND_STRING     1253
#define ACB_CMD_OP_NOTE_ON                                  2000
#define ACB_CMD_OP_DELAY                                    2001
#define ACB_CMD_OP_SET_SYNTH_OR_WAVEFORM                    2002
#define ACB_CMD_OP_NOTE_ON_WITH_NO                          2003
#define ACB_CMD_OP_NOTE_ON_WITH_DURATION                    2004
#define ACB_CMD_OP_UNK_07D5                                 2005
#define ACB_CMD_OP_BLOCK_END                                4050
#define ACB_CMD_OP_TRANSITION_TRACK                         4051
#define ACB_CMD_OP_START_ACTION                             7100
#define ACB_CMD_OP_STOP_ACTION                              7101
#define ACB_CMD_OP_MUTE_TRACK_ACTION                        7102

/*
    Float/Double
*/
#define ACB_CMD_OP_POS_3D_DISTANCE_MIN                      68
#define ACB_CMD_OP_POS_3D_DISTANCE_MAX                      69
#define ACB_CMD_OP_VOLUME_CONTROL                           146