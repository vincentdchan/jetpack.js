//
// Created by Duzhong Chen on 2020/3/22.
//

#pragma once

#include <robin_hood.h>
#include "Utils.h"

namespace rocket_bundle {

    enum class ImportType {
        Invalid = 0,

        Default,
        Identifier,
        Namespace,
    };

    class ImportIdentifierInfo {
    public:
        ImportType type;
        UString local_name;
        UString source_name;
        UString module_name;

    };

    /**
     * Ref: https://github.com/vincentdchan/webpack-deep-scope-analysis-plugin/blob/master/packages/deep-scope-analyser/src/importManager.ts
     */
    class ImportManager {
    public:
        ImportManager() = default;
        ImportManager(const ImportManager&) = delete;

        ImportManager& operator=(const ImportManager&) = delete;

        robin_hood::unordered_map<UString, ImportIdentifierInfo> id_map;

    };

}
