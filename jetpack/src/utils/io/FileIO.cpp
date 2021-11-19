//
// Created by Duzhong Chen on 2021/3/28.
//

#include <iostream>
#include <fmt/format.h>
#include "FileIO.h"
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
            mapped_mem = reinterpret_cast<uintptr_t>(p);
            return IOError::Ok;
#else
            fd = ::open(filename.c_str(), O_RDONLY);
            if (fd < 0) {
                std::cerr << fmt::format("open file {} failed: {}", filename, strerror(errno)) << std::endl;
                return IOError::OpenFailed;
            }

            struct stat st;
            int ec = ::fstat(fd, &st);
            J_ASSERT(ec == 0);

            size = st.st_size;

            mapped_mem = (uintptr_t)::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
            if (reinterpret_cast<void*>(mapped_mem) == MAP_FAILED) {
                std::cerr << fmt::format("map file {} failed: {}", filename, strerror(errno)) << std::endl;
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
        uintptr_t mapped_mem = 0;
#ifdef _WIN32
        HANDLE hMapping = INVALID_HANDLE_VALUE;
        HANDLE hFile = INVALID_HANDLE_VALUE;
#else
        int      fd = -1;
#endif

    };

    class FileWriterInternal {
    public:
        FileWriterInternal(const std::string& path);

        inline IOError Error() const {
            return error_;
        }

        IOError Open();

        IOError Resize(uint64_t size);

        IOError Write(const char* bytes, size_t len);

        IOError WriteByte(unsigned char ch);

        ~FileWriterInternal();

    private:
        IOError EnsureSize(uint64_t);

        std::string path_;
        uint64_t offset_ = 0;
        uint64_t current_size_ = 0;
        unsigned char* mapped_mem_ = nullptr;
#ifdef _WIN32
        HANDLE hMapping = INVALID_HANDLE_VALUE;
        HANDLE hFile = INVALID_HANDLE_VALUE;
#else
        int      fd = -1;
#endif
        IOError error_ = IOError::Ok;

    };

    // 8k buffer
    constexpr uint64_t FILE_SIZE_INCR = 512 * 1024;

    IOError FileWriterInternal::Open() {
#ifdef _WIN32
            hFile = ::CreateFileA(path_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "open file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::OpenFailed;
            }

            DWORD file_size;
            ::GetFileSize(hFile, &file_size);
            size = int64_t(file_size);

            hMapping = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, size, NULL);
            if (hMapping == NULL) {
                ::CloseHandle(hFile);
                std::cerr << "read file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::ReadFailed;
            }

            void* p = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
            if (p == NULL) {
                ::CloseHandle(hMapping);
                ::CloseHandle(hFile);
                std::cerr << "read file failed: " << filename << ", " << ::GetLastError() << std::endl;
                return IOError::ReadFailed;
            }
            mapped_mem = reinterpret_cast<uintptr_t>(p);
            return IOError::Ok;
#else
        fd = ::open(path_.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd < 0) {
            std::cerr << fmt::format("open file {} failed: {}", path_, strerror(errno)) << std::endl;
            return IOError::OpenFailed;
        }

        struct stat st;
        int ec = ::fstat(fd, &st);
        J_ASSERT(ec == 0);

        current_size_ = st.st_size;
        if (auto ret = EnsureSize(FILE_SIZE_INCR); ret != IOError::Ok) {
            return ret;
        }

        mapped_mem_ = reinterpret_cast<unsigned char*>(::mmap(nullptr, current_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (mapped_mem_ == MAP_FAILED) {
            std::cerr << fmt::format("map file {} failed: {}", path_, strerror(errno)) << std::endl;
            return IOError::ReadFailed;
        }

        return IOError::Ok;
#endif
    }

    IOError FileWriterInternal::Resize(uint64_t size) {
        ::munmap(mapped_mem_, current_size_);

        if (::ftruncate(fd, size) != 0) {
            std::cerr << fmt::format("resize file {} failed: {}", path_, strerror(errno)) << std::endl;
            return IOError::ResizeFailed;
        }
        current_size_ = size;

        mapped_mem_ = reinterpret_cast<unsigned char*>(::mmap(nullptr, current_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (mapped_mem_ == MAP_FAILED) {
            std::cerr << fmt::format("map file {} failed: {}", path_, strerror(errno)) << std::endl;
            return IOError::ReadFailed;
        }

        return IOError::Ok;
    }

    IOError FileWriterInternal::EnsureSize(uint64_t size) {
        if (current_size_ >= size) {
            return IOError::Ok;
        }

        uint64_t need_size = current_size_;
        while (need_size < size) {
            need_size += FILE_SIZE_INCR;
        }

        return Resize(need_size);
    }

    IOError FileWriterInternal::Write(const char *bytes, size_t len) {
        IOError err = EnsureSize(offset_ + len);
        if (err != IOError::Ok) {
            return err;
        }
        memcpy(mapped_mem_ + offset_, bytes, len);
        offset_ += len;
        return IOError::Ok;
    }

    IOError FileWriterInternal::WriteByte(unsigned char ch) {
        IOError err = EnsureSize(offset_ + 1);
        if (err != IOError::Ok) {
            return err;
        }
        mapped_mem_[offset_++] = ch;
        return IOError::Ok;
    }

    FileWriterInternal::~FileWriterInternal() {
#ifdef _WIN32
        ::CloseHandle(hMapping);
            ::UnmapViewOfFile(hFile);
            ::CloseHandle(hFile);
#else
        if (likely(mapped_mem_ != nullptr)) {
            ::munmap(mapped_mem_, current_size_);
            mapped_mem_ = nullptr;
        }
        if (likely(fd >= 0)) {
            ::ftruncate(fd, offset_);
            ::close(fd);
            fd = -1;
        }
#endif
    }

    FileWriterInternal::FileWriterInternal(const std::string& path): path_(path) {
    }

    void FileWriterInternalDeleter::operator()(FileWriterInternal *d) {
        delete d;
    }

    FileWriter::FileWriter(const std::string& path) {
        auto ptr = new FileWriterInternal(path);
        d_ = std::unique_ptr<FileWriterInternal, FileWriterInternalDeleter>(ptr);
    }

    IOError FileWriter::Open() {
        return d_->Open();
    }

    IOError FileWriter::Write(const char *bytes, size_t len) {
        return d_->Write(bytes, len);
    }

    IOError FileWriter::WriteByte(unsigned char ch) {
        return d_->WriteByte(ch);
    }

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
            case IOError::ResizeFailed:
                return "ResizeFailed";

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
        FileWriter writer(filename);
        IOError err = writer.Open();
        if (err != IOError::Ok) {
            return err;
        }
        return writer.Write(buffer, size);
    }

}
