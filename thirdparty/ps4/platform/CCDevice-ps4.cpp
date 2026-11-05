#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCDevice.h"

NS_CC_BEGIN

int Device::getDPI()
{
    return 96;
}

void Device::setAccelerometerEnabled(bool isEnabled)
{
    (void)isEnabled;
}

void Device::setAccelerometerInterval(float interval)
{
    (void)interval;
}

void Device::setKeepScreenOn(bool keepScreenOn)
{
    (void)keepScreenOn;
}

void Device::vibrate(float duration)
{
    (void)duration;
}

Data Device::getTextureDataForText(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha)
{
    (void)text;
    (void)textDefinition;
    (void)align;
    width = 0;
    height = 0;
    hasPremultipliedAlpha = false;
    return Data();
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
