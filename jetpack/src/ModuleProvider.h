//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <optional>
#include "ResolveResult.h"
#include "ModuleFile.h"
#include "WorkerError.h"

namespace jetpack {

    class ResolveException : public std::exception {
    public:
        explicit ResolveException(WorkerError e): error(std::move(e)) {}

        WorkerError error;

        const char *what() const noexcept override;

    };

    // better to be stateless
    // should be implemented thread-safe
    class ModuleProvider {
    public:
        virtual std::optional<std::string> Match(const ModuleFile &mf, const std::string& path) = 0;

        virtual Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string& resolved_path) = 0;

        ~ModuleProvider() noexcept = default;

    };

    class FileModuleProvider : public ModuleProvider {
    public:
        explicit FileModuleProvider(std::string base_path): base_path_(std::move(base_path)) {}

        std::optional<std::string> Match(const ModuleFile &mf, const std::string &path) override;

        Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string& resolved_path) override;

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
        explicit inline MemoryModuleProvider(std::string token, std::string content):
        token_(std::move(token)), content_(std::move(content)) {}

        std::optional<std::string> Match(const ModuleFile &mf, const std::string &path) override;

        Sp<MemoryViewOwner> ResolveWillThrow(const ModuleFile &mf, const std::string &path) override;

    private:
        std::string token_;
        std::string content_;

    };

}
