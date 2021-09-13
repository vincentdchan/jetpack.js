//
// Created by Duzhong Chen on 2021/3/30.
//

#include <fmt/format.h>
#include "SourceMapDecoder.h"
#include "utils/Common.h"
#include "SourceMapGenerator.h"

namespace jetpack {

    void SourceMapDecoder::DumpBufferToResult(uint32_t line, const std::string& buffer, SourceMapDecoder::Result& result) {
        if (unlikely(buffer.empty())) {
            return;
        }

        const char* str = buffer.c_str();
        l_after_column_ += SourceMapGenerator::VLQToInt(str, str);
        l_source_index_ += SourceMapGenerator::VLQToInt(str, str);
        l_before_line_ += SourceMapGenerator::VLQToInt(str, str);
        l_before_column_ += SourceMapGenerator::VLQToInt(str, str);
//        int names_index = -1;
//        if (str < buffer.c_str() + buffer.size()) {
//            names_index = SourceMapGenerator::VLQToInt(str, str);
//        }

        SourceMapDecoder::ResultMapping mapping {
            static_cast<uint32_t>(l_source_index_),
            l_before_line_,
            l_before_column_,
            static_cast<int32_t>(line),
            l_after_column_,
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
                    l_after_column_ = 0;
                    buffer.clear();
                    break;

                default:
                    buffer.push_back(ch);
                    break;

            }
        }

        if (!buffer.empty()) {
            DumpBufferToResult(line_counter, buffer, result);
        }

        return result;
    }

}
