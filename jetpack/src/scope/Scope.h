//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once

#include <vector>
#include <list>
#include <memory>

#include "parser/AstContext.h"
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

        Scope(AstContext& ctx): ctx_(ctx) {}
        Scope(const Scope&) = delete;
        Scope(Scope&&) = delete;

        Scope& operator=(const Scope&) = delete;
        Scope& operator=(Scope&&) = delete;

        explicit Scope(ScopeType t, AstContext& ctx): type(t), ctx_(ctx) {
        }

        ScopeType type = ScopeType::Invalid;

        ModuleScope* CastToModule();

        inline bool IsModule() const {
            return type == ScopeType::Module;
        }

        virtual PVar RecursivelyFindVariable(const std::string& var_name);

        virtual PVar CreateVariable(Identifier* var_id, VarKind kind);

        virtual void AddUnresolvedId(Identifier* id) {
            unresolved_id.push_back(id);
        }

        void SetParent(Scope* parent_);

        inline bool RemoveVariable(const std::string& name) {
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
        void ResolveAllSymbols(std::vector<Identifier*>* unresolve_collector);

        virtual bool BatchRenameSymbols(const std::vector<std::tuple<std::string, std::string>>& changeset);

        virtual ~Scope() = default;

        HashMap<std::string, PVar> own_variables;

        std::vector<Scope*> children;

    protected:
        AstContext& ctx_;
        Scope* parent = nullptr;

        /**
         * log identifier when parsing
         */
        std::list<Identifier*> unresolved_id;

    };

    /**
     * Used in left value, do not record id
     */
    class LeftValueScope : public Scope {
    public:
        LeftValueScope(AstContext& ctx): Scope(ctx) {}

        PVar CreateVariable(Identifier* var_id, VarKind kind) override {
            return nullptr;
        }

        void AddUnresolvedId(Identifier* id) override {}

    };

    /**
     * Module scope is special
     */
    class ModuleScope : public Scope {
    public:
        using ChangeSet = std::vector<std::tuple<std::string, std::string>>;

        enum class ModuleType {
            EsModule,
            CommonJs,
        };

        ModuleScope(ModuleType mt, AstContext& ctx);

        bool BatchRenameSymbols(const ChangeSet& changeset) override;

        ImportManager import_manager;
        ExportManager export_manager;

        [[nodiscard]]
        inline ModuleType moduleType() const {
            return module_type_;
        }

    private:
        ModuleType module_type_;

    };

    class VariableExistsError : public std::exception {
    public:
        Identifier* exist_var;
        std::string name;

    };

}
