//
// Created by Duzhong Chen on 2020/3/21.
//

#include "Variable.h"
#include "Scope.h"

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

    /**
     * Js allow using a variable before it's declared,
     * when a variable is declared, remove it from free variable.
     */
    Variable* Scope::CreateVariable(const UString &var_name, VarKind kind) {
        switch (kind) {
            case VarKind::Var:
                if (auto iter = free_var_names.find(var_name); iter != free_var_names.end()) {
                    free_var_names.erase(iter);
                }
                break;

            case VarKind::Let:
            case VarKind::Const:
                if (auto iter = free_var_names.find(var_name); iter != free_var_names.end()) {
                    // throw error
                    return nullptr;
                }
                break;

            default:
                // normally
                return nullptr;

        }
        Variable& var = own_variables[var_name];
        var.scope = this;
        var.name = var_name;
        var.kind = kind;
        return &var;
    }

    void Scope::FindOrAddFreeVarInCurrentScope(const UString &name) {
        if (auto iter = own_variables.find(name); iter != own_variables.end()) {
            return;
        }
        free_var_names.insert(name);
    }

    void Scope::SetParent(Scope* parent_) {
        parent = parent_;
        parent->children.push_back(this);
    }

    void Scope::AddFreeVariableName(const UString &free_var_name) {
        free_var_names.insert(free_var_name);
    }

    ModuleScope::ModuleScope() : Scope(ScopeType::Module) {

    };

    ModuleScope * Scope::CastToMoudle() {
        if (type != ScopeType::Module) return nullptr;
        return reinterpret_cast<ModuleScope*>(this);
    }

}
