//
// Created by Duzhong Chen on 2020/3/22.
//

#include "ExportManager.h"
#include "../parser/SyntaxNodes.h"
#include "../parser/BaseNodes.h"

namespace rocket_bundle {

    ExportManager::EC ExportManager::ResolveAllDecl(const std::shared_ptr<ExportAllDeclaration>& decl) {
        external_asts.emplace_back(decl);
        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveDefaultDecl(const std::shared_ptr<ExportDefaultDeclaration>& decl) {
        local_export_name.emplace_back(u"default");
        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveNamedDecl(const std::shared_ptr<ExportNamedDeclaration>& decl) {
        if (decl->source.has_value()) { // external export
            external_asts.emplace_back(decl);
            return EC::Ok;
        }
        // local export

        for (auto& spec : decl->specifiers) {
            local_export_name.push_back(spec->exported->name);
        }

        return EC::Ok;
    }

    std::vector<ExternalInfo> ExportManager::CollectExternalInfos() {
        std::vector<ExternalInfo> result;

        for (auto& weak_ast : external_asts) {
            auto ast = weak_ast.lock();
            switch (ast->type) {
                case SyntaxNodeType::ExportAllDeclaration: {
                    auto export_all_decl = std::dynamic_pointer_cast<ExportAllDeclaration>(ast);

                    ExternalInfo info;
                    info.is_export_all = true;
                    info.path = export_all_decl->source->str_;

                    result.emplace_back(std::move(info));
                    break;
                }

                case SyntaxNodeType::ExportNamedDeclaration: {
                    auto export_named_decl = std::dynamic_pointer_cast<ExportNamedDeclaration>(ast);

                    ExternalInfo info;
                    info.is_export_all = false;
                    info.path = (*export_named_decl->source)->str_;

                    for (auto& spec : export_named_decl->specifiers) {
                        info.names.push_back(spec->exported->name);
                    }

                    result.emplace_back(std::move(info));
                    break;
                }

                default:
                    break;

            }
        }

        return result;
    }

}
