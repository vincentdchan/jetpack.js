//
// Created by Duzhong Chen on 2020/7/14.
//

#pragma once

#include <sstream>
#include "Utils.h"
#include "string/UString.h"

namespace jetpack {

    class OutputStream {
    public:

        virtual OutputStream& operator<<(const char16_t* str) = 0;
        virtual OutputStream& operator<<(const UString& str) = 0;
        virtual OutputStream& operator<<(char ch) = 0;

        virtual void Close() {}

        virtual ~OutputStream() = default;

    };

    class MemoryOutputStream : public OutputStream {
    public:

        MemoryOutputStream();
        MemoryOutputStream(const MemoryOutputStream&) = delete;
        MemoryOutputStream(MemoryOutputStream&&) = delete;

        MemoryOutputStream& operator=(const MemoryOutputStream&) = delete;
        MemoryOutputStream& operator=(MemoryOutputStream&&) = delete;

        OutputStream& operator<<(const char16_t* str) override;
        OutputStream& operator<<(const UString& str) override;
        OutputStream& operator<<(char ch) override;

        OutputStream& Write(const char16_t* str, std::uint32_t size);

        [[nodiscard]] UString ToString() const;

        [[nodiscard]] std::string ToUTF8() const;

        ~MemoryOutputStream() override;

    protected:
        UString data_;

    };

    class FileOutputStream : public MemoryOutputStream {
    public:
        FileOutputStream(const std::string& path);

        void Close() override;

    private:
        std::string path_;

    };


}
