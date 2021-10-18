//
// Created by Duzhong Chen on 2021/3/31.
//

#include <gtest/gtest.h>
#include "SimpleAPI.h"

TEST(SimpleAPI, FileNotExist) {
    JetpackFlags flags;
    EXPECT_NE(jetpack_bundle_module("wonderful", "ok", static_cast<int>(flags), nullptr), 0);
}
