//
// Created by Duzhong Chen on 2021/3/25.
//

#include "utils/io/FileIO.h"
#include "ModuleProvider.h"
#include "utils/Path.h"

namespace jetpack {

    std::optional<std::string> FileModuleProvider::match(const ModuleFile &mf, const std::string &path) {
        return pMatch(mf, path);
    }

    std::optional<std::string> FileModuleProvider::pMatch(const ModuleFile &mf, const std::string &path) const {
        Path module_path(base_path_);
        module_path.Join(mf.Path());
        module_path.Pop();
        module_path.Join(path);

        std::string sourcePath = module_path.ToString();

        if (unlikely(sourcePath.rfind(base_path_, 0) != 0)) {  // is not under working dir
            std::cerr << "path: " << sourcePath << " is not under working dir: " << base_path_ << std::endl;
            return std::nullopt;
        }

        if (!io::IsFileExist(sourcePath)) {
            if (!module_path.EndsWith(".js")) {
                auto tryResult = TryWithSuffix(mf, sourcePath, ".js");
                if (tryResult.has_value()) {
                    return tryResult;
                }
            }

            if (!module_path.EndsWith(".jsx")) {
                auto tryResult = TryWithSuffix(mf, sourcePath, ".jsx");
                if (tryResult.has_value()) {
                    return tryResult;
                }
            }

            return std::nullopt;
        }
        return { sourcePath.substr(base_path_.size() + 1) };
    }

    std::optional<std::string> FileModuleProvider::TryWithSuffix(const ModuleFile &mf, const std::string &path, const std::string& suffix) const {
        std::string jsPath = path + suffix;
        if (!io::IsFileExist(jsPath)) {
            return std::nullopt;
        }
        return { jsPath.substr(base_path_.size() + 1) };
    }

    ResolveResult<UString> FileModuleProvider::resolve(const jetpack::ModuleFile &mf, const std::string &resolvedPath) {
        UString result;

        // resolvedPath should not be a absolute path
        J_ASSERT(resolvedPath.at(0) != Path::PATH_DIV);

        Path absolutePath(base_path_);
        absolutePath.Join(resolvedPath);
        auto absPathStr = absolutePath.ToString();

        io::IOError err = io::ReadFileToUString(absPathStr, result);
        if (err != io::IOError::Ok) {
            ResolveResult<UString> errResult;
            errResult.error = { { absPathStr, std::string(io::IOErrorToString(err)) } };
            return errResult;
        }
        return ResolveResult(result);
    }

    std::optional<std::string> MemoryModuleProvider::match(const ModuleFile &mf, const std::string &path) {
        if (path == token_) {
            return { path };
        }
        return std::nullopt;
    }

    ResolveResult<UString> MemoryModuleProvider::resolve(const ModuleFile &mf, const std::string &path) {
        return ResolveResult(content_);
    }

}
