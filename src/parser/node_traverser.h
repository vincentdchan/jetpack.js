//
// Created by Duzhong Chen on 2019/9/10.
//
#pragma once

#include <stack>
#include "node_traverser_intf.h"
#include "syntax_nodes.h"

class NodeTraverser {
private:
    std::stack<Sp<SyntaxNode>> nodes_stack_;
    Sp<INodeTraverser> traverser_;

public:

    inline void Push(const Sp<SyntaxNode>& node);
    inline void Traverse();

private:
    void TraverseNode_(const Sp<SyntaxNode>& node);

};

inline void NodeTraverser::Push(const Sp<SyntaxNode>& node) {
    nodes_stack_.push(node);
}

inline void NodeTraverser::Traverse() {
    while (!nodes_stack_.empty()) {
        auto top = nodes_stack_.top();
        nodes_stack_.pop();

        TraverseNode_(top);
    }
}
