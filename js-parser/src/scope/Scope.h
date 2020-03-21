//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <memory>
#include "./Variable.h"
#include <robin_hood.h>

namespace rocket_bundle {

    enum class ScopeType {
        Invalid = 0,

        TDZ,
        Module,
        Block,
        Switch,
        Function,
        Catch,
        With,
        Class,
        Global,
        FunctionExpressionName,
        For,
    };

    class Scope {
    public:

        Scope() = default;

        explicit Scope(ScopeType t) : type(t) {
        }

        Scope* parent = nullptr;
        ScopeType type = ScopeType::Invalid;
        robin_hood::unordered_map<UString, Variable> variables;

        virtual ~Scope() = default;

    };

}
