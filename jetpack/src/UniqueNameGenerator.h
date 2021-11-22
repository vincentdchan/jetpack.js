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

        virtual std::optional<std::string> Next(const std::string& original_name) = 0;

        virtual bool IsNameUsed(const std::string& name) { return false; };

        virtual ~UniqueNameGenerator() = default;

    };

    class UniqueNameGeneratorWithUsedName : public UniqueNameGenerator {
    protected:
        UniqueNameGeneratorWithUsedName();

        HashSet<std::string> used_name;
        static std::mutex used_name_mutex_;

        bool IsJsKeyword(const std::string& name);

    private:
        static std::once_flag init_once_;
        static HashSet<std::string> long_keywords_set;

    };

    class ReadableNameGenerator : public UniqueNameGeneratorWithUsedName {
    public:
        static std::shared_ptr<ReadableNameGenerator> Make();

        std::optional<std::string> Next(const std::string& original_name) override;

        bool IsNameUsed(const std::string& name) override;

    private:
        ReadableNameGenerator() = default;

        int32_t counter = 0;

        Sp<ReadableNameGenerator> prev;

        std::weak_ptr<ReadableNameGenerator> weak_self;

    };

    /**
     * As base prevous generator of MinifyNameGenerator;
     */
    class UnresolvedNameCollector : public UniqueNameGenerator {
    public:
        UnresolvedNameCollector() = default;

        bool IsNameUsed(const std::string& name) override;

        std::optional<std::string> Next(const std::string& original_name) override {
            return std::nullopt;
        }

        HashSet<std::string> used_name;

        std::mutex logger_mutex;

        void InsertByList(std::vector<Identifier*> list);

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

        std::optional<std::string> Next(const std::string& original_name) override;

        bool IsNameUsed(const std::string& name) override;

        std::string GenAName();

    private:
        MinifyNameGenerator() = default;

        int32_t counter = 0;

        Sp<UniqueNameGenerator> prev;

        std::weak_ptr<MinifyNameGenerator> weak_self;

    };

}
