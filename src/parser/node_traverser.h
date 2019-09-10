//
// Created by Duzhong Chen on 2019/9/10.
//
#pragma once

#include <stack>
#include "node_traverser_intf.h"
#include "syntax_nodes.h"

class NodeTraverser {
public:
    enum class visit_tag {
        unvisited = 0,
        visited,
    };

private:
    std::stack<std::pair<Sp<SyntaxNode>, visit_tag>> nodes_stack_;
    Sp<INodeTraverser> traverser_;

public:

    inline void Push(const Sp<SyntaxNode>& node);
    inline void Traverse();

private:
    void TraverseNodeBefore_(const Sp<SyntaxNode>& node);
    void TraverseNodeAfter_(const Sp<SyntaxNode>& node);

};

inline void NodeTraverser::Push(const Sp<SyntaxNode>& node) {
    nodes_stack_.push(std::make_pair(node, visit_tag::unvisited));
}

inline void NodeTraverser::Traverse() {
    while (!nodes_stack_.empty()) {
        auto& top = nodes_stack_.top();

        if (top.second == visit_tag::unvisited) {
            TraverseNodeBefore_(top.first);
            top.second = visit_tag::visited;
        } else {
            TraverseNodeAfter_(top.first);
            nodes_stack_.pop();
        }
    }
}
