#ifndef __CC_PLATFORM_DEFINE_PS4_H__
#define __CC_PLATFORM_DEFINE_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#define CC_DLL

#define CC_ASSERT(cond)    assert(cond)
#define CC_UNUSED_PARAM(unusedparam) (void)unusedparam

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __CC_PLATFORM_DEFINE_PS4_H__
