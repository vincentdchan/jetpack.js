//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <cinttypes>
#include <string>
#include <parser/Parser.hpp>
#include "string/UString.h"
#include "codegen/CodeGen.h"
#include "UniqueNameGenerator.h"

namespace jetpack {
    class ModuleResolver;
    class ModuleProvider;

    struct RenamerCollection {
    public:
        std::vector<Sp<MinifyNameGenerator>> content;
        std::shared_ptr<UnresolvedNameCollector> idLogger;

        std::mutex mutex_;

    };

    class ModuleFile {
    public:
        /**
         * Unique id in module resolver
         */
        int32_t id = 0;

        /**
         * Abosolute path
         */
        std::string path;

        UString default_export_name;

        // interface to provide content by contents;
        Sp<ModuleProvider> provider;

        Weak<ModuleResolver> module_resolver;

        Sp<Module> ast;

        /**
         * relative path -> absolute path
         */
        HashMap<std::string, std::string> resolved_map;

        /**
         * Temp for parallel codegen
         */
        UString codegen_result;

        /**
         * For Postorder traversal
         */
        bool visited_mark = false;

        /**
         * For Postorder traversal
         */
        std::vector<std::weak_ptr<ModuleFile>> ref_mods;

        void RenameInnerScopes(RenamerCollection& col);
        Sp<MinifyNameGenerator> RenameInnerScopes(Scope& scope, UnresolvedNameCollector* idLogger);

        void CodeGenFromAst(const CodeGen::Config &config);

        UString GetModuleVarName() const;

        UString GetSource() const;

        inline ExportManager& GetExportManager() {
            return ast->scope->export_manager;
        }

    };


}
