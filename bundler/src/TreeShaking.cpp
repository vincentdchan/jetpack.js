//
// Created by Duzhong Chen on 2020/4/8.
//

#include "TreeShaking.h"
#include "ModuleResolver.h"

namespace disk_fs {

    TreeShaking::TreeShaking(ModuleFile* parent_)
    : parent(parent_) {}

    void TreeShaking::MarkUsedExport(const UString &name) {

    }

}
