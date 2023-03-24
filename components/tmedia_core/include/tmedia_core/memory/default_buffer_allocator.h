/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef _TMDEFAULTBUFFERALLOCATOR_H
#define _TMDEFAULTBUFFERALLOCATOR_H

#include "buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 *   The default allocator initialization is called when the application is started,
 * and needs to be executed before the buffer pool initialization
 *
 * @return allocator
 */
int DefaultAllocator_Init();

/**
 * Deinitialization default Allocator when the application exits
 *
 * @return allocator
 */
void DefaultAllocator_DeInit();


/**
 *  get default allocator
 *
 * @return Allocator
 */
TMBufferAllocator *DefaultBufferAllocator_Get();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif