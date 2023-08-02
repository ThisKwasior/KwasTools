#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* 
	Utils
*/
#include <kwaslib/utils/cpu/endianness.h>

#include <kwaslib/utils/crypto/crc32.h>

#include <kwaslib/utils/io/arg_parser.h>
#include <kwaslib/utils/io/dir_list.h>
#include <kwaslib/utils/io/file_utils.h>
#include <kwaslib/utils/io/path_utils.h>
#include <kwaslib/utils/io/type_readers.h>
#include <kwaslib/utils/io/type_writers.h>

#include <kwaslib/utils/math/boundary.h>
#include <kwaslib/utils/math/half.h>
#include <kwaslib/utils/math/vec.h>

/* 
	Hedgehog Engine
*/
#include <kwaslib/he/BINA.h>
#include <kwaslib/he/mirage.h>
#include <kwaslib/he/uv_anim.h>
#include <kwaslib/he/cam_anim.h>

/* 
	Platinum Games engine
*/
#include <kwaslib/platinum/dat.h>
#include <kwaslib/platinum/wmb4.h>
#include <kwaslib/platinum/wtb.h>

#ifdef __cplusplus
}
#endif