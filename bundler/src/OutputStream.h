//
// Created by Duzhong Chen on 2020/7/14.
//

#pragma once

#include "Utils.h"

namespace jetpack {

    class OutputStream {
    public:

        virtual OutputStream& operator<<(const char16_t* str) = 0;
        virtual OutputStream& operator<<(const UString& str) = 0;

    };

}
