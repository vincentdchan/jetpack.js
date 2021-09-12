//
// Created by Duzhong Chen on 2021/9/11.
//

#include "Config.h"

namespace jetpack::parser {

    Config Config::Default() {
        return {
                std::nullopt,
                false,
                true,
                false,
                false,
                false,
                false,
                false,
                true,
        };
    }

}
