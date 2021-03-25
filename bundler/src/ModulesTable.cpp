//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModulesTable.h"

namespace jetpack {

    bool ModulesTable::insertIfNotExists(const Sp<ModuleFile>& mf) {
        std::lock_guard guard(m);

        if (pathToModule.find(mf->path) != pathToModule.end()) {
            return false;  // exists;
        }

        insertWithoutLock(mf);

        return true;
    }

    Sp<ModuleFile> ModulesTable::createNewIfNotExists(const std::string &path, bool& isNew) {
        std::lock_guard guard(m);

        auto iter = pathToModule.find(path);
        if (iter != pathToModule.end()) {
            isNew = false;
            return iter->second;  // exists;
        }

        auto newMod = std::make_shared<ModuleFile>();

        insertWithoutLock(newMod);
        isNew = true;
        return newMod;
    }

    void ModulesTable::insertWithoutLock(const Sp<ModuleFile> &mf) {
        mf->id = mod_counter_++;
        pathToModule[mf->path] = mf;
        idToModule[mf->id] = mf;
    }

}
