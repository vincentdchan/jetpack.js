//
// Created by Duzhong Chen on 2020/3/22.
//

#include "ExportManager.h"
#include "parser/SyntaxNodes.h"
#include "parser/BaseNodes.h"

namespace jetpack {

    const char * ExportManager::ECToStr(EC ec) {
        switch (ec) {
            case UnknownSpecifier:
                return "unkown specifier type";

            case FunctionHasNoId:
                return "A function should has a name";

            case ClassHasNoId:
                return "A class should has a name";

            case UnsupportExport:
                return "Unsupport export declaration";

            default:
                return "unknown error";

        }

    }

    ExportManager::EC ExportManager::ResolveAllDecl(ExportAllDeclaration* decl) {
        ExternalExportInfo info;
        info.relative_path = decl->source->str_;
        info.is_export_all = true;

        external_exports_map[info.relative_path] = info;
        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveDefaultDecl(ExportDefaultDeclaration* decl) {
        auto info = std::make_shared<LocalExportInfo>();
        info->export_name = "default";
        info->default_export_ast = { decl };

        AddLocalExport(info);
        return EC::Ok;
    }

    ExportManager::EC ExportManager::ResolveNamedDecl(ExportNamedDeclaration* decl) {
        if (decl->source.has_value()) { // external export
            ExternalExportInfo info;
            info.relative_path = (*decl->source)->str_;
            info.is_export_all = false;

            for (auto& spec : decl->specifiers) {
                ExternalExportAlias alias;
                alias.source_name = spec->local->name;
                alias.export_name = spec->exported->name;
                info.names.push_back(std::move(alias));
            }

            external_exports_map[info.relative_path] = std::move(info);
            return EC::Ok;
        }

        if (decl->declaration.has_value()) {
            switch ((*decl->declaration)->type) {
                case SyntaxNodeType::FunctionDeclaration: {
                    auto fun_decl = dynamic_cast<FunctionDeclaration*>(*decl->declaration);

                    auto info = std::make_shared<LocalExportInfo>();
                    if (!fun_decl->id.has_value()) {
                        return EC::FunctionHasNoId;
                    }
                    info->local_name = (*fun_decl->id)->name;
                    info->export_name = info->local_name;

                    AddLocalExport(info);
                    break;
                }

                case SyntaxNodeType::ClassDeclaration: {
                    auto cls_decl = dynamic_cast<ClassDeclaration*>(*decl->declaration);

                    auto info = std::make_shared<LocalExportInfo>();
                    if (!cls_decl->id.has_value()) {
                        return EC::ClassHasNoId;
                    }
                    info->local_name = (*cls_decl->id)->name;
                    info->export_name = info->local_name;

                    AddLocalExport(info);
                    break;
                }

                case SyntaxNodeType::VariableDeclaration: {
                    auto var_decl = dynamic_cast<VariableDeclaration*>(*decl->declaration);

                    for (auto& spec : var_decl->declarations) {
                        auto info = std::make_shared<LocalExportInfo>();

                        if (spec->id->type != SyntaxNodeType::Identifier) {
                            return EC::UnsupportExport;
                        }
                        auto id = dynamic_cast<Identifier*>(spec->id);

                        info->local_name = id->name;
                        info->export_name = info->local_name;

                        AddLocalExport(info);
                    }

                    break;
                }

                default:
                    return EC::UnknownSpecifier;

            }
        } else {
            // local export
            for (auto& spec : decl->specifiers) {
                auto info = std::make_shared<LocalExportInfo>();
                info->local_name = spec->local->name;
                info->export_name = spec->exported->name;
                AddLocalExport(info);
            }
        }

        return EC::Ok;
    }

    void ExportManager::AddLocalExport(const std::shared_ptr<LocalExportInfo> &info) {
        local_exports_name[info->export_name] = info;
        local_exports_by_local_name[info->local_name] = info;
    }

    std::vector<ExternalExportInfo> ExportManager::CollectExternalInfos() {
        std::vector<ExternalExportInfo> result;

        for (auto& tuple : external_exports_map) {
            result.push_back(tuple.second);
        }

        return result;
    }

}
