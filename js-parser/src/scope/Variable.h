//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <vector>
#include <memory>
#include "../macros.h"
#include "Utils.h"
#include "../parser/NodeTypes.h"

namespace jetpack {

    class Scope;

    enum class VarKind {
        Invalid = 0,
        Var,
        Let,
        Const,

        Init,

        /**
         * "constructor" of a class
         */
        Ctor,

        Method,
        Get,
        Set,
    };

    class Variable {
    public:
        VarKind kind = VarKind::Invalid;
//        bool is_exported = false;
//        bool is_imported = false;
//        bool is_mutated = false;
        Scope* scope = nullptr;

        UString name;

        /**
         * for imported and exported variable
         */
        UString external_name;

        std::vector<std::shared_ptr<Identifier>> identifiers;

    };

}
