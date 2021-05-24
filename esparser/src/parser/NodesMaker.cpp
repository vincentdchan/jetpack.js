//
// Created by Duzhong Chen on 2021/5/24.
//

#include "NodesMaker.h"

namespace jetpack {

    Sp<Identifier> MakeId(const UString& content) {
        auto id = std::make_shared<Identifier>();
        id->location.fileId = -2;
        id->name = content;
        return id;
    }

    Sp<Identifier> MakeId(const std::string& content) {
        return MakeId(UString::fromUtf8(content.c_str(), content.size()));
    }

    Sp<Literal> MakeStringLiteral(const UString& str) {
        auto lit = std::make_shared<Literal>();
        lit->ty = Literal::Ty::String;
        lit->str_ = str;
        lit->raw = u"\"" + str + u"\"";
        return lit;
    }

}
