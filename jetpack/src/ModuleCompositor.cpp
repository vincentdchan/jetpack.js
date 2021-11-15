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

    ModuleCompositor& ModuleCompositor::Append(const CodeGenFragment& fragment) {
        for (const auto& item : fragment.mapping_items) {
            auto item_copy = item;
            if (item_copy.dist_line == 1) {  // first line
                item_copy.dist_column += column_;
            }
            item_copy.dist_line += line_;
            mapping_items_.push_back(item_copy);
        }

        line_ += fragment.line - 1;
        result_ += fragment.content;

        if (fragment.line > 1) {
            column_ = fragment.column;
        } else {
            column_ += fragment.column;
        }

        return *this;
    }

}