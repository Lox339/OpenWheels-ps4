#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/ps4/CCGLViewImpl-ps4.h"
#include "base/CCDirector.h"
#include <cstdio>
#include <cstdlib>

NS_CC_BEGIN

GLViewImpl* GLViewImpl::create(const std::string& viewName)
{
    auto ret = new (std::nothrow) GLViewImpl();
    if (ret && ret->initWithFullScreen(viewName))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithRect(const std::string& viewName, Rect rect, float frameZoomFactor)
{
    return create(viewName);
}

GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName)
{
    return create(viewName);
}

GLViewImpl::GLViewImpl()
    : _eglDisplay(EGL_NO_DISPLAY)
    , _eglSurface(EGL_NO_SURFACE)
    , _eglContext(EGL_NO_CONTEXT)
    , _isClosed(false)
{
}

GLViewImpl::~GLViewImpl()
{
    end();
}

bool GLViewImpl::initWithRect(const std::string& viewName, Rect rect, float frameZoomFactor)
{
    return initWithFullScreen(viewName);
}

bool GLViewImpl::initWithFullScreen(const std::string& viewName)
{
    (void)viewName;
    _screenSize = Size(1920.0f, 1080.0f);
    setDesignResolutionSize(1920.0f, 1080.0f, ResolutionPolicy::SHOW_ALL);

    _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (_eglDisplay != EGL_NO_DISPLAY)
    {
        EGLint major = 0, minor = 0;
        if (eglInitialize(_eglDisplay, &major, &minor))
        {
            const EGLint configAttribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                EGL_NONE
            };

            EGLConfig config;
            EGLint numConfigs = 0;
            if (eglChooseConfig(_eglDisplay, configAttribs, &config, 1, &numConfigs) && numConfigs > 0)
            {
                const EGLint contextAttribs[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
                };

                _eglContext = eglCreateContext(_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
                _eglSurface = eglCreateWindowSurface(_eglDisplay, config, (EGLNativeWindowType)0, nullptr);
                eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext);
            }
        }
    }

    return true;
}

bool GLViewImpl::isOpenGLReady()
{
    return (_screenSize.width > 0 && _screenSize.height > 0);
}

void GLViewImpl::end()
{
    if (_eglDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (_eglContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(_eglDisplay, _eglContext);
            _eglContext = EGL_NO_CONTEXT;
        }
        if (_eglSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(_eglDisplay, _eglSurface);
            _eglSurface = EGL_NO_SURFACE;
        }
        eglTerminate(_eglDisplay);
        _eglDisplay = EGL_NO_DISPLAY;
    }
    _isClosed = true;
}

void GLViewImpl::swapBuffers()
{
    if (_eglDisplay != EGL_NO_DISPLAY && _eglSurface != EGL_NO_SURFACE)
    {
        eglSwapBuffers(_eglDisplay, _eglSurface);
    }
}

void GLViewImpl::setIMEKeyboardState(bool bOpen)
{
    (void)bOpen;
}

bool GLViewImpl::windowShouldClose()
{
    return _isClosed;
}

void GLViewImpl::pollEvents()
{
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
