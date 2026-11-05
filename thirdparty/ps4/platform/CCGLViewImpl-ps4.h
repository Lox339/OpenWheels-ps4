#ifndef __CC_GLVIEWIMPL_PS4_H__
#define __CC_GLVIEWIMPL_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCGLView.h"
#include <EGL/egl.h>

NS_CC_BEGIN

class CC_DLL GLViewImpl : public GLView
{
public:
    static GLViewImpl* create(const std::string& viewName);
    static GLViewImpl* createWithRect(const std::string& viewName, Rect rect, float frameZoomFactor = 1.0f);
    static GLViewImpl* createWithFullScreen(const std::string& viewName);

    bool isOpenGLReady() override;
    void end() override;
    void swapBuffers() override;
    void setIMEKeyboardState(bool bOpen) override;
    bool windowShouldClose() override;
    void pollEvents() override;

protected:
    GLViewImpl();
    virtual ~GLViewImpl();

    bool initWithRect(const std::string& viewName, Rect rect, float frameZoomFactor);
    bool initWithFullScreen(const std::string& viewName);

    EGLDisplay _eglDisplay;
    EGLSurface _eglSurface;
    EGLContext _eglContext;
    bool _isClosed;
};

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __CC_GLVIEWIMPL_PS4_H__
