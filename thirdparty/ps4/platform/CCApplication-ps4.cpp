#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/ps4/CCApplication-ps4.h"
#include "base/CCDirector.h"
#include <sys/time.h>
#include <unistd.h>

NS_CC_BEGIN

Application* Application::sm_pSharedApplication = nullptr;

static long getCurrentMillSecond() {
    struct timeval stCurrentTime;
    gettimeofday(&stCurrentTime, NULL);
    return stCurrentTime.tv_sec * 1000 + stCurrentTime.tv_usec / 1000;
}

Application::Application()
    : _animationInterval(1.0f / 60.0f * 1000.0f)
{
    CC_ASSERT(!sm_pSharedApplication);
    sm_pSharedApplication = this;
}

Application::~Application()
{
    CC_ASSERT(this == sm_pSharedApplication);
    sm_pSharedApplication = nullptr;
}

int Application::run()
{
    initGLContextAttrs();
    if (!applicationDidFinishLaunching())
    {
        return 0;
    }

    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview)
    {
        return 0;
    }
    glview->retain();

    long lastTime = 0L;
    long curTime = 0L;

    while (!glview->windowShouldClose())
    {
        lastTime = getCurrentMillSecond();

        director->mainLoop();
        glview->pollEvents();

        curTime = getCurrentMillSecond();
        if (curTime - lastTime < _animationInterval)
        {
            usleep((_animationInterval - curTime + lastTime) * 1000);
        }
    }

    if (glview->isOpenGLReady())
    {
        director->end();
        director->mainLoop();
        director = nullptr;
    }
    glview->release();
    return 0;
}

void Application::setAnimationInterval(float interval)
{
    _animationInterval = interval * 1000.0f;
}

Application::Platform Application::getTargetPlatform()
{
    return Platform::OS_LINUX;
}

std::string Application::getVersion()
{
    return "1.0.0";
}

bool Application::openURL(const std::string &url)
{
    (void)url;
    return false;
}

Application* Application::getInstance()
{
    CC_ASSERT(sm_pSharedApplication);
    return sm_pSharedApplication;
}

Application* Application::sharedApplication()
{
    return Application::getInstance();
}

const char* Application::getCurrentLanguageCode()
{
    return "en";
}

LanguageType Application::getCurrentLanguage()
{
    return LanguageType::ENGLISH;
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
