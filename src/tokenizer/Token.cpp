//
// Created by Duzhong Chen on 2019/9/3.
//

#include "Token.h"


#define DD(NAME) \
    case JsTokenType::NAME: \
        return #NAME;

static const char* TokenTypeToCString(JsTokenType tt) {
    switch (tt) {

        DEF_TOKEN(DD)

        default:
            return "<invalid>";
    }
}

#undef DD
