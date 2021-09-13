//
// Created by chenduzhong on 2021/4/1.
//

#include "JetJSON.h"
#include <sstream>
#include <iomanip>

namespace jetpack {

    static std::string FormatByte(unsigned char value) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
           << static_cast<unsigned int>(value);
        return ss.str();
    }

    std::string EscapeJSONString(std::string_view str) {
        std::string m;
        m.reserve(str.size() * 2);

        for (size_t i = 0; i < str.size(); ++i) {
            const char ch = str[i];
            switch (ch) {
                case '/':
                    m += ch;
                    break;
                case '\\':
                case '"':
                    m += '\\';
                    m += ch;
                    break;
                case '\b':
                    m += "\\b";
                    break;
                case '\t':
                    m += "\\t";
                    break;
                case '\n':
                    m += "\\n";
                    break;
                case '\f':
                    m += "\\f";
                    break;
                case '\r':
                    m += "\\r";
                    break;
                default:
                    if (ch < ' ') {
                        m += "\\u00";
                        m += FormatByte(static_cast<unsigned char>(ch));
                    } else {
                        m += ch;
                    }
                    break;
            }
        }

        return m;
    }

}
