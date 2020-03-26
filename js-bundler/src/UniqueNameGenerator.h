//
// Created by Duzhong Chen on 2020/3/24.
//

#pragma once

#include <string>

namespace rocket_bundle {

    class UniqueNameGenerator {
    public:
        UniqueNameGenerator() = default;

        virtual std::u16string Next(const std::u16string& original_name) = 0;

    };

    class MinifyNameGenerator : public UniqueNameGenerator {
    public:
        static constexpr std::size_t BUFFER_SIZE = 32;

        MinifyNameGenerator();

        std::u16string Next(const std::u16string& original_name) override;

    private:
        std::int32_t buffer[BUFFER_SIZE];
        std::int32_t counter = 0;

    };

}
