//
// Created by Duzhong Chen on 2020/4/3.
//

#include "ExternalImportHandler.h"

namespace jetpack {

    void ExternalImportHandler::HandleImport(const Sp<ImportDeclaration> &import) {
        imports.push_back(import);
        external_import_ptrs.insert(reinterpret_cast<std::intptr_t>(import.get()));
    }

    bool ExternalImportHandler::IsImportExternal(const Sp<ImportDeclaration> &import) {
        return external_import_ptrs.find(reinterpret_cast<std::intptr_t>(import.get())) != external_import_ptrs.end();
    }

}
