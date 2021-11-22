//
// Created by Duzhong Chen on 2021/3/25.
//

#include <filesystem.hpp>
#include <fmt/format.h>
#include "utils/io/FileIO.h"
#include "ModuleProvider.h"

namespace jetpack {

    const char *ResolveException::what() const noexcept {
        return error.error_content.c_str();
    }

    std::optional<ghc::filesystem::path> FileModuleProvider::Match(const ModuleFile &mf, const std::string &path) {
        return pMatch(mf, path);
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    std::optional<ghc::filesystem::path> FileModuleProvider::pMatch(const ModuleFile &mf, const std::string &path) const {
        ghc::filesystem::path module_path(base_path_);
        module_path.append(mf.Path());
        module_path.append("..");
        module_path.append(path);
        module_path = module_path.lexically_normal();

        std::string source_path = module_path.string();

        if (unlikely(source_path.rfind(base_path_, 0) != 0)) {  // is not under working dir
            std::cerr << fmt::format("path {} is not under working dir: {}", source_path, base_path_.string()) << std::endl;
            return std::nullopt;
        }

        if (!exists(module_path)) {
            auto ext = module_path.extension().string();
            if (!ends_with(source_path, ".js")) {
                auto try_result = TryWithSuffix(mf, source_path, ".js");
                if (try_result.has_value()) {
                    return try_result;
                }
            }

            if (!ends_with(source_path, ".jsx")) {
                auto try_result = TryWithSuffix(mf, source_path, ".jsx");
                if (try_result.has_value()) {
                    return try_result;
                }
            }

            return std::nullopt;
        }
        return { module_path.lexically_relative(base_path_) };
    }

    std::optional<ghc::filesystem::path> FileModuleProvider::TryWithSuffix(const ModuleFile &mf, const std::string &path, const std::string& suffix) const {
        std::string js_path = path + suffix;
        if (!ghc::filesystem::exists(js_path)) {
            return std::nullopt;
        }
        return { ghc::filesystem::relative(js_path, base_path_) };
    }

    Sp<MemoryViewOwner> FileModuleProvider::ResolveWillThrow(const jetpack::ModuleFile &mf, const std::string &resolved_path) {
        // resolvedPath should not be a absolute path
        ghc::filesystem::path abs_path(base_path_);
        abs_path.append(resolved_path);
        auto abs_path_str = abs_path.string();

        std::string content;
        io::IOError err = io::ReadFileToStdString(abs_path_str, content);
        if (err != io::IOError::Ok) {
            WorkerError error = { abs_path_str, std::string(io::IOErrorToString(err)) };
            throw ResolveException(error);
        }
        auto result = std::make_shared<StringMemoryOwner>(std::move(content));
        return result;
    }

    std::optional<ghc::filesystem::path> MemoryModuleProvider::Match(const ModuleFile &mf, const std::string &path) {
        if (path == token_) {
            return { path };
        }
        return std::nullopt;
    }

    Sp<MemoryViewOwner> MemoryModuleProvider::ResolveWillThrow(const ModuleFile &mf, const std::string &path) {
        return std::make_shared<StringMemoryOwner>(content_);
    }

}
