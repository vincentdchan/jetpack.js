//
// Created by Duzhong Chen on 2021/9/24.
//
#include <gtest/gtest.h>
#include <iostream>
#include "parser/AstContext.h"
#include "parser/SyntaxNodes.h"

using namespace jetpack;

static bool on = false;

class MyField {
public:

    ~MyField() {
        on = true;
    }

};

class MyNode : public SyntaxNode {
public:

    MyField field;

};

TEST(Memroy, Delete) {
    EXPECT_EQ(on, false);
    {
        AstContext ctx;

        auto node = ctx.Alloc<MyNode>();
        std::cout << "alloc: " << reinterpret_cast<uintptr_t>(node) << std::endl;
    }

    EXPECT_EQ(on, true);
}
