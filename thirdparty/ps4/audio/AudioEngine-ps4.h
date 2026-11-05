#ifndef __AUDIO_ENGINE_PS4_H__
#define __AUDIO_ENGINE_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include <string>
#include <functional>

namespace cocos2d {
namespace experimental {

class AudioEngineImpl
{
public:
    AudioEngineImpl() {}
    ~AudioEngineImpl() {}

    bool init() { return true; }
    int play2d(const std::string& file, bool loop, float volume) { (void)file; (void)loop; (void)volume; return 1; }
    void setLoop(int id, bool loop) { (void)id; (void)loop; }
    void setVolume(int id, float volume) { (void)id; (void)volume; }
    void pause(int id) { (void)id; }
    void resume(int id) { (void)id; }
    void stop(int id) { (void)id; }
    void stopAll() {}
    void uncache(const std::string& path) { (void)path; }
    void uncacheAll() {}
    float getDuration(int id) { (void)id; return 0.0f; }
    bool setCurrentTime(int id, float time) { (void)id; (void)time; return true; }
    float getCurrentTime(int id) { (void)id; return 0.0f; }
    void setFinishCallback(int id, const std::function<void(int, const std::string&)>& cb) { (void)id; (void)cb; }
    void preload(const std::string& path, const std::function<void(bool)>& cb) { (void)path; if (cb) cb(true); }
    void update(float dt) { (void)dt; }
};

} // namespace experimental
} // namespace cocos2d

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __AUDIO_ENGINE_PS4_H__
