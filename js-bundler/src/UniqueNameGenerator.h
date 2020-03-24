//
// Created by Duzhong Chen on 2020/3/24.
//

#pragma once

#include <string>

namespace rocket_bundle {

    class UniqueNameGenerator {
    public:
        static constexpr std::size_t BUFFER_SIZE = 32;

        UniqueNameGenerator();

        std::string Next();

    private:
        std::int32_t buffer[BUFFER_SIZE];
        std::int32_t counter = 0;

    };

}
