//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once
#include <mutex>
#include <atomic>
#include "utils/Common.h"
#include "ModuleFile.h"

namespace jetpack {

    struct ModulesTable {
    public:
        ModulesTable() = default;

        /**
         * absolute path -> module
         */
        HashMap<std::string, Sp<ModuleFile>> pathToModule;

        HashMap<int32_t, Sp<ModuleFile>>     idToModule;

        inline void insert(const Sp<ModuleFile>& mf) {
            std::lock_guard guard(m);
            insertWithoutLock(mf);
        }

        Sp<ModuleFile> createNewIfNotExists(const std::string& path, bool& isNew);

        // nullable!
        Sp<ModuleFile> findModuleById(int32_t id);

        int32_t ModCount() const;

    private:
        std::mutex m;

        void insertWithoutLock(const Sp<ModuleFile>& mf);

    };

}
