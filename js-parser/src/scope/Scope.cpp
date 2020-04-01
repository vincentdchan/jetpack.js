//
// Created by Duzhong Chen on 2020/3/21.
//

#include "Variable.h"
#include "Scope.h"
#include "../parser/SyntaxNodes.h"

namespace rocket_bundle {

    Scope::PVar
    Scope::RecursivelyFindVariable(const UString &var_name) {
        auto iter = own_variables.find(var_name);
        if (iter != own_variables.end()) {
            return iter->second;
        }
        if (parent == nullptr) {
            return nullptr;
        }
        return parent->RecursivelyFindVariable(var_name);
    }

    Scope::PVar
    Scope::CreateVariable(const std::shared_ptr<Identifier>& var_id, VarKind kind) {
        Scope* target_scope = this;

        if (kind == VarKind::Var) {
            while (target_scope->type != ScopeType::Function && target_scope->parent != nullptr) {
                target_scope = target_scope->parent;
            }
        }

        PVar var;
        if (auto iter = target_scope->own_variables.find(var_id->name); iter != target_scope->own_variables.end()) {
            if (kind == VarKind::Var) {
                var = iter->second;
            } else {
                VariableExistsError err;
                err.name = iter->second->name;
                if (iter->second->identifiers.empty()) {
                    throw std::runtime_error("identifiers can not be empty");
                }
                err.exist_var = iter->second->identifiers[0];
                throw std::move(err);
            }
        } else {  // contruct a new
            var = std::make_shared<Variable>();
            var->scope = target_scope;
            var->name = var_id->name;
            var->kind = kind;
            target_scope->own_variables[var_id->name] = var;
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
    void Scope::ResolveAllSymbols(std::vector<std::shared_ptr<Identifier>>* unresolve_collector) {
        for (auto iter = unresolved_id.begin(); iter != unresolved_id.end();) {
            auto var = RecursivelyFindVariable((*iter)->name);
            if (var != nullptr) {
                var->identifiers.push_back(*iter);
                iter = unresolved_id.erase(iter);
            } else {
                if (unresolve_collector) {
                    unresolve_collector->push_back(*iter);
                }
                iter++;
            }
        }

        for (auto child : children) {
            child->ResolveAllSymbols(unresolve_collector);
        }
    }

    bool Scope::BatchRenameSymbols(const std::vector<std::tuple<UString, UString>>& changeset) {
        std::vector<PVar> buffer;
        buffer.reserve(changeset.size());

        for (auto& tuple : changeset) {
            auto iter = own_variables.find(std::get<0>(tuple));
            if (iter == own_variables.end()) {
                return false;
            }
            PVar pvar = iter->second;

            buffer.push_back(pvar);
            pvar->name = std::get<1>(tuple);

            for (auto& id : pvar->identifiers) {
                id->name = std::get<1>(tuple);
            }

            own_variables.erase(iter);
        }

        for (auto& var : buffer) {
            own_variables[var->name] = std::move(var);
        }

        return true;
    }

    void Scope::SetParent(Scope* parent_) {
        parent = parent_;
        parent->children.push_back(this);
    }

    LeftValueScope LeftValueScope::default_;

    ModuleScope::ModuleScope() : Scope(ScopeType::Module) {
    };

    bool ModuleScope::BatchRenameSymbols(const std::vector<std::tuple<UString, UString>>& changeset) {
        if (!Scope::BatchRenameSymbols(changeset)) {
            return false;
        }

        std::vector<std::shared_ptr<LocalExportInfo>> info_changeset;

        for (auto& tuple : changeset) {
            auto iter = export_manager.local_exports_by_local_name.find(std::get<0>(tuple));
            if (iter == export_manager.local_exports_by_local_name.end()) {  // not a local export
                continue;
            }

            auto local_export_info = iter->second;
            export_manager.local_exports_by_local_name.erase(iter);  // remove old index

            local_export_info->local_name = std::get<1>(tuple);

            info_changeset.push_back(std::move(local_export_info));
        }

        for (auto& item : info_changeset) {
            export_manager.local_exports_by_local_name[item->local_name] = item;  // add new index
        }

        return true;
    }

    ModuleScope * Scope::CastToMoudle() {
        if (type != ScopeType::Module) return nullptr;
        return reinterpret_cast<ModuleScope*>(this);
    }

}
