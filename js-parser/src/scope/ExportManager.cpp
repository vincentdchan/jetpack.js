//
// Created by Duzhong Chen on 2020/3/22.
//

#include "ExportManager.h"
#import "../parser/SyntaxNodes.h"

namespace rocket_bundle {

    ExportManager::EC ExportManager::ResolveAllDecl(const std::shared_ptr<ExportAllDeclaration>& decl) {
        ExternalVariable ext_var;
        ext_var.is_export_all = true;
        ext_var.source_name = decl->source->str_;

        external_export_vars[decl->source->str_] = ext_var;

        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveDefaultDecl(const std::shared_ptr<ExportDefaultDeclaration>& decl) {
        local_export_name.emplace_back(u"default");
        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveNamedDecl(const std::shared_ptr<ExportNamedDeclaration>& decl) {
        if (decl->source.has_value()) { // external export
            ExternalVariable ext_var;
            ext_var.source_name = (*decl->source)->str_;
            ext_var.is_export_all = false;
            ext_var.is_export_all = false;
            for (auto& spec : decl->specifiers) {
                ext_var.export_names.push_back(spec->exported->name);
            }

            external_export_vars[(*decl->source)->str_] = ext_var;

            return EC::Ok;
        }
        // local export

        for (auto& spec : decl->specifiers) {
            local_export_name.push_back(spec->exported->name);
        }

        return EC::Ok;
    }

}
