//
// Created by Duzhong Chen on 2021/5/24.
//

#pragma once

#include "SyntaxNodes.h"

namespace jetpack {

    Sp<Literal> MakeNull();

    Sp<Identifier> MakeId(const std::string& content);

    Sp<Identifier> MakeId(const SourceLocation& loc, const std::string& content);

    Sp<Literal> MakeStringLiteral(const std::string& str);

    void WrapModuleWithCommonJsTemplate(const Sp<Module>& module, const std::string& var_name, const std::string& cjs_call);

}
