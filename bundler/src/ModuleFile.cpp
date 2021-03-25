//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModuleFile.h"
#include "ModuleResolver.h"

namespace jetpack {

    void ModuleFile::RenameInnerScopes(RenamerCollection& renamer) {
        std::vector<Sp<MinifyNameGenerator>> result;
        result.reserve(ast->scope->children.size());
        for (auto child : ast->scope->children) {
            result.push_back(RenameInnerScopes(*child, renamer.idLogger.get()));
        }

        auto final = MinifyNameGenerator::Merge(result);
        {
            std::lock_guard<std::mutex> lk(renamer.mutex_);
            renamer.content.push_back(std::move(final));
        }
    }

    Sp<MinifyNameGenerator> ModuleFile::RenameInnerScopes(Scope &scope, UnresolvedNameCollector* idLogger) {
        std::vector<Sp<MinifyNameGenerator>> temp;
        temp.reserve(scope.children.size());

        for (auto child : scope.children) {
            temp.push_back(RenameInnerScopes(*child, idLogger));
        }

        std::vector<std::tuple<UString, UString>> renames;
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

    void ModuleFile::CodeGenFromAst(const CodeGen::Config &config) {
        MemoryOutputStream memoryOutputStream;

        if (config.comments) {
            memoryOutputStream << u"// " << UString::fromStdString(this->path) << u"\n";
        }

        CodeGen codegen(config, memoryOutputStream);
        codegen.Traverse(ast);
        codegen_result = memoryOutputStream.ToString();
    }

    UString ModuleFile::GetModuleVarName() {
        std::string tmp = "mod_" + std::to_string(id);
        return UString::fromStdString(tmp);
    }


}
