#ifndef __CC_APPLICATION_PS4_H__
#define __CC_APPLICATION_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCApplicationProtocol.h"
#include <string>

NS_CC_BEGIN

class CC_DLL Application : public ApplicationProtocol
{
public:
    Application();
    virtual ~Application();

    int run();
    static Application* getInstance();
    static Application* sharedApplication();

    void setAnimationInterval(float interval) override;
    LanguageType getCurrentLanguage() override;
    const char* getCurrentLanguageCode() override;
    Platform getTargetPlatform() override;
    std::string getVersion() override;
    bool openURL(const std::string &url) override;

protected:
    static Application* sm_pSharedApplication;
    float _animationInterval;
};

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __CC_APPLICATION_PS4_H__
