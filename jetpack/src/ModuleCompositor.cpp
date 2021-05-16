//
// Created by Duzhong Chen on 2021/3/30.
//

#include "ModuleCompositor.h"

namespace jetpack {

    ModuleCompositor& ModuleCompositor::append(const UString &content, Sp<MappingCollector> mappingCollector) {
        if (likely(mappingCollector)) {
            for (auto& item : mappingCollector->items_) {
                if (item.dist_line == 1) {  // first line
                    item.dist_column += column;
                }
                item.dist_line += line;
            }
            sourceMapGenerator.AddCollector(std::move(mappingCollector));
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

    void ModuleCompositor::appendCommonJsCodeSnippet() {
        result += u"let __commonJS = (callback, module) => () => {\n"
                  "  if (!module) {\n"
                  "    module = {exports: {}};\n"
                  "    callback(module.exports, module);\n"
                  "  }\n"
                  "  return module.exports;\n"
                  "};";
    }

}
