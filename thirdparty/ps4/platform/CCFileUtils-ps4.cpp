#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/ps4/CCFileUtils-ps4.h"
#include <sys/stat.h>
#include <unistd.h>

NS_CC_BEGIN

FileUtils* FileUtils::getInstance()
{
    if (s_sharedFileUtils == nullptr)
    {
        s_sharedFileUtils = new FileUtilsPS4();
        if (!s_sharedFileUtils->init())
        {
            delete s_sharedFileUtils;
            s_sharedFileUtils = nullptr;
        }
    }
    return s_sharedFileUtils;
}

FileUtilsPS4::FileUtilsPS4()
{
    _defaultResRootPath = "/app0/";
    _writablePath = "/data/OpenWheels/";
}

bool FileUtilsPS4::init()
{
    _defaultResRootPath = "/app0/";
    _writablePath = "/data/OpenWheels/";
    return FileUtils::init();
}

std::string FileUtilsPS4::getWritablePath() const
{
    struct stat st;
    if (stat(_writablePath.c_str(), &st) != 0)
    {
        mkdir(_writablePath.c_str(), 0777);
    }
    return _writablePath;
}

void FileUtilsPS4::setWritablePath(const std::string& path)
{
    _writablePath = path;
}

bool FileUtilsPS4::isFileExistInternal(const std::string& strFilePath) const
{
    if (strFilePath.empty())
    {
        return false;
    }
    std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    {
        strPath.insert(0, _defaultResRootPath);
    }
    struct stat sts;
    return (stat(strPath.c_str(), &sts) == 0);
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
