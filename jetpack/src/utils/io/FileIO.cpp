//
// Created by Duzhong Chen on 2021/3/28.
//

#include "FileIO.h"
#include <iostream>
#if defined(_WIN32)
#include <fstream>
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

namespace jetpack::io {

    struct MappedFileReader {
    public:
        MappedFileReader() = default;

        inline IOError Open(const std::string& filename) {
#ifdef _WIN32
            HANDLE hFile = ::CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "open file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::OpenFailed;
            }

            DWORD file_size;
            ::GetFileSize(hFile, &file_size);
            size = int64_t(file_size);

            hMapping = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, file_size, NULL);
            if (hMapping == NULL) {
                ::CloseHandle(hFile);
                std::cerr << "read file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::ReadFailed;
            }

            void* p = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
            if (p == NULL) {
                ::CloseHandle(hMapping);
                ::CloseHandle(hFile);
                std::cerr << "read file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::ReadFailed;
            }
            mapped_mem = reinterpret_cast<intptr_t>(p);
            return IOError::Ok;
#else
            fd = ::open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                return IOError::OpenFailed;
            }

            struct stat st;
            int ec = ::fstat(fd, &st);
            J_ASSERT(ec == 0);

            size = st.st_size;

            mapped_mem = (intptr_t)::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
            if (reinterpret_cast<void*>(mapped_mem) == MAP_FAILED) {
                std::cerr << "map file failed: " << strerror(errno) << std::endl;
                return IOError::ReadFailed;
            }

            return IOError::Ok;
#endif
        }

        inline int64_t FileSize() const {
            return size;
        }

        inline const char* Data() const {
            return reinterpret_cast<char*>(mapped_mem);
        }

        ~MappedFileReader() noexcept {
#ifdef _WIN32
            ::CloseHandle(hMapping);
            ::UnmapViewOfFile(hFile);
            ::CloseHandle(hFile);
#else
            if (likely(mapped_mem != -1)) {
                ::munmap(reinterpret_cast<void*>(mapped_mem), size);
                mapped_mem = -1;
            }
            if (likely(fd >= 0)) {
                ::close(fd);
                fd = -1;
            }
#endif
        }

    private:
        int64_t  size = -1;
        intptr_t mapped_mem = -1;
#ifdef _WIN32
        HANDLE hMapping = INVALID_HANDLE_VALUE;
        HANDLE hFile = INVALID_HANDLE_VALUE;
#else
        int      fd = -1;
#endif

    };

    IOError ReadFileToStdString(const std::string& filename, std::string& result) {
        MappedFileReader reader;
        IOError error = reader.Open(filename);
        if (error != IOError::Ok) {
            return error;
        }

        result = std::string(reader.Data(), reader.FileSize());

        return IOError::Ok;
    }

    const char* IOErrorToString(IOError err) {
        switch (err) {
            case IOError::WriteFailed:
                return "WriteFailed";

            case IOError::ReadFailed:
                return "ReadFailed";

            case IOError::OpenFailed:
                return "OpenFailed";

            default:
                return "Ok";

        }
    }

    IOError WriteBufferToPath(const std::string& filename, const char* buffer, int64_t size) {
#if defined(_WIN32)
        HANDLE hFile = ::CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "open file failed: " << filename << ", " << ::GetLastError() << std::endl;
            return IOError::OpenFailed;
        }

        HANDLE hMapping = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, size, NULL);
        if (hMapping == NULL) {
            ::CloseHandle(hFile);
            std::cerr << "write file failed: " << filename << ", " << ::GetLastError() << std::endl;
            return IOError::WriteFailed;
        }

        void* p = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
        if (p == NULL) {
            ::CloseHandle(hMapping);
            ::CloseHandle(hFile);
            std::cerr << "write file failed: " << filename << ", " << ::GetLastError() << std::endl;
            return IOError::WriteFailed;
        }

        ::memcpy(p, buffer, size);

        ::UnmapViewOfFile(p);
        ::CloseHandle(hMapping);
        ::CloseHandle(hFile);

        return IOError::Ok;
#else
        int fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd < 0) {
            std::cerr << "open file failed: " << strerror(errno) << std::endl;
            return IOError::OpenFailed;
        }

        int ec = ::ftruncate(fd, size);
        J_ASSERT(ec >= 0);

        char* mappedData = (char*)::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mappedData == MAP_FAILED) {
            ::close(fd);
            std::cerr << "map file failed: " << strerror(errno) << std::endl;
            return IOError::WriteFailed;
        }

        ::memcpy(mappedData, buffer, size);

        ::munmap(mappedData, size);
        ::close(fd);

        return IOError::Ok;
#endif
    }

}
