//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModuleFile.h"
#include "ModuleResolver.h"

namespace jetpack {

    ModuleFile::ModuleFile(const std::string& path, int32_t id): path_(path), id_(id) {
        mapping_collector_ = std::make_shared<MappingCollector>();
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
            memoryOutputStream << u"// " << UString::fromStdString(path()) << u"\n";
        }

        Sp<MappingCollector> mappingCollector;
        if (config.sourcemap) {
            // TODO: filename?
            mappingCollector = std::make_shared<MappingCollector>();
        }

        CodeGen codegen(config, mappingCollector, memoryOutputStream);
        codegen.Traverse(ast);
        codegen_result = memoryOutputStream.ToString();
    }

    UString ModuleFile::GetModuleVarName() const {
        std::string tmp = "mod_" + std::to_string(id());
        return UString::fromStdString(tmp);
    }

    ResolveResult<UString> ModuleFile::GetSource() const {
        assert(provider);
        return provider->resolve(*this, path());
    }


}
