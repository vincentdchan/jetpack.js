//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModuleProvider.h"
#include "Utils.h"
#include "Path.h"
#include "FileIO.h"

namespace jetpack {

    std::optional<std::string> FileModuleProvider::match(const ModuleFile &mf, const std::string &path) {
        return pMatch(mf, path);
    }

    std::optional<std::string> FileModuleProvider::pMatch(const ModuleFile &mf, const std::string &path) const {
        Path module_path(mf.path);
        module_path.Pop();
        module_path.Join(path);

        std::string source_path = module_path.ToString();

        if (!io::IsFileExist(source_path)) {
            if (!module_path.EndsWith(".js")) {
                module_path.slices[module_path.slices.size() - 1] += ".js";
                source_path = module_path.ToString();
            }

            if (!io::IsFileExist(source_path)) {
//                ResolveResult<std::string> result;
//                result.error = { { mf.path, std::string("file doesn't exist: ") + source_path } };
                return std::nullopt;
            }
        }
        return { source_path };
    }

    ResolveResult<UString> FileModuleProvider::resolve(const jetpack::ModuleFile &mf, const std::string &resolvedPath) {
        UString result;
        io::IOError err = io::ReadFileToUString(resolvedPath, result);
        if (err != io::IOError::Ok) {
            ResolveResult<UString> errResult;
            errResult.error = { { resolvedPath, std::string(io::IOErrorToString(err)) } };
            return errResult;
        }
        // TODO: give error?
        assert(err == io::IOError::Ok);
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
