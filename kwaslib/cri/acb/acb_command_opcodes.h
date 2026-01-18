#pragma once

/*
    Integers
*/
#define ACB_CMD_OP_NOP                                      (uint16_t)(0)
#define ACB_CMD_OP_MUTE                                     (uint16_t)(33)
#define ACB_CMD_OP_CATEGORY                                 (uint16_t)(65)
#define ACB_CMD_OP_VOLUME_GAIN_RESOLUTION100                (uint16_t)(87)
#define ACB_CMD_OP_UNK_006F                                 (uint16_t)(111)
#define ACB_CMD_OP_STOP_AT_LOOP_END                         (uint16_t)(124)
#define ACB_CMD_OP_SEQUENCE_START_RANDOM                    (uint16_t)(998)
#define ACB_CMD_OP_SEQUENCE_START                           (uint16_t)(999)
#define ACB_CMD_OP_NOTE_OFF                                 (uint16_t)(1000)
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_ID                (uint16_t)(1251)
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_STRING            (uint16_t)(1252)
#define ACB_CMD_OP_SEQUENCE_CALLBACK_WITH_ID_AND_STRING     (uint16_t)(1253)
#define ACB_CMD_OP_NOTE_ON                                  (uint16_t)(2000)
#define ACB_CMD_OP_DELAY                                    (uint16_t)(2001)
#define ACB_CMD_OP_SET_SYNTH_OR_WAVEFORM                    (uint16_t)(2002)
#define ACB_CMD_OP_NOTE_ON_WITH_NO                          (uint16_t)(2003)
#define ACB_CMD_OP_NOTE_ON_WITH_DURATION                    (uint16_t)(2004)
#define ACB_CMD_OP_UNK_07D5                                 (uint16_t)(2005)
#define ACB_CMD_OP_SEQUENCE_END                             (uint16_t)(4000)
#define ACB_CMD_OP_BLOCK_END                                (uint16_t)(4050)
#define ACB_CMD_OP_TRANSITION_TRACK                         (uint16_t)(4051)
#define ACB_CMD_OP_START_ACTION                             (uint16_t)(7100)
#define ACB_CMD_OP_STOP_ACTION                              (uint16_t)(7101)
#define ACB_CMD_OP_MUTE_TRACK_ACTION                        (uint16_t)(7102)

/*
    Float/Double
*/
#define ACB_CMD_OP_POS_3D_DISTANCE_MIN                      (uint16_t)(68)
#define ACB_CMD_OP_POS_3D_DISTANCE_MAX                      (uint16_t)(69)
#define ACB_CMD_OP_VOLUME_CONTROL                           (uint16_t)(146)


/*
    Opcode specific defines
*/
#define ACB_CMD_NOTE_ON_TYPE_SYNTH                          (uint16_t)(2)
#define ACB_CMD_NOTE_ON_TYPE_SEQUENCE                       (uint16_t)(3)
#define ACB_CMD_NOTE_ON_TYPE_SYNTH_NAME                     (const char*)"SYNTH"
#define ACB_CMD_NOTE_ON_TYPE_SEQUENCE_NAME                  (const char*)"SEQUENCE"
#define ACB_CMD_NOTE_ON_TYPE_UNK_NAME                       (const char*)"UNKNOWN_PLEASE_TELL_ME"

/*
    Names
*/
#define ACB_CMD_NAME_NOP                                    (const char*)"nop"
#define ACB_CMD_NAME_MUTE                                   (const char*)"mute"
#define ACB_CMD_NAME_CATEGORY                               (const char*)"category"
#define ACB_CMD_NAME_VOLUME_GAIN_RESOLUTION100              (const char*)"volumeGainResolution100"
#define ACB_CMD_NAME_STOP_AT_LOOP_END                       (const char*)"stopAtLoopEnd"
#define ACB_CMD_NAME_SEQUENCE_START_RANDOM                  (const char*)"sequenceStartRandom"
#define ACB_CMD_NAME_SEQUENCE_START                         (const char*)"sequenceStart"
#define ACB_CMD_NAME_NOTE_OFF                               (const char*)"noteOff"
#define ACB_CMD_NAME_SEQUENCE_CALLBACK_WITH_ID              (const char*)"sequenceCallbackWithId"
#define ACB_CMD_NAME_SEQUENCE_CALLBACK_WITH_STRING          (const char*)"sequenceCallbackWithString"
#define ACB_CMD_NAME_SEQUENCE_CALLBACK_WITH_ID_AND_STRING   (const char*)"sequenceCallbackWithIdAndString"
#define ACB_CMD_NAME_NOTE_ON                                (const char*)"noteOn"
#define ACB_CMD_NAME_DELAY                                  (const char*)"delay"
#define ACB_CMD_NAME_SET_SYNTH_OR_WAVEFORM                  (const char*)"setSynthOrWaveform"
#define ACB_CMD_NAME_NOTE_ON_WITH_NO                        (const char*)"noteOnWithNo"
#define ACB_CMD_NAME_NOTE_ON_WITH_DURATION                  (const char*)"noteOnWithDuration"
#define ACB_CMD_NAME_SEQUENCE_END                             (const char*)"sequenceEnd"
#define ACB_CMD_NAME_BLOCK_END                              (const char*)"blockEnd"
#define ACB_CMD_NAME_TRANSITION_TRACK                       (const char*)"transitionTrack"
#define ACB_CMD_NAME_START_ACTION                           (const char*)"startAction"
#define ACB_CMD_NAME_STOP_ACTION                            (const char*)"stopAction"
#define ACB_CMD_NAME_MUTE_TRACK_ACTION                      (const char*)"muteTrackAction"

#define ACB_CMD_NAME_POS_3D_DISTANCE_MIN                    (const char*)"pos3dDistanceMin"
#define ACB_CMD_NAME_POS_3D_DISTANCE_MAX                    (const char*)"pos3dDistanceMax"
#define ACB_CMD_NAME_VOLUME_CONTROL                         (const char*)"volumeControl"