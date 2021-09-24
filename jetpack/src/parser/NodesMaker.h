//
// Created by Duzhong Chen on 2021/5/24.
//

#pragma once

#include "SyntaxNodes.h"
#include "parser/AstContext.h"

namespace jetpack {

    Literal* MakeNull(AstContext& ctx);

    Identifier* MakeId(AstContext& ctx, const std::string& content);

    Identifier* MakeId(AstContext& ctx, const SourceLocation& loc, const std::string& content);

    Literal* MakeStringLiteral(AstContext& ctx, const std::string& str);

    void WrapModuleWithCommonJsTemplate(
            AstContext& ctx,
            Module& module, const std::string& var_name, const std::string& cjs_call);

}
