//
// Created by Duzhong Chen on 2019/9/11.
//
#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include "../parser/node_traverser_intf.h"

namespace dumper {

    using json = nlohmann::json;

    template <typename T>
    using Sp = std::shared_ptr<T>;

    class AstToJson: public INodeTraverser {
    private:
        json result;

    public:

        inline json& Result() {
            return result;
        }

        void TraverseAfter(const Sp<ArrayExpression>& node) override;

        void TraverseAfter(const Sp<ArrayPattern>& node) override;

    };

}

