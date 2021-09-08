//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include <vector>
#include <set>
#include <mutex>
#include "utils/Common.h"
#include "parser/SyntaxNodes.h"
#include "UniqueNameGenerator.h"
#include "codegen/CodeGen.h"

namespace jetpack {

    struct GlobalImportInfo {
    public:
        std::int32_t id = -1;

        bool has_namespace = false;
        UString ns_import_name;

        bool has_default = false;
        UString default_local_name;

        UString path;

        std::vector<UString> names;

        HashMap<UString, UString> alias_map;

    };

    /**
     * Manage global import declarations
     */
    class GlobalImportHandler {
    public:
        void HandleImport(const Sp<ImportDeclaration>& import);

        bool IsImportExternal(const Sp<ImportDeclaration>& import);

        void DistributeNameToImportVars(const std::shared_ptr<UniqueNameGenerator>&,
                                        const std::vector<GlobalImportInfo*>& infos);

        void GenAst(const std::shared_ptr<UniqueNameGenerator>&);

        CodeGenResult GenCode(
                const CodeGen::Config &config,
                const Sp<MappingCollector>& mc);

        std::vector<Sp<ImportDeclaration>> imports;

        /**
         * use set of pointers to mark external imports
         */
        std::set<uintptr_t> external_import_ptrs;

        HashMap<UString, GlobalImportInfo> import_infos;

        std::vector<Sp<ImportDeclaration>> gen_import_decls;

        int32_t import_counter = 0;

    private:
        std::mutex m;

    };

}
