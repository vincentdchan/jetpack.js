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
        HashMap<std::string, Sp<ModuleFile>> path_to_module;

        HashMap<int32_t, Sp<ModuleFile>>     id_to_module;

        inline void Insert(const Sp<ModuleFile>& mf) {
            std::lock_guard guard(m);
            InsertWithoutLock(mf);
        }

        Sp<ModuleFile> CreateNewIfNotExists(const std::string& path, bool& isNew);

        // nullable!
        Sp<ModuleFile> FindModuleById(int32_t id);

        int32_t ModCount() const;

    private:
        std::mutex m;

        void InsertWithoutLock(const Sp<ModuleFile>& mf);

    };

}
