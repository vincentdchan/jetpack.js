//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include "SyntaxNodes.h"

namespace jetpack {

    class ContantFolding {
    public:

        static Sp<Expression> TryBinaryExpression(const Sp<BinaryExpression>& binary);

    };

}
