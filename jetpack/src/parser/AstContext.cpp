//
// Created by Duzhong Chen on 2021/9/24.
//

#include "AstContext.h"
#include "parser/SyntaxNodes.h"

namespace jetpack {

    AstContext::~AstContext() noexcept {
        for (auto ptr : nodes_) {
            ptr->~SyntaxNode();
        }
    }

}
