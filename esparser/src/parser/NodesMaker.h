//
// Created by Duzhong Chen on 2021/5/24.
//

#ifndef ROCKET_BUNDLE_NODESMAKER_H
#define ROCKET_BUNDLE_NODESMAKER_H

#include "string/UString.h"
#include "SyntaxNodes.h"

namespace jetpack {

    Sp<Identifier> MakeId(const UString& content);

    Sp<Identifier> MakeId(const std::string& content);

    Sp<Literal> MakeStringLiteral(const UString& str);

    void WrapModuleWithCommonJsTemplate(const Sp<Module>& module, const UString& var_name, const UString& cjs_call);

}

#endif //ROCKET_BUNDLE_NODESMAKER_H
