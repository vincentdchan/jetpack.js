//
// Created by Duzhong Chen on 2021/3/31.
//

#include <gtest/gtest.h>
#include "SimpleAPI.h"

TEST(SimpleAPI, FileNotExist) {
    EXPECT_NE(jetpack::simple_api::BundleModule(true, true, true, true, "wonderful", "ok"), 0);
}
