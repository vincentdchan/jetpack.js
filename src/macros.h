//
// Created by Duzhong Chen on 2019/9/4.
//

#pragma once

#define DO(EXPR) \
    if (!(EXPR)) return false;

//#define U(EXP) \
//    utils::To_UTF16(EXP)

typedef std::u16string UString;

typedef double JS_Number;
typedef UString JS_RegExp;

template <typename T>
using Sp = std::shared_ptr<T>;
