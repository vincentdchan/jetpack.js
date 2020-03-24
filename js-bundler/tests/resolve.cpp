//
// Created by Duzhong Chen on 2020/3/24.
//

#include <gtest/gtest.h>
#include <unordered_set>
#include "../src/UniqueNameGenerator.h"

using namespace rocket_bundle;

/**
 * do not generate duplicate var name
 */
TEST(UniqueNameGenerator, Next) {
    UniqueNameGenerator gen;
    std::unordered_set<std::string> gen_set;

    for (int i = 0; i < 10000; i++) {
        auto next_str = gen.Next();
        EXPECT_TRUE(gen_set.find(next_str) == gen_set.end());
        gen_set.insert(next_str);
    }
}
