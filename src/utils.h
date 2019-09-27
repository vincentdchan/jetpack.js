//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <locale>
#include <codecvt>
#include <string>
#include <chrono>

typedef std::u16string UString;

namespace utils {

    inline int64_t GetCurrentMs() {
        using namespace std::chrono;
        milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
        );
        return ms.count();
    }

    inline UString FromCodePoint(char32_t cp) {
        UString result;
        if (cp < 0x10000) {
            result.push_back(static_cast<char16_t>(cp));
        } else {
            result.push_back(static_cast<char16_t>(0xD800 + ((cp - 0x10000) >> 10)));
            result.push_back(static_cast<char16_t>(0xdc00 + ((cp - 0x10000) & 1023)));
        }
        return result;
    }

    inline std::u16string To_UTF16(const std::string &s) {
        std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
        return conv.from_bytes(s);
    }

    inline std::string To_UTF8(const std::u16string &s) {
        std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
        return conv.to_bytes(s);
    }

    inline std::string To_UTF8(const std::u32string &s) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(s);
    }

    inline void AddU32ToUtf16(UString& target, char32_t code) {
        if (code < 0x10000) {
            target.push_back(code);
        }

        std::u32string tmp;
        tmp.push_back(code);

        auto utf8 = To_UTF8(tmp);
        auto utf16 = To_UTF16(utf8);

        target.insert(target.end(), utf16.begin(), utf16.end());
    }

    inline bool IsLineTerminator(char32_t cp) {
        return (cp == 0x0A) || (cp == 0x0D) || (cp == 0x2028) || (cp == 0x2029);
    }

    static char32_t WHITE_SPACE[] = {0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202F, 0x205F, 0x3000, 0xFEFF};

    inline bool IsWhiteSpace(char32_t cp) {
        if (cp >= 0x1680) {
            std::size_t count = sizeof(WHITE_SPACE) / sizeof(char32_t);
            for (std::size_t i = 0; i < count; i++) {
                if (WHITE_SPACE[i] == cp) return true;
            }
        }
        return (cp == 0x20) || (cp == 0x09) || (cp == 0x0B) || (cp == 0x0C) || (cp == 0xA0);
    }

    inline bool IsIdentifierStart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); // && Regex.NonAsciiIdentifierStart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsIdentifierPart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp >= 0x30 && cp <= 0x39) ||         // 0..9
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); //&& Regex.NonAsciiIdentifierPart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsDecimalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39);      // 0..9
    }

    inline bool IsHexDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39) ||    // 0..9
        (cp >= 0x41 && cp <= 0x46) ||       // A..F
        (cp >= 0x61 && cp <= 0x66);         // a..f
    }

    inline bool IsOctalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x37);      // 0..7
    }

}
