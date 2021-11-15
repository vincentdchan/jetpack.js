//
// Created by Duzhong Chen on 2021/3/30.
//

#include "ModuleCompositor.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace jetpack {

    void ModuleCompositor::AddSnippet(const std::string &content) {
        std::vector<std::string> lines;
        boost::split(lines, content, boost::is_any_of("\n"), boost::token_compress_on);
        for (const auto& line : lines) {
            Write(line);
            WriteLineEnd();
        }
    }

    void ModuleCompositor::Write(const std::string& content) {
        result_ += content;
        column_ += UTF16LenOfUtf8(content);
    }

    void ModuleCompositor::WriteLineEnd() {
        if (!config_.minify) {
            result_ += config_.line_end;
            line_++;
            column_ = 0;
        }
    }

    ModuleCompositor& ModuleCompositor::append(const CodeGenFragment& fragment) {
        for (auto& item : fragment.mapping_items) {
            if (item.dist_line == 1) {  // first line
                item.dist_column += column_;
            }
            item.dist_line += line_;
        }
//        sourcemap_generator_.AddCollector(mapping_collector);

        for (uint32_t i = 0; i < content.length(); i++) {
            if (unlikely(content.at(i) == '\n')) {
                line_++;
                column_ = 0;
            } else {
                column_++;
            }
        }

        result_.append(content);

        return *this;
    }

}