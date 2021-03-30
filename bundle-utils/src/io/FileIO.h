//
// Created by Duzhong Chen on 2021/3/28.
//

#ifndef ROCKET_BUNDLE_FILEIO_H
#define ROCKET_BUNDLE_FILEIO_H

#include <string>
#include "string/UString.h"

#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace jetpack::io {

    enum class IOError {
        WriteFailed = -3,
        ReadFailed = -2,
        OpenFailed = -1,
        Ok = 0,
    };

    const char* IOErrorToString(IOError);
    IOError ReadFileToStdString(const std::string& filename, std::string& result);
    IOError ReadFileToUString(const std::string& filename, UString& result);

    IOError WriteBufferToPath(const std::string& filename, const char* buffer, int64_t size);

    inline bool IsFileExist(const std::string& path) {
#ifndef _WIN32
        return access(path.c_str(), F_OK) == 0;
#else
        DWORD dwAttrib = GetFileAttributesA(path.c_str());

        return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
    }

}

#endif //ROCKET_BUNDLE_FILEIO_H
