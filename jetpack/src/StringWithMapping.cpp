//
// Created by Duzhong Chen on 2021/9/8.
//

#include "StringWithMapping.h"

namespace jetpack {

    StringWithMapping::StringWithMapping(Up<MemoryViewOwner> content): data_(std::move(content)) {
        mapping_.resize(data_->View().size(), 0);
    }

}
