#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* 
	Utils
*/
#include <kwaslib/core/cpu/endianness.h>

#include <kwaslib/core/crypto/crc32.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/dir_list.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/type_writers.h>

#include <kwaslib/core/math/boundary.h>
#include <kwaslib/core/math/half.h>
#include <kwaslib/core/math/vec.h>

#include <kwaslib/core/data/dbl_link_list.h>

/* 
	CRIWARE
*/
#include <kwaslib/cri/acb_command.h>
#include <kwaslib/cri/utf.h>
#include <kwaslib/cri/awb.h>

/* 
	Hedgehog Engine
*/
#include <kwaslib/he/BINA.h>
#include <kwaslib/he/mirage.h>
#include <kwaslib/he/uv_anim.h>
#include <kwaslib/he/cam_anim.h>

/* 
	NW4R (Nintendo Wii)
*/
#include <kwaslib/nw4r/brres.h>
#include <kwaslib/nw4r/srt0.h>
#include <kwaslib/nw4r/scn0.h>

/* 
	Platinum Games engine
*/
#include <kwaslib/platinum/dat.h>
#include <kwaslib/platinum/wmb4.h>
#include <kwaslib/platinum/wtb.h>

#ifdef __cplusplus
}
#endif