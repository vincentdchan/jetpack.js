//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once
#include <mutex>
#include <atomic>
#include "Utils.h"
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

        inline int32_t modCount() const {
            return mod_counter_.load();
        }

    private:
        std::mutex m;
        std::atomic<int32_t> mod_counter_{};

        void insertWithoutLock(const Sp<ModuleFile>& mf);

    };

}
