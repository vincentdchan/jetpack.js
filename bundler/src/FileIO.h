//
// Created by Duzhong Chen on 2021/3/28.
//

#ifndef ROCKET_BUNDLE_FILEIO_H
#define ROCKET_BUNDLE_FILEIO_H

#include <string>
#include "string/UString.h"

namespace jetpack::io {

    enum class IOError {
        WriteFailed = -3,
        ReadFailed = -2,
        OpenFailed = -1,
        Ok = 0,
    };

    const char* IOErrorToString(IOError);
    IOError ReadFileToUString(const std::string& filename, UString& result);

    IOError WriteBufferToPath(const std::string& filename, const char* buffer, int64_t size);

}

#endif //ROCKET_BUNDLE_FILEIO_H
