//
// Created by Duzhong Chen on 2020/3/21.
//

#include "./Variable.h"
#include "../parser/SyntaxNodes.h"

namespace rocket_bundle {

    const UString& Variable::Name() const {
        auto id = std::dynamic_pointer_cast<Identifier>(node.lock());
        return id->name;
    }

}
