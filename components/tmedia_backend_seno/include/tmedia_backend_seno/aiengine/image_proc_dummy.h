/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef __IMAGE_PROC_DUMMY_H__
#define __IMAGE_PROC_DUMMY_H__

#include <string>

#include <tmedia_core/entity/aiengine/image_proc.h>

using namespace std;

class ImageProcDummy : public ImageProc
{
public:
    ImageProcDummy();
    ~ImageProcDummy();

    int Open(int idx);
    int Close();

    int CvtColor(const TMVideoFrame &src, TMImageInfo::PixelFormat dst_format, TMVideoFrame &dst);
    int Resize(const TMVideoFrame &src, const TMImageInfo::ImageSize_t &dst_size, TMVideoFrame &dst);
    int CvtResize(const TMVideoFrame &src, const TMImageInfo::ImageSize_t &dst_size, TMImageInfo::PixelFormat dst_format, TMVideoFrame &dst);
    int Crop(const TMVideoFrame &src, const TMImageInfo::ImageRect_t &dst_rect, TMVideoFrame &dst);
    int CropResize(const TMVideoFrame &src, const TMImageInfo::ImageRect_t &crop_rect, const TMImageInfo::ImageSize_t &dst_size,
                      TMImageInfo::PixelFormat dst_format, TMVideoFrame &dst) override;
};




#endif // __IMAGE_PROC_DUMMY_H__