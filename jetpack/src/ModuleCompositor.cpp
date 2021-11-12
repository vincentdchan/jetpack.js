//
// Created by Duzhong Chen on 2021/3/30.
//

#include "ModuleCompositor.h"

namespace jetpack {

    ModuleCompositor& ModuleCompositor::append(const std::string &content, const Sp<MappingCollector>& mapping_collector) {
        if (likely(mapping_collector)) {
            for (auto& item : mapping_collector->items_) {
                if (item.dist_line == 1) {  // first line
                    item.dist_column += column;
                }
                item.dist_line += line;
            }
            sourcemap_generator_.AddCollector(mapping_collector);
        }

        for (uint32_t i = 0; i < content.length(); i++) {
            if (unlikely(content.at(i) == '\n')) {
                line++;
                column = 0;
            } else {
                column++;
            }
        }

        result.append(content);

        return *this;
    }

}