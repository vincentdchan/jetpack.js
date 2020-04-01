//
// Created by Duzhong Chen on 2020/3/24.
//

#pragma once

#include <robin_hood.h>
#include <memory>
#include <string>
#include <optional>
#include <mutex>
#include <robin_hood.h>
#include <parser/SyntaxNodes.h>

namespace jetpack {

    class UniqueNameGenerator {
    public:

        virtual std::optional<std::u16string> Next(const std::u16string& original_name) = 0;

        virtual bool IsNameUsed(const std::u16string& name) { return false; };

        virtual ~UniqueNameGenerator() = default;

    };

    class UniqueNameGeneratorWithUsedName : public UniqueNameGenerator {
    protected:
        UniqueNameGeneratorWithUsedName();

        robin_hood::unordered_set<std::u16string> used_name;

        bool IsJsKeyword(const std::u16string& name);

    private:
        static std::once_flag init_once_;
        static robin_hood::unordered_set<std::u16string> long_keywords_set;

    };

    class ReadableNameGenerator : public UniqueNameGeneratorWithUsedName {
    public:
        static std::shared_ptr<ReadableNameGenerator> Make();

        std::optional<std::u16string> Next(const std::u16string& original_name) override;

        bool IsNameUsed(const std::u16string& name) override;

    private:
        ReadableNameGenerator() = default;

        std::int32_t counter = 0;

        std::shared_ptr<ReadableNameGenerator> prev;

        std::weak_ptr<ReadableNameGenerator> weak_self;

    };

    /**
     * As base prevous generator of MinifyNameGenerator;
     */
    class UnresolvedNameCollector : public UniqueNameGenerator {
    public:
        UnresolvedNameCollector() = default;

        bool IsNameUsed(const std::u16string& name) override;

        std::optional<std::u16string> Next(const std::u16string& original_name) override {
            return std::nullopt;
        }

        robin_hood::unordered_set<std::u16string> used_name;

        std::mutex logger_mutex;

        void InsertByList(std::vector<std::shared_ptr<Identifier>> list);

    };

    class MinifyNameGenerator : public UniqueNameGeneratorWithUsedName {
    public:
        static constexpr std::size_t BUFFER_SIZE = 32;

        static std::shared_ptr<MinifyNameGenerator> Make();

        static
        std::shared_ptr<MinifyNameGenerator>
        Merge(std::vector<std::shared_ptr<MinifyNameGenerator>>& vec);

        static
        std::shared_ptr<MinifyNameGenerator>
        Merge(std::vector<std::shared_ptr<MinifyNameGenerator>>& vec,
              const std::shared_ptr<UniqueNameGenerator>& prev);

        std::optional<std::u16string> Next(const std::u16string& original_name) override;

        bool IsNameUsed(const std::u16string& name) override;

        std::u16string GenAName();

    private:
        MinifyNameGenerator() = default;

        std::int32_t counter = 0;

        std::shared_ptr<UniqueNameGenerator> prev;

        std::weak_ptr<MinifyNameGenerator> weak_self;

    };

}
