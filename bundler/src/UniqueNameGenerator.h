//
// Created by Duzhong Chen on 2020/3/24.
//

#pragma once

#include <string>
#include <unordered_set>
#include <optional>

namespace rocket_bundle {

    class UniqueNameGenerator {
    public:
        UniqueNameGenerator() = default;

        virtual std::optional<std::u16string> Next(const std::u16string& original_name) = 0;

        std::unordered_set<std::u16string> used_name;

        virtual ~UniqueNameGenerator() = default;

    };

    class ReadableNameGenerator : public UniqueNameGenerator {
    public:
        ReadableNameGenerator() = default;

        std::optional<std::u16string> Next(const std::u16string& original_name) override;

    private:
        std::int32_t counter = 0;

    };

    class MinifyNameGenerator : public UniqueNameGenerator {
    public:
        static constexpr std::size_t BUFFER_SIZE = 32;

        MinifyNameGenerator() = default;

        std::optional<std::u16string> Next(const std::u16string& original_name) override;

    private:
        std::int32_t buffer[BUFFER_SIZE];
        std::int32_t counter = 0;

    };

}
