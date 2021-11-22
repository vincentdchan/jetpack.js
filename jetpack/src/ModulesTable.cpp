//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModulesTable.h"

namespace jetpack {

    void ModulesTable::Insert(const Sp<ModuleFile>& mf) {
        std::unique_lock lock(mutex_);
        InsertWithoutLock(mf);
    }

    Sp<ModuleFile> ModulesTable::CreateNewIfNotExists(const std::string &path, bool& is_new) {
        std::unique_lock lock(mutex_);

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

    Sp<ModuleFile> ModulesTable::FindModuleById(int32_t id) const {
        std::shared_lock lock(mutex_);
        if (id < 0 || id >= id_to_module.size()) {
            return nullptr;
        }
        return id_to_module[id];
    }

    Sp<ModuleFile> ModulesTable::FindModuleByPath(const std::string& path) const {
        std::shared_lock lock(mutex_);
        auto iter = path_to_module.find(path);
        if (iter == path_to_module.end()) {
            return nullptr;
        }
        return iter->second;
    }

    size_t ModulesTable::ModCount() const {
        std::shared_lock lock(mutex_);
        return path_to_module.size();
    }

    bool ModulesTable::Empty() const {
        std::shared_lock lock(mutex_);
        return id_to_module.empty();
    }

    Slice<const Sp<ModuleFile>> ModulesTable::Modules() const {
        std::shared_lock lock(mutex_);
        return make_slice(id_to_module);
    }

    void ModulesTable::InsertWithoutLock(const Sp<ModuleFile> &mf) {
        path_to_module[mf->Path()] = mf;
        id_to_module.push_back(mf);
    }

}
