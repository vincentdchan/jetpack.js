//
// Created by Duzhong Chen on 2021/3/30.
//

#include <fmt/format.h>
#include "SourceMapDecoder.h"
#include "utils/Utils.h"
#include "SourceMapGenerator.h"

namespace jetpack {

    static void DumpBufferToResult(uint32_t line, const std::string& buffer, SourceMapDecoder::Result& result) {
        if (unlikely(buffer.empty())) {
            return;
        }

        const char* str = buffer.c_str();
        int after_column = SourceMapGenerator::VLQToInt(str, str);
        int source_index = SourceMapGenerator::VLQToInt(str, str);
        int before_line = SourceMapGenerator::VLQToInt(str, str);
        int before_column = SourceMapGenerator::VLQToInt(str, str);
//        int names_index = -1;
//        if (str < buffer.c_str() + buffer.size()) {
//            names_index = SourceMapGenerator::VLQToInt(str, str);
//        }

        SourceMapDecoder::ResultMapping mapping {
            static_cast<uint32_t>(source_index),
            before_line,
            before_column,
            static_cast<int32_t>(line),
            after_column,
        };
        result.content.push_back(mapping);
    }

    std::string SourceMapDecoder::ResultMapping::ToString() const {
        return fmt::format("fileIndex: {} before: {}:{} after: {}:{}", source_index,
                           before_line, before_column,
                           after_line, after_column);
    }

    SourceMapDecoder::Result
    SourceMapDecoder::Decode() {
        Result result;
        auto mappings = sourcemap_json["mappings"].get<std::string>();

        uint32_t line_counter = 1;

        std::string buffer;

        for (char ch : mappings) {
            switch (ch) {
                case ',':
                    DumpBufferToResult(line_counter, buffer, result);
                    buffer.clear();
                    break;

                case ';':
                    DumpBufferToResult(line_counter, buffer, result);
                    line_counter++;
                    buffer.clear();
                    break;

                default:
                    buffer.push_back(ch);
                    break;

            }
        }


        return result;
    }

}
