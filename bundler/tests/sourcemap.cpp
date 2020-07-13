//
// Created by Duzhong Chen on 2020/7/14.
//

#include <gtest/gtest.h>
#include "../src/sourcemap/SourceMapGenerator.h"

using namespace jetpack;

TEST(SourceMap, Generator) {
    std::stringstream ss;
    EXPECT_TRUE(SourceMapGenerator::IntToVLQ(ss, 16));
    std::string str = ss.str();
    EXPECT_STREQ(str.c_str(), "gB");
}
