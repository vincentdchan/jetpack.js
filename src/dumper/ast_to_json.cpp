//
// Created by Duzhong Chen on 2019/9/11.
//
#include "ast_to_json.h"
#include "../parser/node_traverser.h"

namespace dumper {

    void AstToJson::TraverseAfter(const Sp<ArrayExpression>& node) {
        result = json::object();
        result["type"] = "ArrayExpression";

        auto children = json::array();
        auto children_traverser = std::make_shared<AstToJson>();
        for (auto& elm : node->elements) {
            NodeTraverser::Traverse(elm, children_traverser);
            children.push_back(std::move(children_traverser->Result()));
        }
        result["children"] = children;
    }

    void AstToJson::TraverseAfter(const Sp<ArrayPattern> &node) {

    }

}
