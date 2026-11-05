#ifndef __CC_FILEUTILS_PS4_H__
#define __CC_FILEUTILS_PS4_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_PS4

#include "platform/CCFileUtils.h"
#include <string>

NS_CC_BEGIN

class CC_DLL FileUtilsPS4 : public FileUtils
{
    friend class FileUtils;
protected:
    FileUtilsPS4();
    std::string _writablePath;
public:
    bool init() override;
    std::string getWritablePath() const override;
    void setWritablePath(const std::string& path) override;
private:
    bool isFileExistInternal(const std::string& strFilePath) const override;
};

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_PS4
#endif // __CC_FILEUTILS_PS4_H__
