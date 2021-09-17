//
// Created by Duzhong Chen on 2021/3/23.
//

#include "UString.h"

/*
 * This lookup table is used to help decode the first byte of
 * a multi-byte UTF8 character.
 * copy from SQLite3
 */
static const unsigned char Utf8Trans1[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = Utf8Trans1[c-0xc0];                         \
    while( zIn!=zTerm && (*zIn & 0xc0)==0x80 ){            \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
  }
char32_t ReadCodepointFromUtf8(const uint8_t buf[], uint32_t *idx, size_t strlen)
{
    char32_t c = buf[(*idx)++];
    if( c>=0xc0 ){
        c = Utf8Trans1[c-0xc0];
        while (*idx < strlen && (buf[*idx] & 0xc0)==0x80) {
            c = (c<<6) + (0x3f & buf[(*idx)++]);
        }
        if( c<0x80
            || (c&0xFFFFF800)==0xD800
            || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }
    }
    return c;
}

std::string StringFromCodePoint(char32_t cp) {
    return StringFromUtf32(&cp, 1);
}

#define U8(C) static_cast<uint8_t>(C)

std::string StringFromUtf32(const char32_t* content, std::size_t size) {
    std::string result;
    for (std::size_t i = 0; i < size; i++) {
        char32_t c = content[i];
        if( c<0x00080 ){
            result.push_back(U8(c&0xFF));
        }
        else if( c<0x00800 ){
            result.push_back(U8(0xC0) + (unsigned char)((c>>6)&0x1F));
            result.push_back(U8(0x80) + U8(c & 0x3F));
        }
        else if( c<0x10000 ){
            result.push_back(U8(0xE0) + U8((c>>12)&0x0F));
            result.push_back(U8(0x80) + U8((c>>6) & 0x3F));
            result.push_back(U8(0x80) + U8(c & 0x3F));
        }else{
            result.push_back(U8(0xF0) + U8((c>>18) & 0x07));
            result.push_back(U8(0x80) + U8((c>>12) & 0x3F));
            result.push_back(U8(0x800) + U8((c>>6) & 0x3F));
            result.push_back(U8(0x80) + U8(c & 0x3F));
        }
    }
    return result;
}

std::size_t UTF16LenOfUtf8(std::string_view u8) {
    const uint8_t* zIn = reinterpret_cast<const uint8_t*>(u8.data());
    const uint8_t* zTerm = zIn + u8.size();
    uint32_t c;
    std::size_t result = 0;
    while (zIn < zTerm) {
        READ_UTF8(zIn, zTerm, c);
        if (c <= 0xFFFF) {
            result++;
        } else {
            result += 2;
        }
    }
    return u8.size();
}
