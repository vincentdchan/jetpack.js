//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <vector>
#include <list>
#include <memory>

#include "Variable.h"
#include "ImportManager.h"
#include "ExportManager.h"

namespace jetpack {

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

    class VariableExistsError;

    class Scope {
    public:
        using PVar = std::shared_ptr<Variable>;

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

        virtual PVar RecursivelyFindVariable(const UString& var_name);

        virtual PVar CreateVariable(const std::shared_ptr<Identifier>& var_id, VarKind kind);

        virtual void AddUnresolvedId(const std::shared_ptr<Identifier>& id) {
            unresolved_id.push_back(id);
        }

        void SetParent(Scope* parent_);

        inline bool RemoveVariable(const UString& name) {
            auto iter = own_variables.find(name);
            if (iter == own_variables.end()) {
                return false;
            }
            own_variables.erase(iter);
            return true;
        }

        inline Scope* GetParent() {
            return parent;
        }

        inline Scope* GetRoot() {
            Scope* result = this;
            while (result->GetParent() != nullptr) {
                result = result->GetParent();
            }
            return result;
        }

        /**
         * @param unresolve_collector collect unresolved ids
         */
        void ResolveAllSymbols(std::vector<std::shared_ptr<Identifier>>* unresolve_collector);

        virtual bool BatchRenameSymbols(const std::vector<std::tuple<UString, UString>>& changeset);

        virtual ~Scope() = default;

        HashMap<UString, PVar> own_variables;

        std::vector<Scope*> children;

    protected:
        Scope* parent = nullptr;

        /**
         * log identifier when parsing
         */
        std::list<std::shared_ptr<Identifier>> unresolved_id;

    };

    /**
     * Used in left value, do not record id
     */
    class LeftValueScope : public Scope {
    public:
        static LeftValueScope default_;

        PVar CreateVariable(const std::shared_ptr<Identifier>& var_id, VarKind kind) override {
            return nullptr;
        }

        void AddUnresolvedId(const std::shared_ptr<Identifier>& id) override {}

    };

    /**
     * Module scope is special
     */
    class ModuleScope : public Scope {
    public:
        using ChangeSet = std::vector<std::tuple<UString, UString>>;

        ModuleScope();

        bool BatchRenameSymbols(const ChangeSet& changeset) override;

        ImportManager import_manager;
        ExportManager export_manager;

    };

    class VariableExistsError : public std::exception {
    public:
        std::shared_ptr<Identifier> exist_var;
        UString name;

    };

}
