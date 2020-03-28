//
// Created by Duzhong Chen on 2020/3/21.
//

#include "Variable.h"
#include "Scope.h"
#include "../parser/SyntaxNodes.h"

namespace rocket_bundle {

    Variable * Scope::RecursivelyFindVariable(const UString &var_name) {
        auto iter = own_variables.find(var_name);
        if (iter != own_variables.end()) {
            return &iter->second;
        }
        if (parent == nullptr) {
            return nullptr;
        }
        return parent->RecursivelyFindVariable(var_name);
    }

    Variable* Scope::CreateVariable(const std::shared_ptr<Identifier>& var_id, VarKind kind) {
        Scope* target_scope = this;

        if (kind == VarKind::Var) {
            while (target_scope->type != ScopeType::Function && target_scope->parent != nullptr) {
                target_scope = target_scope->parent;
            }
        }

        Variable* var = nullptr;
        if (auto iter = target_scope->own_variables.find(var_id->name); iter != target_scope->own_variables.end()) {
            if (kind == VarKind::Var) {
                var = &iter->second;
            } else {
                VariableExistsError err;
                err.name = iter->second.name;
                if (iter->second.identifiers.empty()) {
                    throw std::runtime_error("identifiers can not be empty");
                }
                err.exist_var = iter->second.identifiers[0];
                throw std::move(err);
            }
        } else {  // contruct a new
            var = &target_scope->own_variables[var_id->name];
            var->scope = target_scope;
            var->name = var_id->name;
            var->kind = kind;
        }

        var->identifiers.push_back(var_id);

        return var;
    }

    /**
     * Recursively resolve symbols.
     *
     * Add identifier to their scope.
     *
     * Do this after parsing.
     */
    void Scope::ResolveAllSymbols() {
        for (auto iter = unresolved_id.begin(); iter != unresolved_id.end();) {
            auto var = RecursivelyFindVariable((*iter)->name);
            if (var != nullptr) {
                var->identifiers.push_back(*iter);
                iter = unresolved_id.erase(iter);
            } else {
                iter++;
            }
        }

        for (auto child : children) {
            child->ResolveAllSymbols();
        }
    }

    bool Scope::RenameSymbol(const UString &old_name, const UString &new_name) {
        auto iter = own_variables.find(old_name);
        if (iter == own_variables.end()) {
            return false;
        }

        Variable tmp = iter->second;
        own_variables.erase(iter);

        tmp.name = new_name;
        for (auto& id : tmp.identifiers) {
            id->name = new_name;
        }

        own_variables[tmp.name] = std::move(tmp);

        return true;
    }

    void Scope::SetParent(Scope* parent_) {
        parent = parent_;
        parent->children.push_back(this);
    }

    LeftValueScope LeftValueScope::default_;

    ModuleScope::ModuleScope() : Scope(ScopeType::Module) {
    };

    bool ModuleScope::RenameSymbol(const UString &old_name, const UString &new_name) {
        if (!Scope::RenameSymbol(old_name, new_name)) {
            return false;
        }

        auto iter = export_manager.local_exports_by_local_name.find(old_name);
        if (iter == export_manager.local_exports_by_local_name.end()) {  // not a local export
            return true;
        }

        auto local_export_info = iter->second;
        export_manager.local_exports_by_local_name.erase(iter);  // remove old index

        local_export_info->local_name = new_name;

        export_manager.local_exports_by_local_name[local_export_info->local_name] = local_export_info;  // add new index

        return true;
    }

    ModuleScope * Scope::CastToMoudle() {
        if (type != ScopeType::Module) return nullptr;
        return reinterpret_cast<ModuleScope*>(this);
    }

}
