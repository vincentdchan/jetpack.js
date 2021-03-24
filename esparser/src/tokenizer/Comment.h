//
// Created by Duzhong Chen on 2019/10/10.
//

#pragma once

#include "Token.h"
#include <utility>

struct Comment {
    bool multi_line_;
    UString value_;
    std::pair<std::uint32_t, std::uint32_t> range_;
    SourceLocation loc_;

};
