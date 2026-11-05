// PS4 DualShock 4 Controller Implementation for OpenWheels (src/input/Gamepad.h)
#include "platform/ps4/PS4Gamepad.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "cocos2d.h"
#include "input/Gamepad.h"
#include "input/Tilt.h"

#if defined(__ORBIS__) || defined(PS4) || defined(__PS4__)
#if __has_include(<pad.h>)
#include <pad.h>
#endif
#if __has_include(<user_service.h>)
#include <user_service.h>
#endif
#endif

// Ensure PS4 types and constants exist even if compiling with minimal headers
#ifndef SCE_PAD_PORT_TYPE_STANDARD
#define SCE_PAD_PORT_TYPE_STANDARD 0
#endif

#ifndef SCE_PAD_MAX_TOUCH_NUM
#define SCE_PAD_MAX_TOUCH_NUM 2
#endif

#ifndef SCE_PAD_BUTTON_L3
#define SCE_PAD_BUTTON_L3          0x00000002
#define SCE_PAD_BUTTON_R3          0x00000004
#define SCE_PAD_BUTTON_OPTIONS     0x00000008
#define SCE_PAD_BUTTON_UP          0x00000010
#define SCE_PAD_BUTTON_RIGHT       0x00000020
#define SCE_PAD_BUTTON_DOWN        0x00000040
#define SCE_PAD_BUTTON_LEFT        0x00000080
#define SCE_PAD_BUTTON_L2          0x00000100
#define SCE_PAD_BUTTON_R2          0x00000200
#define SCE_PAD_BUTTON_L1          0x00000400
#define SCE_PAD_BUTTON_R1          0x00000800
#define SCE_PAD_BUTTON_TRIANGLE    0x00001000
#define SCE_PAD_BUTTON_CIRCLE      0x00002000
#define SCE_PAD_BUTTON_CROSS       0x00004000
#define SCE_PAD_BUTTON_SQUARE      0x00008000
#define SCE_PAD_BUTTON_TOUCH_PAD   0x00100000
#define SCE_PAD_BUTTON_INTERCEPTED 0x80000000
#endif

#pragma pack(push, 1)
struct PS4PadDataFallback {
    uint32_t buttons;
    struct { uint8_t x, y; } leftStick;
    struct { uint8_t x, y; } rightStick;
    struct { uint8_t l2, r2; } analogButtons;
    struct { uint16_t pad; } padding;
    struct { float x, y, z, w; } orientation;
    struct { float x, y, z; } acceleration;
    struct { float x, y, z; } angularVelocity;
    struct {
        uint8_t touchNum;
        uint8_t pad[3];
        uint32_t timestamp;
        struct { uint16_t x, y; uint8_t id, pad[3]; } touch[SCE_PAD_MAX_TOUCH_NUM];
    } touchData;
    bool connected;
    uint64_t timestamp;
    uint32_t extensionUnitData;
    uint8_t connectedCount;
    uint8_t reserve[3];
    uint8_t deviceUniqueData[12];
};

struct PS4VibrationParamFallback {
    uint8_t largeMotor;
    uint8_t smallMotor;
};
#pragma pack(pop)

extern "C" {
int scePadInit(void);
int scePadOpen(int, int, int, const void*);
int scePadClose(int);
int scePadReadState(int, void*);
int scePadSetVibration(int, const void*);
int sceUserServiceInitialize(const void*);
int sceUserServiceGetInitialUser(int* userId);

#if !defined(__ORBIS__) && !defined(PS4) && !defined(__PS4__)
// Weak symbols for link compatibility in non-PS4 stubs
__attribute__((weak)) int scePadInit(void) { return 0; }
__attribute__((weak)) int scePadOpen(int, int, int, const void*) { return 1; }
__attribute__((weak)) int scePadClose(int) { return 0; }
__attribute__((weak)) int scePadReadState(int, void*) { return 0; }
__attribute__((weak)) int scePadSetVibration(int, const void*) { return 0; }
__attribute__((weak)) int sceUserServiceInitialize(const void*) { return 0; }
__attribute__((weak)) int sceUserServiceGetInitialUser(int* userId) { if (userId) *userId = 0x10000000; return 0; }
#endif
}

