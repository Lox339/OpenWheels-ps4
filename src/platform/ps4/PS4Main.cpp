// PS4 Platform Entry Point for OpenWheels
#include "platform/ps4/PS4Main.h"
#include "platform/ps4/PS4Gamepad.h"

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "cocos2d.h"
#include "game/app/AppDelegate.h"

#if defined(__ORBIS__) || defined(PS4) || defined(__PS4__)
#if __has_include(<kernel.h>)
#include <kernel.h>
#endif
#if __has_include(<system_service.h>)
#include <system_service.h>
#endif
#if __has_include(<sysmodule.h>)
#include <sysmodule.h>
#endif
#if __has_include(<net.h>)
#include <net.h>
#endif
#endif

#ifndef SCE_SYSMODULE_NET
#define SCE_SYSMODULE_NET 0x0001
#endif
#ifndef SCE_SYSMODULE_AUDIO_OUT
#define SCE_SYSMODULE_AUDIO_OUT 0x0004
#endif
#ifndef SCE_SYSMODULE_USER_SERVICE
#define SCE_SYSMODULE_USER_SERVICE 0x0006
#endif
#ifndef SCE_SYSMODULE_PIGLET
#define SCE_SYSMODULE_PIGLET 0x0024
#endif

extern "C" {
#if !defined(__ORBIS__) && !defined(PS4) && !defined(__PS4__)
__attribute__((weak)) int sceSysmoduleLoadModule(uint16_t) { return 0; }
__attribute__((weak)) int sceSysmoduleUnloadModule(uint16_t) { return 0; }
__attribute__((weak)) int sceNetInit(void) { return 0; }
__attribute__((weak)) int sceNetPoolCreate(const char*, int, int) { return 0; }
#endif
}

USING_NS_CC;

namespace openwheels {
namespace ps4 {
namespace {

void createDirectoryRecursive(const std::string& path)
{
    std::string current;
    for (size_t i = 0; i < path.size(); ++i)
    {
        current += path[i];
        if (path[i] == '/' || i == path.size() - 1)
        {
            mkdir(current.c_str(), 0777);
        }
    }
}

void setupFilesystem()
{
    // On PS4 homebrew, /app0 is the read-only mounted package root.
    // /data/OpenWheels is the writable storage on the internal HDD.
    const std::string writableRoot = "/data/OpenWheels/";
    createDirectoryRecursive(writableRoot);
    createDirectoryRecursive(writableRoot + "online/");
    createDirectoryRecursive(writableRoot + "saves/");

    auto fileUtils = FileUtils::getInstance();
    fileUtils->setWritablePath(writableRoot);

    // Prioritized search paths
    fileUtils->addSearchPath(writableRoot, true);
    fileUtils->addSearchPath("/app0/assets/", true);
    fileUtils->addSearchPath("/app0/assets/shared/", true);
    fileUtils->addSearchPath("/app0/assets/sounds/", true);
    fileUtils->addSearchPath("/app0/generated/", true);
    fileUtils->addSearchPath("/app0/levels/restored/", true);
    fileUtils->addSearchPath("/app0/", true);

    // Fallback relative paths if running in an emulator or test environment
    fileUtils->addSearchPath("assets/");
    fileUtils->addSearchPath("assets/shared/");
    fileUtils->addSearchPath("assets/sounds/");
    fileUtils->addSearchPath("generated/");
    fileUtils->addSearchPath("levels/restored/");
}

void initSysmodules()
{
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_USER_SERVICE);
    sceSysmoduleLoadModule(SCE_SYSMODULE_AUDIO_OUT);
    sceSysmoduleLoadModule(SCE_SYSMODULE_PIGLET);

    // PS4 network memory pool initialization
    sceNetInit();
    sceNetPoolCreate("OWNetPool", 1024 * 1024, 0);
}

void cleanupSysmodules()
{
    ps4::shutdownGamepad();
    sceSysmoduleUnloadModule(SCE_SYSMODULE_PIGLET);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_AUDIO_OUT);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_USER_SERVICE);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

}  // namespace

int run(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    initSysmodules();
    setupFilesystem();

    // Create cocos2d Application and start the main loop
    AppDelegate app;
    int ret = Application::getInstance()->run();

    cleanupSysmodules();
    return ret;
}

}  // namespace ps4
}  // namespace openwheels

int main(int argc, char** argv)
{
    return openwheels::ps4::run(argc, argv);
}
