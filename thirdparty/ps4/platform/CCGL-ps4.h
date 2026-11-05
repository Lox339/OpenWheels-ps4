#ifndef __CCGL_PS4_H__
#define __CCGL_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#define glClearDepth                glClearDepthf
#define glDeleteVertexArrays        glDeleteVertexArraysOES
#define glGenVertexArrays           glGenVertexArraysOES
#define glBindVertexArray           glBindVertexArrayOES
#define glMapBuffer                 glMapBufferOES
#define glUnmapBuffer               glUnmapBufferOES

#define GL_DEPTH24_STENCIL8         0x88F0
#define GL_WRITE_ONLY               0x88B9

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef char GLchar;

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __CCGL_PS4_H__
