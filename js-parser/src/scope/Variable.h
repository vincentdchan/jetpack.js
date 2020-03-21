//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <memory>
#include "../macros.h"
#include "../utils.h"

namespace rocket_bundle {

    class Scope;
    class Identifier;
    class SyntaxNode;

    class Variable {
    public:
        bool is_mutated;
        std::weak_ptr<Scope> scope;
        std::weak_ptr<SyntaxNode> node;

        [[nodiscard]] const UString& Name() const;

    };

}
