//
// Created by Duzhong Chen on 2020/4/8.
//

#pragma once

#include "Utils.h"

namespace disk_fs {

    class ModuleFile;

    class TreeShaking {
    public:
        TreeShaking(ModuleFile* parent_);

        void MarkUsedExport(const UString& name);

    private:
        ModuleFile* parent;

    };

}
