//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModuleFile.h"
#include "ModuleResolver.h"
#include "Benchmark.h"
#include "parser/NodesMaker.h"

namespace jetpack {

    ModuleFile::ModuleFile(const std::string& path, int32_t id): path_(path), id_(id) {
    }

    void ModuleFile::RenameInnerScopes(RenamerCollection& renamer) {
        std::vector<Sp<MinifyNameGenerator>> result;
        result.reserve(ast->scope->children.size());
        for (auto child : ast->scope->children) {
            result.push_back(RenameInnerScopes(*child, renamer.idLogger.get()));
        }

        auto final = MinifyNameGenerator::Merge(result);
        renamer.PushGenerator(final);
    }

    Sp<MinifyNameGenerator> ModuleFile::RenameInnerScopes(Scope &scope, UnresolvedNameCollector* idLogger) {
        std::vector<Sp<MinifyNameGenerator>> temp;
        temp.reserve(scope.children.size());

        for (auto child : scope.children) {
            temp.push_back(RenameInnerScopes(*child, idLogger));
        }

        std::vector<std::tuple<std::string, std::string>> renames;
        auto renamer = MinifyNameGenerator::Merge(temp);

        for (auto& variable : scope.own_variables) {
            auto new_opt = renamer->Next(variable.first);
            if (new_opt.has_value()) {
                renames.emplace_back(variable.first, *new_opt);
            }
        }

        scope.BatchRenameSymbols(renames);

        return renamer;
    }

    bool ModuleFile::GetSource(WorkerError& error) {
        J_ASSERT(provider);
        try {
            src_content = provider->ResolveWillThrow(*this, Path());
        } catch (ResolveException& exc) {
            error = exc.error;
            return false;
        }
        return true;
    }


}
