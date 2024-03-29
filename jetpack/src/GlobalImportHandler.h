//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include <vector>
#include <set>
#include <mutex>
#include "utils/Common.h"
#include "parser/SyntaxNodes.h"
#include "parser/AstContext.h"
#include "UniqueNameGenerator.h"
#include "codegen/CodeGen.h"

namespace jetpack {

    struct GlobalImportInfo {
    public:
        std::int32_t id = -1;

        bool has_namespace = false;
        std::string ns_import_name;

        bool has_default = false;
        std::string default_local_name;

        std::string path;

        std::vector<std::string> names;

        HashMap<std::string, std::string> alias_map;

    };

    /**
     * Manage global import declarations
     */
    class GlobalImportHandler {
    public:
        void HandleImport(ImportDeclaration* import);

        bool IsImportExternal(ImportDeclaration* import);

        void DistributeNameToImportVars(const std::shared_ptr<UniqueNameGenerator>&,
                                        const std::vector<GlobalImportInfo*>& infos);

        void GenAst(const std::shared_ptr<UniqueNameGenerator>&);

        void GenCode(CodeGen& codegen);

        std::vector<ImportDeclaration*> imports;

        /**
         * use set of pointers to mark external imports
         */
        std::set<uintptr_t> external_import_ptrs;

        HashMap<std::string, GlobalImportInfo> import_infos;

        std::vector<ImportDeclaration*> gen_import_decls;

        int32_t import_counter = 0;

    private:
        AstContext ctx_;
        std::mutex m;

    };

}
