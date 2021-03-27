//
// Created by Duzhong Chen on 2021/3/25.
//

#include <fstream>
#include "ModuleProvider.h"
#include "Utils.h"
#include "Path.h"

namespace jetpack {

    std::optional<std::string> FileModuleProvider::match(const ModuleFile &mf, const std::string &path) {
        return pMatch(mf, path);
    }

    std::optional<std::string> FileModuleProvider::pMatch(const ModuleFile &mf, const std::string &path) const {
        Path module_path(mf.path);
        module_path.Pop();
        module_path.Join(path);

        std::string source_path = module_path.ToString();

        if (!utils::IsFileExist(source_path)) {
            if (!module_path.EndsWith(".js")) {
                module_path.slices[module_path.slices.size() - 1] += ".js";
                source_path = module_path.ToString();
            }

            if (!utils::IsFileExist(source_path)) {
//                ResolveResult<std::string> result;
//                result.error = { { mf.path, std::string("file doesn't exist: ") + source_path } };
                return std::nullopt;
            }
        }
        return { source_path };
    }

    UString readFileStream(const std::string& filename) {
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        return UString::fromStdString(str);
    }

    ResolveResult<UString> FileModuleProvider::resolve(const jetpack::ModuleFile &mf, const std::string &resolvedPath) {
        return ResolveResult(readFileStream(resolvedPath));
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
