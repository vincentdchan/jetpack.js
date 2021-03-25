//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <optional>
#include "ModuleFile.h"
#include "WorkerError.h"

namespace jetpack {

    // better to be stateless
    // should be implemented thread-safe
    class ModuleProvider {
    public:
        virtual std::optional<std::string> match(const ModuleFile &mf, const std::string& path) = 0;

        virtual UString resolve(const ModuleFile &mf, const std::string& resolvedPath) = 0;

        ~ModuleProvider() noexcept = default;

        std::optional<WorkerError> error;

    };

    class FileModuleProvider : public ModuleProvider {
    public:
        explicit FileModuleProvider(const std::string& base_path): base_path_(base_path) {}

        std::optional<std::string> match(const ModuleFile &mf, const std::string &path) override;

        UString resolve(const ModuleFile &mf, const std::string& resolvedPath) override;

    private:
        std::string base_path_;

    };

    class MemoryModuleProvider : public ModuleProvider {
    public:
        explicit inline MemoryModuleProvider(const std::string& token, const UString& content):
        token_(token), content_(content) {}

        std::optional<std::string> match(const ModuleFile &mf, const std::string &path) override;

        UString resolve(const ModuleFile &mf, const std::string &path) override;

    private:
        std::string token_;
        UString     content_;

    };

}
