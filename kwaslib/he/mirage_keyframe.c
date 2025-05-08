#include "mirage_keyframe.h"

#include <kwaslib/core/io/type_readers.h>

const MIRAGE_KEYFRAME_SET mirage_read_kfs_from_data(const uint8_t* data)
{
    MIRAGE_KEYFRAME_SET kfs = {0};
    kfs.type = data[0];
    kfs.flag2 = data[1];
    kfs.interpolation = data[2];
    kfs.flag4 = data[3];
    kfs.length = tr_read_u32be(&data[4]);
    kfs.start = tr_read_u32be(&data[8]);
    
    return kfs;
}

void mirage_read_keyframe_sets_from_data(const uint8_t* data, const uint32_t kfs_count, CVEC keyframe_sets)
{
    uint32_t kfs_it = 0;
    cvec_resize(keyframe_sets, kfs_count);

    for(uint32_t i = 0; i != kfs_count; ++i)
    {
        MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(keyframe_sets, i);
        const MIRAGE_KEYFRAME_SET read_kfs = mirage_read_kfs_from_data(&data[kfs_it]);
        kfs->type = read_kfs.type;
        kfs->flag2 = read_kfs.flag2;
        kfs->interpolation = read_kfs.interpolation;
        kfs->flag4 = read_kfs.flag4;
        kfs->length = read_kfs.length;
        kfs->start = read_kfs.start;
        kfs_it += MIRAGE_KEYFRAME_SET_SIZE;
    }
}

void mirage_read_keyframes_from_data(const uint8_t* data, const uint32_t data_size, CVEC keyframes)
{
    uint32_t kf_it = 0;
    const uint32_t keyframe_count = data_size/MIRAGE_KEYFRAME_SIZE;
    cvec_resize(keyframes, keyframe_count);
    
    for(uint32_t i = 0; i != keyframe_count; ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(keyframes, i);
        kf->index = tr_read_f32be(&data[kf_it]);
        kf->value = tr_read_f32be(&data[kf_it+4]);
        kf_it += MIRAGE_KEYFRAME_SIZE;
    }
}

void mirage_push_keyframe(CVEC keyframes, const float index, const float value)
{
    MIRAGE_KEYFRAME kf = {0};
    kf.index = index;
    kf.value = value;
    cvec_push_back(keyframes, &kf);
}

void mirage_push_kfs(CVEC keyframe_sets, const uint8_t type, 
                     const uint8_t flag2, const uint8_t interpolation,
                     const uint8_t flag4, const uint32_t length,
                     const uint32_t start)
{
    MIRAGE_KEYFRAME_SET kfs = {0};
    kfs.type = type;
    kfs.flag2 = flag2;
    kfs.interpolation = interpolation;
    kfs.flag4 = flag4;
    kfs.length = length;
    kfs.start = start;
    cvec_push_back(keyframe_sets, &kfs);
}

MIRAGE_KEYFRAME* mirage_get_kf_by_id(CVEC keyframes, const uint32_t id)
{
    return (MIRAGE_KEYFRAME*)cvec_at(keyframes, id);
}

MIRAGE_KEYFRAME_SET* mirage_get_kfs_by_id(CVEC keyframe_sets, const uint32_t id)
{
    return (MIRAGE_KEYFRAME_SET*)cvec_at(keyframe_sets, id);
}