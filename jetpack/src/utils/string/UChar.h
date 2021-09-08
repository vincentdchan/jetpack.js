//
// Created by Duzhong Chen on 2021/3/25.
//

#ifndef ROCKET_BUNDLE_UCHAR_H
#define ROCKET_BUNDLE_UCHAR_H

namespace UChar {

    inline bool IsDecimalDigit(char32_t cp) {
        return (cp >= U'0' && cp <= U'9');
    }

    inline bool IsLineTerminator(char32_t cp) {
        return (cp == U'\n') || (cp == U'\r') || (cp == 0x2028) || (cp == 0x2029);
    }

    static char32_t WHITE_SPACE[] = {0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202F, 0x205F, 0x3000, 0xFEFF};

    inline bool IsWhiteSpace(char32_t cp) {
        if (cp >= 0x1680) {
            std::size_t count = sizeof(WHITE_SPACE) / sizeof(char32_t);
            for (std::size_t i = 0; i < count; i++) {
                if (WHITE_SPACE[i] == cp) return true;
            }
        }
        return (cp == U' ') || (cp == U'\t') || (cp == 0x0B) || (cp == 0x0C) || (cp == 0xA0);
    }

    inline bool IsIdentifierStart(char32_t cp) {
        return (cp == U'$') || (cp == U'_') ||
               (cp >= U'A' && cp <= U'Z') ||
               (cp >= U'a' && cp <= U'z') ||
               (cp == U'\\') ||
               ((cp >= 0x80)); // && Regex.NonAsciiIdentifierStart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsIdentifierPart(char32_t cp) {
        return (cp == U'$') || (cp == U'_') ||  // $ (dollar) and _ (underscore)
               (cp >= U'A' && cp <= U'Z') ||
               (cp >= U'a' && cp <= U'z') ||
               (cp >= U'0' && cp <= U'9') ||
               (cp == U'\\') ||
               ((cp >= 0x80)); //&& Regex.NonAsciiIdentifierPart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsHexDigit(char32_t cp) {
        return (cp >= U'0' && cp <= U'9') ||
               (cp >= U'A' && cp <= U'F') ||
               (cp >= U'a' && cp <= U'f');
    }

    inline bool IsOctalDigit(char32_t cp) {
        return (cp >= U'0' && cp <= U'7');      // 0..7
    }

}

#endif //ROCKET_BUNDLE_UCHAR_H
