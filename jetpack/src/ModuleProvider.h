//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <optional>
#include "ResolveResult.h"
#include "ModuleFile.h"
#include "WorkerError.h"

namespace jetpack {

    // better to be stateless
    // should be implemented thread-safe
    class ModuleProvider {
    public:
        virtual std::optional<std::string> match(const ModuleFile &mf, const std::string& path) = 0;

        virtual ResolveResult<UString> resolve(const ModuleFile &mf, const std::string& resolvedPath) = 0;

        ~ModuleProvider() noexcept = default;

    };

    class FileModuleProvider : public ModuleProvider {
    public:
        explicit FileModuleProvider(const std::string& base_path): base_path_(base_path) {}

        std::optional<std::string> match(const ModuleFile &mf, const std::string &path) override;

        ResolveResult<UString> resolve(const ModuleFile &mf, const std::string& resolvedPath) override;

    private:
        std::string base_path_;

        // const version, not allowed to modify members
        // because this will run in parallel
        [[nodiscard]]
        std::optional<std::string> pMatch(const ModuleFile &mf, const std::string &path) const;

        [[nodiscard]]
        std::optional<std::string> TryWithSuffix(const ModuleFile &mf, const std::string &path, const std::string& suffix) const;

    };

    class MemoryModuleProvider : public ModuleProvider {
    public:
        explicit inline MemoryModuleProvider(const std::string& token, const UString& content):
        token_(token), content_(content) {}

        std::optional<std::string> match(const ModuleFile &mf, const std::string &path) override;

        ResolveResult<UString> resolve(const ModuleFile &mf, const std::string &path) override;

    private:
        std::string token_;
        UString     content_;

    };

}
