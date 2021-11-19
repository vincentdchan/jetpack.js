//
// Created by Duzhong Chen on 2021/3/28.
//

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include "utils/string/UString.h"
#include "utils/MemoryViewOwner.h"

#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace jetpack::io {
    enum class IOError {
        ResizeFailed= -4,
        WriteFailed = -3,
        ReadFailed = -2,
        OpenFailed = -1,
        Ok = 0,
    };

    class Writer {
    public:
        inline IOError Write(std::string_view view) {
            return Write(view.data(), view.size());
        }

        inline IOError WriteS(const std::string& str) {
            return Write(str.c_str(), str.size());
        }

        virtual IOError Write(const char* bytes, size_t len) = 0;

        virtual ~Writer() = default;

    };

    class FileWriterInternal;

    struct FileWriterInternalDeleter {

        void operator()(FileWriterInternal* d);

    };

    class FileWriter : public Writer {
    public:
        FileWriter(const std::string& path);

        IOError Open();

        IOError Write(const char* bytes, size_t len) override;

        ~FileWriter() override = default;

    private:
        std::unique_ptr<FileWriterInternal, FileWriterInternalDeleter> d_;

    };

    const char* IOErrorToString(IOError);
    IOError ReadFileToStdString(const std::string& filename, std::string& result);

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
