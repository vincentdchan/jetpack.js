//
// Created by Duzhong Chen on 2021/3/29.
//

#include "MappingCollector.h"

namespace jetpack {

    void MappingCollector::AddMapping(const std::string &name, const SourceLocation &origin, int32_t column) {
        MappingItem item(name, origin, dist_line_, column);
        items_.push_back(item);
    }

}
