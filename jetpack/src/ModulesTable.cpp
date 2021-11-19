//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModulesTable.h"

namespace jetpack {

    Sp<ModuleFile> ModulesTable::CreateNewIfNotExists(const std::string &path, bool& is_new) {
        std::lock_guard guard(m);

        auto iter = path_to_module.find(path);
        if (iter != path_to_module.end()) {
            is_new = false;
            return iter->second;  // exists;
        }

        int32_t new_id = id_to_module.size();
        auto new_mod = std::make_shared<ModuleFile>(path, new_id);

        InsertWithoutLock(new_mod);
        is_new = true;
        return new_mod;
    }

    Sp<ModuleFile> ModulesTable::FindModuleById(int32_t id) {
        std::lock_guard guard(m);
        if (id < 0 || id >= id_to_module.size()) {
            return nullptr;
        }
        return id_to_module[id];
    }

    int32_t ModulesTable::ModCount() const {
        return path_to_module.size();
    }

    void ModulesTable::InsertWithoutLock(const Sp<ModuleFile> &mf) {
        path_to_module[mf->Path()] = mf;
        id_to_module.push_back(mf);
    }

}