namespace openwheels {
namespace ps4 {
namespace {

int g_padHandle = -1;
int g_userId = 0x10000000;
bool g_sensorEnabled = false;
float g_lastTiltAngle = 0.0f;

float deadzone(float v, float dz = 0.15f)
{
    if (std::abs(v) < dz) return 0.0f;
    return (v > 0.0f) ? (v - dz) / (1.0f - dz) : (v + dz) / (1.0f - dz);
}

void splitStick(float* control, int up, float x, float y)
{
    x = deadzone(x);
    y = deadzone(y);
    control[up]     = (y < 0.0f) ? -y : 0.0f; // up
    control[up + 1] = (y > 0.0f) ?  y : 0.0f; // down
    control[up + 2] = (x < 0.0f) ? -x : 0.0f; // left
    control[up + 3] = (x > 0.0f) ?  x : 0.0f; // right
}

void pollPad(std::vector<pad::State>& states)
{
    if (g_padHandle < 0) return;

    PS4PadDataFallback padData;
    std::memset(&padData, 0, sizeof(padData));

    int ret = scePadReadState(g_padHandle, &padData);
    if (ret != 0 || !padData.connected) return;

    pad::State s;
    s.id = 0;
    s.name = "DualShock 4";
    float* c = s.control;

    const uint32_t btn = padData.buttons;

    // Face buttons (PS layout: Cross = A, Circle = B, Square = X, Triangle = Y)
    c[pad::kA] = (btn & SCE_PAD_BUTTON_CROSS)    ? 1.0f : 0.0f;
    c[pad::kB] = (btn & SCE_PAD_BUTTON_CIRCLE)   ? 1.0f : 0.0f;
    c[pad::kX] = (btn & SCE_PAD_BUTTON_SQUARE)   ? 1.0f : 0.0f;
    c[pad::kY] = (btn & SCE_PAD_BUTTON_TRIANGLE) ? 1.0f : 0.0f;

    // Bumpers
    c[pad::kLB] = (btn & SCE_PAD_BUTTON_L1) ? 1.0f : 0.0f;
    c[pad::kRB] = (btn & SCE_PAD_BUTTON_R1) ? 1.0f : 0.0f;

    // Triggers (Analog priority, digital fallback)
    float lt = (float)padData.analogButtons.l2 / 255.0f;
    float rt = (float)padData.analogButtons.r2 / 255.0f;
    if (lt <= 0.0f && (btn & SCE_PAD_BUTTON_L2)) lt = 1.0f;
    if (rt <= 0.0f && (btn & SCE_PAD_BUTTON_R2)) rt = 1.0f;
    c[pad::kLT] = lt;
    c[pad::kRT] = rt;

    // System buttons
    c[pad::kStart] = (btn & SCE_PAD_BUTTON_OPTIONS) ? 1.0f : 0.0f;
    c[pad::kBack]  = (btn & (SCE_PAD_BUTTON_TOUCH_PAD)) ? 1.0f : 0.0f;

    // Stick clicks
    c[pad::kLS] = (btn & SCE_PAD_BUTTON_L3) ? 1.0f : 0.0f;
    c[pad::kRS] = (btn & SCE_PAD_BUTTON_R3) ? 1.0f : 0.0f;

    // D-Pad
    c[pad::kDUp]    = (btn & SCE_PAD_BUTTON_UP)    ? 1.0f : 0.0f;
    c[pad::kDDown]  = (btn & SCE_PAD_BUTTON_DOWN)  ? 1.0f : 0.0f;
    c[pad::kDLeft]  = (btn & SCE_PAD_BUTTON_LEFT)  ? 1.0f : 0.0f;
    c[pad::kDRight] = (btn & SCE_PAD_BUTTON_RIGHT) ? 1.0f : 0.0f;

    // Analog sticks (0..255, 128 = center; y: 0 is up, 255 is down)
    const float lx = ((float)padData.leftStick.x - 128.0f) / 128.0f;
    const float ly = ((float)padData.leftStick.y - 128.0f) / 128.0f;
    splitStick(c, pad::kLUp, lx, ly);

    const float rx = ((float)padData.rightStick.x - 128.0f) / 128.0f;
    const float ry = ((float)padData.rightStick.y - 128.0f) / 128.0f;
    splitStick(c, pad::kRUp, rx, ry);

    states.push_back(std::move(s));

    // Tilt / Gyro steering sensor
    if (g_sensorEnabled)
    {
        // DualShock 4 accelerometer X axis gives roll tilt
        float ax = padData.acceleration.x;
        if (std::abs(ax) > 0.05f)
        {
            g_lastTiltAngle = std::max(-1.0f, std::min(1.0f, ax * 1.5f));
        }
        else
        {
            g_lastTiltAngle = 0.0f;
        }
    }
}

void setRumble(float strength, float seconds)
{
    (void)seconds;
    if (g_padHandle < 0) return;

    PS4VibrationParamFallback param;
    param.largeMotor = (uint8_t)std::max(0.0f, std::min(255.0f, strength * 255.0f));
    param.smallMotor = (uint8_t)std::max(0.0f, std::min(255.0f, strength * 0.7f * 255.0f));

    scePadSetVibration(g_padHandle, &param);
}

}  // namespace

void initGamepad()
{
    sceUserServiceInitialize(nullptr);
    sceUserServiceGetInitialUser(&g_userId);

    scePadInit();
    g_padHandle = scePadOpen(g_userId, SCE_PAD_PORT_TYPE_STANDARD, 0, nullptr);
}

void shutdownGamepad()
{
    if (g_padHandle >= 0)
    {
        scePadClose(g_padHandle);
        g_padHandle = -1;
    }
}

}  // namespace ps4

namespace pad {

void installPlatformProvider()
{
    ps4::initGamepad();
    setPoll(ps4::pollPad);
    setRumbleOutput(ps4::setRumble);
}

}  // namespace pad

namespace tilt {

void enableSensor(bool on)
{
    ps4::g_sensorEnabled = on;
}

}  // namespace tilt

}  // namespace openwheels
