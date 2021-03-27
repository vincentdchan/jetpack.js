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

    IOError ReadFileToUString(const std::string& filename, UString& result) {
#if defined(_WIN32)
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        result = UString::fromStdString(str);
        return IOError::Ok;
#else
        int fd = ::open(filename.c_str(), O_RDONLY);
        if (fd < 0) {
            return IOError::OpenFailed;
        }

        struct stat st;
        int ec = ::fstat(fd, &st);
        assert(ec == 0);

        char* mem = (char*)::mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED) {
            ::close(fd);
            std::cerr << "map file failed: " << strerror(errno) << std::endl;
            return IOError::ReadFailed;
        }

        result = UString::fromUtf8(mem, st.st_size);

        ::munmap(mem, st.st_size);
        ::close(fd);

        return IOError::Ok;
#endif
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
        int fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd < 0) {
            std::cerr << "open file failed: " << strerror(errno) << std::endl;
            return IOError::OpenFailed;
        }

        int ec = ::ftruncate(fd, size);
        assert(ec >= 0);

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
    }

}
