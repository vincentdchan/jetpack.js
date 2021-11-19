//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once
#include <mutex>
#include <atomic>
#include <vector>
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

        std::vector<Sp<ModuleFile>> id_to_module;

        inline void Insert(const Sp<ModuleFile>& mf) {
            std::lock_guard guard(m);
            InsertWithoutLock(mf);
        }

        Sp<ModuleFile> CreateNewIfNotExists(const std::string& path, bool& is_new);

        // nullable!
        Sp<ModuleFile> FindModuleById(int32_t id);

        int32_t ModCount() const;

    private:
        std::mutex m;

        void InsertWithoutLock(const Sp<ModuleFile>& mf);

    };

}
