//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include <vector>
#include <set>
#include "parser/SyntaxNodes.h"

namespace jetpack {

    class ExternalImportHandler {
    public:
        void HandleImport(const Sp<ImportDeclaration>& import);

        bool IsImportExternal(const Sp<ImportDeclaration>& import);

        std::vector<Sp<ImportDeclaration>> imports;

        /**
         * use set of pointers to mark external imports
         */
        std::set<std::uintptr_t> external_import_ptrs;

    };

}
