//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <vector>
#include <memory>
#include <robin_hood.h>
#include <unordered_set>

#include "Variable.h"
#include "ImportManager.h"
#include "ExportManager.h"

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

        /**
         * Actually, variable declarator doesn't
         * create new scope.
         * It's regarded as a scope for graph computation.
         */
        VariableDeclarator,

    };

    class ModuleScope;

    class Scope {
    public:

        Scope() = default;
        Scope(const Scope&) = delete;
        Scope(Scope&&) = delete;

        Scope& operator=(const Scope&) = delete;
        Scope& operator=(Scope&&) = delete;

        explicit Scope(ScopeType t) : type(t) {
        }

        ScopeType type = ScopeType::Invalid;

        ModuleScope* CastToMoudle();

        inline bool IsMoudle() const {
            return type == ScopeType::Module;
        }

        virtual Variable* RecursivelyFindVariable(const UString& var_name);

        virtual Variable* CreateVariable(const UString& var_name, VarKind kind);

        virtual void FindOrAddFreeVarInCurrentScope(const UString& name);

        void AddFreeVariableName(const UString& free_var_name);

        void SetParent(Scope* parent_);

        inline Scope* GetParent() {
            return parent;
        }

        virtual ~Scope() = default;

    protected:
        Scope* parent = nullptr;

        robin_hood::unordered_map<UString, Variable> own_variables;
        std::unordered_set<UString> free_var_names;

    };

    /**
     * Moudle scope is special
     */
    class ModuleScope : public Scope {
    public:

        ModuleScope();

        ImportManager import_manager;
        ExportManager export_manager;

    };

}
