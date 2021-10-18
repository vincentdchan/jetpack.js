//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModulesTable.h"

namespace jetpack {

    Sp<ModuleFile> ModulesTable::CreateNewIfNotExists(const std::string &path, bool& isNew) {
        std::lock_guard guard(m);

        auto iter = path_to_module.find(path);
        if (iter != path_to_module.end()) {
            isNew = false;
            return iter->second;  // exists;
        }

        int32_t newId = path_to_module.size();
        auto newMod = std::make_shared<ModuleFile>(path, newId);

        InsertWithoutLock(newMod);
        isNew = true;
        return newMod;
    }

    Sp<ModuleFile> ModulesTable::FindModuleById(int32_t id) {
        std::lock_guard guard(m);
        auto iter = id_to_module.find(id);
        if (iter == id_to_module.end()) {
            return nullptr;
        }
        return iter->second;
    }

    int32_t ModulesTable::ModCount() const {
        return path_to_module.size();
    }

    void ModulesTable::InsertWithoutLock(const Sp<ModuleFile> &mf) {
        path_to_module[mf->Path()] = mf;
        id_to_module[mf->id()] = mf;
    }

}
