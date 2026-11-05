#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCCommon.h"
#include <cstdio>

NS_CC_BEGIN

void MessageBox(const char * msg, const char * title)
{
    printf("[OpenWheels PS4] %s: %s\n", title ? title : "", msg ? msg : "");
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
