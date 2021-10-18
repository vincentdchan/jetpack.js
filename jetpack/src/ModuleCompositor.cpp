//
// Created by Duzhong Chen on 2021/3/30.
//

#include "ModuleCompositor.h"

namespace jetpack {

    ModuleCompositor& ModuleCompositor::append(const std::string &content, const Sp<MappingCollector>& mappingCollector) {
        if (likely(mappingCollector)) {
            for (auto& item : mappingCollector->items_) {
                if (item.dist_line == 1) {  // first line
                    item.dist_column += column;
                }
                item.dist_line += line;
            }
            sourcemap_generator_.AddCollector(mappingCollector);
        }

        for (uint32_t i = 0; i < content.length(); i++) {
            if (unlikely(content.at(i) == '\n')) {
                line++;
                column = 0;
            } else {
                column++;
            }
        }

        result_.append(content);

        return *this;
    }

}