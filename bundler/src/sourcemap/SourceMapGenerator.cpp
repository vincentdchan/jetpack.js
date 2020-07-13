//
// Created by Duzhong Chen on 2020/7/13.
//

#include "./SourceMapGenerator.h"
#include <cstdlib>
#include <cstring>

#define IntToVLQBufferSize 8

namespace jetpack {
    using std::stringstream;
    using std::memset;

    static char Base64EncodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                         'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                         'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                         'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                         'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                         'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                         'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                         '4', '5', '6', '7', '8', '9', '+', '/'};

    bool SourceMapGenerator::GenerateVLQStr(stringstream& ss, int transformed_column,
            int file_index, int before_line, int before_column, int var_index) {

        if (!IntToVLQ(ss, transformed_column)) {
            return false;
        }

        if (!IntToVLQ(ss, file_index)) {
            return false;
        }

        if (!IntToVLQ(ss, before_line)) {
            return false;
        }

        if (!IntToVLQ(ss, before_column)) {
            return false;
        }

        return IntToVLQ(ss, var_index);
    }

    bool SourceMapGenerator::IntToVLQ(std::stringstream& ss, int code) {
        int buffer[IntToVLQBufferSize];

        if (code < 0) return false;
        int s1 = code << 1;
        int counter = 0;
        if (s1 > 0b11111) {
            memset(buffer, 0, IntToVLQBufferSize * sizeof(int));
            while (s1 > 0b11111) {
                buffer[counter++] = s1 & 0b11111;
                s1 >>= 5;
            }
            buffer[counter++] = s1;

            for (int i = 0; i < counter; i++) {
                if (i != counter - 1) {
                    buffer[i] |= 0b100000;
                }
                char ch = 0;
                if (!IntToBase64(buffer[i], ch)) {
                    return false;
                }
                ss << ch;
            }
        } else {
            char ch = 0;
            if (!IntToBase64(s1, ch)) {
                return false;
            }
            ss << buffer[0];
        }
        return true;
    }

    bool SourceMapGenerator::IntToBase64(int code, char &ch) {
        if (code < 0 || code > 0b111111) {
            return false;
        }
        ch = Base64EncodingTable[code];
        return true;
    }

    bool SourceMapGenerator::AddLocation(const std::string &name, int after_col, int file_index, int before_line, int before_col) {
        int var_index = names.size();
        names.push_back(name);

        return GenerateVLQStr(ss, after_col, file_index, before_line, before_col, var_index);
    }

}
