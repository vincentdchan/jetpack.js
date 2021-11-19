//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once
#include <mutex>
#include <atomic>
#include <vector>
#include <shared_mutex>
#include "utils/Common.h"
#include "Slice.h"
#include "ModuleFile.h"

namespace jetpack {

    // thread safe modules index
    struct ModulesTable {
    public:
        ModulesTable() = default;

        void Insert(const Sp<ModuleFile>& mf);

        Sp<ModuleFile> CreateNewIfNotExists(const std::string& path, bool& is_new);

        // nullable!
        Sp<ModuleFile> FindModuleById(int32_t id) const;

        // nullable!
        Sp<ModuleFile> FindModuleByPath(const std::string& path) const;

        size_t ModCount() const;

        bool Empty() const;

        Slice<const Sp<ModuleFile>> Modules() const;

    private:
        mutable std::shared_mutex mutex_;

        /**
         * absolute path -> module
         */
        HashMap<std::string, Sp<ModuleFile>> path_to_module;

        std::vector<Sp<ModuleFile>> id_to_module;

        void InsertWithoutLock(const Sp<ModuleFile>& mf);

    };

}
