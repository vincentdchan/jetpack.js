//
// Created by Duzhong Chen on 2020/7/14.
//

#pragma once

#include <fstream>
#include <sstream>
#include "Utils.h"

namespace jetpack {

    class OutputStream {
    public:

        virtual OutputStream& operator<<(const char16_t* str) = 0;
        virtual OutputStream& operator<<(const UString& str) = 0;
        virtual OutputStream& operator<<(char ch) = 0;

        virtual void Close() {}

        virtual ~OutputStream() = default;

    };

    class FileOutputStream : public OutputStream {
    public:

        FileOutputStream(const std::string& path);
        OutputStream& operator<<(const char16_t* str) override;
        OutputStream& operator<<(const UString& str) override;
        OutputStream& operator<<(char ch) override;

        void Close() override {
            ofs.close();
        }

    private:
        std::ofstream ofs;

    };

    class MemoryOutputStream : public OutputStream {
    public:

        MemoryOutputStream() = default;
        OutputStream& operator<<(const char16_t* str) override;
        OutputStream& operator<<(const UString& str) override;
        OutputStream& operator<<(char ch) override;
        MemoryOutputStream& operator<<(const char* str);
        MemoryOutputStream& operator<<(const std::string& str);

        std::string ToString() const;

    private:
        std::stringstream ss;

    };

}
