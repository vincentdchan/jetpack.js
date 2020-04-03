//
// Created by Duzhong Chen on 2020/4/3.
//

#pragma once

#include <vector>
#include "parser/SyntaxNodes.h"

namespace jetpack {

    class ExternalImportHandler {
    public:

        std::vector<Sp<ImportDeclaration>> imports;

    };

}
