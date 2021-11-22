//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <optional>
#include <filesystem.hpp>
#include "ResolveResult.h"
#include "ModuleFile.h"
#include "WorkerError.h"

namespace jetpack {

    class ResolveException : public std::exception {
    public:
        ResolveException(WorkerError e): error(std::move(e)) {}

        WorkerError error;

        const char *what() const noexcept override;

    };

    // better to be stateless
    // should be implemented thread-safe
    class ModuleProvider {
    public:
        virtual std::optional<ghc::filesystem::path> Match(const ModuleFile &mf, const std::string& path) = 0;

        virtual Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string& resolved_path) = 0;

        ~ModuleProvider() noexcept = default;

    };

    class FileModuleProvider : public ModuleProvider {
    public:
        explicit FileModuleProvider(ghc::filesystem::path base_path):
        base_path_(std::move(base_path)) {}

        std::optional<ghc::filesystem::path> Match(const ModuleFile &mf, const std::string &path) override;

        Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string& resolved_path) override;

    private:
        ghc::filesystem::path base_path_;

        // const version, not allowed to modify members
        // because this will run in parallel
        [[nodiscard]]
        std::optional<ghc::filesystem::path> pMatch(const ModuleFile &mf, const std::string &path) const;

        [[nodiscard]]
        std::optional<ghc::filesystem::path> TryWithSuffix(const ModuleFile &mf, const std::string &path, const std::string& suffix) const;

    };

    class MemoryModuleProvider : public ModuleProvider {
    public:
        explicit inline MemoryModuleProvider(const std::string& token, const std::string& content):
        token_(token), content_(content) {}

        std::optional<ghc::filesystem::path> Match(const ModuleFile &mf, const std::string &path) override;

        Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string &path) override;

    private:
        std::string token_;
        std::string content_;

    };

}
