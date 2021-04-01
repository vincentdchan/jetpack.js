//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModulesTable.h"

namespace jetpack {

    Sp<ModuleFile> ModulesTable::createNewIfNotExists(const std::string &path, bool& isNew) {
        std::lock_guard guard(m);

        auto iter = pathToModule.find(path);
        if (iter != pathToModule.end()) {
            isNew = false;
            return iter->second;  // exists;
        }

        int32_t newId = pathToModule.size();
        auto newMod = std::make_shared<ModuleFile>(path, newId);

        insertWithoutLock(newMod);
        isNew = true;
        return newMod;
    }

    Sp<ModuleFile> ModulesTable::findModuleById(int32_t id) {
        std::lock_guard guard(m);
        auto iter = idToModule.find(id);
        if (iter == idToModule.end()) {
            return nullptr;
        }
        return iter->second;
    }

    int32_t ModulesTable::ModCount() const {
        return pathToModule.size();
    }

    void ModulesTable::insertWithoutLock(const Sp<ModuleFile> &mf) {
        pathToModule[mf->Path()] = mf;
        idToModule[mf->id()] = mf;
    }

}
