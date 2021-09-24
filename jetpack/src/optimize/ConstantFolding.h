//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include "parser/SyntaxNodes.h"
#include "parser/AstContext.h"

namespace jetpack {

    class ContantFolding {
    public:

        static Expression* TryBinaryExpression(AstContext& ctx, BinaryExpression* binary);

    };

}
