//
// Created by Duzhong Chen on 2020/4/3.
//

#include <algorithm>
#include "GlobalImportHandler.h"
#include "parser/NodesMaker.h"

namespace jetpack {

    void GlobalImportHandler::HandleImport(ImportDeclaration* import) {
        std::lock_guard<std::mutex> guard(m);

        imports.push_back(import);
        external_import_ptrs.insert(reinterpret_cast<std::intptr_t>(import));

        auto& path = import->source->str_;
        for (auto& spec : import->specifiers) {
            switch (spec->type) {
                case SyntaxNodeType::ImportNamespaceSpecifier: {
                    auto ns = spec->As<ImportNamespaceSpecifier>();

                    auto& info = import_infos[path];
                    if (info.id < 0) {
                        info.id = import_counter++;
                        info.path = path;
                    }
                    info.has_namespace = true;
                    info.ns_import_name = ns->local->name;
                    info.names.push_back(ns->local->name);
                    break;
                }

                case SyntaxNodeType::ImportSpecifier: {
                    auto import_spec = spec->As<ImportSpecifier>();

                    auto& info = import_infos[path];
                    if (info.id < 0) {
                        info.id = import_counter++;
                        info.path = path;
                    }
                    info.names.push_back(import_spec->imported->name);
                    break;
                }

                case SyntaxNodeType::ImportDefaultSpecifier: {
                    auto default_spec = spec->As<ImportDefaultSpecifier>();
                    auto& info = import_infos[path];
                    if (info.id < 0) {
                        info.id = import_counter++;
                        info.path = path;
                    }
                    info.has_default = true;
                    info.default_local_name = default_spec->local->name;
                    break;
                }

                default:
                    // ignore
                    break;

            }

        }
    }

    bool GlobalImportHandler::IsImportExternal(ImportDeclaration* import) {
        return external_import_ptrs.find(reinterpret_cast<std::intptr_t>(import)) != external_import_ptrs.end();
    }

    void GlobalImportHandler::DistributeNameToImportVars(const std::shared_ptr<UniqueNameGenerator>& generator,
                                                         const std::vector<GlobalImportInfo*>& infos) {

        for (GlobalImportInfo* info : infos) {
            if (info->has_namespace) {
                auto name = generator->Next(info->ns_import_name);
                if (name.has_value()) {
                    info->ns_import_name = *name;
                }
            }
            if (info->has_default) {
                auto name = generator->Next(info->default_local_name);
                if (name.has_value()) {
                    info->default_local_name = *name;
                }
            }

            for (auto& name : info->names) {
                auto alias = generator->Next(name);
                if (alias.has_value()) {
                    info->alias_map[name] = *alias;
                }
            }
        }
    }

    void GlobalImportHandler::GenAst(const std::shared_ptr<UniqueNameGenerator>& generator) {
        std::vector<GlobalImportInfo*> infos;

        for (auto& tuple : import_infos) {
            infos.push_back(&tuple.second);
        }

        /**
         * FIFO
         * keep the import order
         */
        std::sort(std::begin(infos), std::end(infos), [] (GlobalImportInfo* a, GlobalImportInfo* b) {
            return a->id < b->id;
        });

        DistributeNameToImportVars(generator, infos);

        for (GlobalImportInfo* import_info : infos) {
            if (import_info->has_namespace) {
                auto import_ns_decl = ctx_.Alloc<ImportDeclaration>();

                auto ns_spec = ctx_.Alloc<ImportNamespaceSpecifier>();
                ns_spec->local = MakeId(ctx_, import_info->ns_import_name);

                import_ns_decl->specifiers.push_back(std::move(ns_spec));

                import_ns_decl->source = MakeStringLiteral(ctx_, import_info->path);
                gen_import_decls.push_back(import_ns_decl);
            }
            if (!import_info->names.empty() || import_info->has_default) {
                auto import_decl = ctx_.Alloc<ImportDeclaration>();
                import_decl->source = MakeStringLiteral(ctx_, import_info->path);

                if (import_info->has_default) {
                    auto default_spec = ctx_.Alloc<ImportDefaultSpecifier>();
                    default_spec->local = MakeId(ctx_, import_info->default_local_name);
                    import_decl->specifiers.push_back(default_spec);
                }

                HashSet<std::string> visited_names;

                for (auto& name : import_info->names) {
                    if (visited_names.find(name) != visited_names.end()) {
                        continue;
                    }
                    visited_names.insert(name);

                    auto spec = ctx_.Alloc<ImportSpecifier>();
                    spec->imported = MakeId(ctx_, name);

                    if (auto iter = import_info->alias_map.find(name); iter != import_info->alias_map.end()) {
                        spec->local = MakeId(ctx_, iter->second);
                    } else {
                        spec->local = MakeId(ctx_, name);
                    }

                    import_decl->specifiers.push_back(spec);
                }

                gen_import_decls.push_back(import_decl);
            }
        }
    }

    void GlobalImportHandler::GenCode(CodeGen& codegen) {
        for (auto& decl : gen_import_decls) {
            codegen.Traverse(*decl);
        }
    }

}
