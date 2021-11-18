//
// Created by Duzhong Chen on 2021/3/30.
//

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "ModuleCompositor.h"
#include "Benchmark.h"

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
        d_.content += content;
        d_.column += UTF16LenOfUtf8(content);
    }

    void ModuleCompositor::WriteLineEnd() {
        if (!config_.minify) {
            d_.content += config_.line_end;
            d_.line++;
            d_.column = 0;
        }
    }

    ModuleCompositor& ModuleCompositor::Append(const CodeGenFragment& fragment) {
        const auto copy_column = d_.column;
        const auto copy_line = d_.line;
        thread_pool_.enqueue([this, fragment, copy_column, copy_line] {
            for (const auto& item : fragment.mapping_items) {
                auto item_copy = item;
                if (item_copy.dist_line == 1) {  // first line
                    item_copy.dist_column += copy_column;
                }
                item_copy.dist_line += copy_line - 1;
                d_.mapping_items.push_back(item_copy);
            }
        });

        d_.line += fragment.line - 1;
        d_.content += fragment.content;

        if (fragment.line > 1) {
            d_.column = fragment.column;
        } else {
            d_.column += fragment.column;
        }

        return *this;
    }

    std::future<void> ModuleCompositor::DumpSourcemap(Sp<SourceMapGenerator> sg, std::string path) {
        return thread_pool_.enqueue([this, sg, path] {
            benchmark::BenchMarker sourcemap_marker(benchmark::BENCH_FINALIZE_SOURCEMAP);
            sg->Finalize(make_slice(d_.mapping_items));
            sourcemap_marker.Submit();
            sg->DumpFile(path);
        });
    }

}