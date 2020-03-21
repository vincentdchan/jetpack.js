//
// Created by Duzhong Chen on 2020/3/22.
//

#pragma once

#include <Utils.h>
#include <robin_hood.h>

namespace rocket_bundle {

    class ExternalVariable {
    public:
        bool is_local_var;
        bool is_export_all;
        UString export_name;
        UString source_name;

    };

    /**
     * Ref: https://github.com/vincentdchan/webpack-deep-scope-analysis-plugin/blob/master/packages/deep-scope-analyser/src/exportManager.ts
     */
    class ExportManager {
    public:
        ExportManager() = default;
        ExportManager(const ExportManager&) = delete;

        ExportManager& operator=(const ExportManager) = delete;

        robin_hood::unordered_map<UString, ExternalVariable> export_map;

    };

}
