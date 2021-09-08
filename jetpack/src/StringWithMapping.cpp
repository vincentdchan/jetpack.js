//
// Created by Duzhong Chen on 2021/9/8.
//

#include "StringWithMapping.h"

Sp<StringWithMapping> StringWithMapping::Make(std::string &&content) {
    return std::make_shared<StringWithMapping>(std::move(content));
}

StringWithMapping::StringWithMapping(std::string &&content):
data_(std::move(content)) {
    mapping_.resize(content.size(), 0);
}
