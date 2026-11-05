#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCCommon.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cmath>

NS_CC_BEGIN

void MessageBox(const char * msg, const char * title)
{
    printf("[OpenWheels PS4] %s: %s\n", title ? title : "", msg ? msg : "");
}

NS_CC_END

namespace cocos2d {
namespace ui {
    class EditBox;
    class EditBoxImpl;
    EditBoxImpl* __createSystemEditBox(EditBox*) {
        return nullptr;
    }
}
}

extern "C" {
int __fprintf_chk(FILE *stream, int flag, const char *format, ...) {
    (void)flag;
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stream, format, ap);
    va_end(ap);
    return ret;
}

int __vfprintf_chk(FILE *stream, int flag, const char *format, va_list ap) {
    (void)flag;
    return vfprintf(stream, format, ap);
}

int __sprintf_chk(char *str, int flag, size_t maxlen, const char *format, ...) {
    (void)flag;
    (void)maxlen;
    va_list ap;
    va_start(ap, format);
    int ret = vsprintf(str, format, ap);
    va_end(ap);
    return ret;
}

void *__memcpy_chk(void *dest, const void *src, size_t len, size_t destlen) {
    (void)destlen;
    return memcpy(dest, src, len);
}

float __powf_finite(float x, float y) {
    return powf(x, y);
}

float __expf_finite(float x) {
    return expf(x);
}
}

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
