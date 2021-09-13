//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <nlohmann/json.hpp>
#include <vector>

namespace jetpack {

    class SourceMapDecoder {
    public:
        struct ResultMapping {
        public:
            uint32_t source_index;
            int32_t  before_line;
            int32_t  before_column;
            int32_t  after_line;
            int32_t  after_column;

            [[nodiscard]]
            std::string ToString() const;

        };

        struct Result {
        public:
            std::vector<ResultMapping> content;

        };

        inline explicit SourceMapDecoder(nlohmann::json& j) noexcept : sourcemap_json(j) {}

        Result Decode();

        ~SourceMapDecoder() noexcept = default;

    private:
        nlohmann::json& sourcemap_json;

        int l_after_column_ = 0;
        int l_source_index_ = 0;
        int l_before_line_ = 1;
        int l_before_column_ = 0;

        void DumpBufferToResult(uint32_t line, const std::string& buffer, SourceMapDecoder::Result& result);

    };

    inline bool operator==(const SourceMapDecoder::ResultMapping& lhs, const SourceMapDecoder::ResultMapping& rhs) {
        return ::memcmp(&lhs, &rhs, sizeof(SourceMapDecoder::ResultMapping)) == 0;
    }

}
