//
// Created by Duzhong Chen on 2020/3/20.
//

#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <parser/ParserCommon.h>
#include <fstream>
#include <iostream>
#include <memory>

#include "Path.h"
#include "./ModuleResolver.h"
#include "./Utils.h"
#include "./Error.h"
#include "./codegen/CodeGen.h"

namespace rocket_bundle {
    using parser::ParserContext;
    using parser::Parser;

    inline Sp<Identifier> MakeId(const UString& content) {
        auto id = std::make_shared<Identifier>();
        id->name = content;
        return id;
    }

    inline Sp<Identifier> MakeId(const std::string& content) {
        return MakeId(utils::To_UTF16(content));
    }

    inline Sp<SyntaxNode> MakeModuleVar(const UString& var_name) {
        auto mod_var = std::make_shared<VariableDeclaration>();
        mod_var->kind = VarKind::Const;

        auto declarator = std::make_shared<VariableDeclarator>();
        declarator->id = MakeId(var_name);

        declarator->init = { std::make_shared<ObjectExpression>() };

        mod_var->declarations.push_back(std::move(declarator));
        return mod_var;
    }

    void ModuleFile::CodeGenFromAst() {
        std::stringstream ss;
        ss << "// " << this->path << std::endl;
        CodeGen::Config config;
        CodeGen codegen(config, ss);
        codegen.Traverse(ast);
        codegen_result = ss.str();
    }

    /**
     * import { a, b as c } from './mod1';
     *
     * TO
     *
     * const { a, b: c } = mod_1;
     */
    std::vector<Sp<VariableDeclaration>> ModuleFile::HandleImportDeclaration(Sp<rocket_bundle::ImportDeclaration> &import_decl) {
        std::vector<Sp<VariableDeclaration>> result;

        auto full_path_iter = resolved_map.find(utils::To_UTF8(import_decl->source->str_));
        if (full_path_iter == resolved_map.end()) {
            throw std::runtime_error("can not resolver path: " + utils::To_UTF8(import_decl->source->str_));
        }

        auto resolver = module_resolver.lock();
        auto target_module = resolver->modules_map_[full_path_iter->second];

        if (target_module == nullptr) {
            throw std::runtime_error("can not find module: " + full_path_iter->second);
        }

        auto target_mode_name = target_module->GetModuleVarName();

        if (import_decl->specifiers.empty()) {
            return result;
        }

        if (import_decl->specifiers[0]->type == SyntaxNodeType::ImportNamespaceSpecifier) {
            auto import_ns = std::dynamic_pointer_cast<ImportNamespaceSpecifier>(import_decl->specifiers[0]);

            auto decl = std::make_shared<VariableDeclaration>();
            decl->kind = VarKind::Const;

            auto declarator = std::make_shared<VariableDeclarator>();

            declarator->id = MakeId(import_ns->local->name);  // try not to use old ast

            auto obj = std::make_shared<ObjectExpression>();

            auto __proto__ = std::make_shared<Property>();
            __proto__->key = MakeId("__proto__");

            auto null_lit = std::make_shared<Literal>();
            null_lit->ty = Literal::Ty::Null;
            null_lit->str_ = u"null";
            null_lit->raw = u"null";

            __proto__->value = null_lit;

            obj->properties.push_back(__proto__);

            declarator->init = { obj };

            decl->declarations.push_back(std::move(declarator));
            result.push_back(std::move(decl));
        } else {
//            auto result = std::make_shared<VariableDeclaration>();
//            result->kind = VarKind::Const;
//
//            auto pattern = std::make_shared<ObjectPattern>();
//
//            for (auto& spec : import_decl->specifiers) {
//                switch (spec->type) {
//                    case SyntaxNodeType::ImportDefaultSpecifier: {
//                        auto default_spec = std::dynamic_pointer_cast<ImportDefaultSpecifier>(spec);
//
//                        auto prop = std::make_shared<Property>();
//
//                        prop->key = MakeId("default");
//                        prop->value = { MakeId(default_spec->local->name) };
//
//                        pattern->properties.push_back(std::move(prop));
//                        break;
//                    }
//
//                    case SyntaxNodeType::ImportSpecifier: {
//                        auto named_spec = std::dynamic_pointer_cast<ImportSpecifier>(spec);
//
//                        auto prop = std::make_shared<Property>();
//
//                        prop->key = MakeId(named_spec->imported->name);
//                        if (named_spec->imported->name != named_spec->local->name) {
//                            prop->value = { MakeId(named_spec->local->name) };
//                        }
//
//                        pattern->properties.push_back(std::move(prop));
//                        break;
//                    }
//
//                    default:
//                        break;
//
//                }
//            }
//
//            declarator->id = std::move(pattern);
//            declarator->init = { MakeId(target_mode_name) };
//
//            result->declarations.push_back(declarator);
        }

        return result;
    }

    void ModuleFile::ReplaceAllNamedExports() {
        std::vector<Sp<SyntaxNode>> new_body;
//        new_body.push_back(MakeModuleVar(GetModuleVarName()));

        for (auto& stmt : ast->body) {
            switch (stmt->type) {
                case SyntaxNodeType::ImportDeclaration: {
                    auto import_decl = std::dynamic_pointer_cast<ImportDeclaration>(stmt);

                    auto tmp = HandleImportDeclaration(import_decl);
                    new_body.insert(std::end(new_body), std::begin(tmp), std::end(tmp));
                    continue;
                }

                case SyntaxNodeType::ExportNamedDeclaration: {
                    auto export_named_decl = std::dynamic_pointer_cast<ExportNamedDeclaration>(stmt);
                    if (export_named_decl->declaration.has_value()) {  // is local
                        new_body.push_back(*export_named_decl->declaration);
                    }
                    break;
                }

                case SyntaxNodeType::ExportDefaultDeclaration: {
                    auto export_default_decl = std::dynamic_pointer_cast<ExportDefaultDeclaration>(stmt);
                    auto resolver = module_resolver.lock();
                    std::string new_name = "default_" + std::to_string(resolver->NextNameId());

                    /**
                     * export defaut var1
                     *
                     * TO
                     *
                     * const defaut_0 = var1;
                     */
                    if (export_default_decl->declaration->type == SyntaxNodeType::AssignmentExpression) {
                        auto exist_assign = std::dynamic_pointer_cast<AssignmentExpression>(export_default_decl->declaration);

                        auto var_decl1 = std::make_shared<VariableDeclaration>();
                        var_decl1->kind = VarKind::Const;

                        auto dector1 = std::make_shared<VariableDeclarator>();

                        if (exist_assign->left->type != SyntaxNodeType::Identifier) {
                            std::runtime_error("left of assign should be identifer");
                        }

                        auto assign_left_id = std::dynamic_pointer_cast<Identifier>(exist_assign->left);
                        dector1->id = assign_left_id;
                        dector1->init = {  exist_assign->right };
                        var_decl1->declarations.push_back(dector1);

                        new_body.push_back(var_decl1);
                        // ---------------
                        auto var_decl2 = std::make_shared<VariableDeclaration>();
                        var_decl2->kind = VarKind::Const;

                        auto dector2 = std::make_shared<VariableDeclarator>();
                        dector2->id = MakeId(new_name);

                        auto right_id = std::make_shared<Identifier>();
                        right_id->name = assign_left_id->name;  // copy name

                        dector2->init = { right_id };

                        var_decl2->declarations.push_back(dector2);

                        new_body.push_back(var_decl2);

                        continue;
                    } else if (export_default_decl->declaration->IsExpression()) {
                        auto exist_id = std::dynamic_pointer_cast<Expression>(export_default_decl->declaration);

                        auto var_decl = std::make_shared<VariableDeclaration>();
                        var_decl->kind = VarKind::Const;

                        auto var_dector = std::make_shared<VariableDeclarator>();

                        var_dector->id = MakeId(new_name);
                        var_dector->init = { exist_id };

                        var_decl->declarations.push_back(var_dector);

                        new_body.push_back(var_decl);
                        continue;
                    }

                    switch (export_default_decl->declaration->type) {
                        /**
                         * Case 1:
                         *
                         * export default function() {
                         * }
                         *
                         * TO
                         *
                         * function default_0() {
                         * }
                         *
                         * Case 2:
                         *
                         * export default function name() {
                         * }
                         *
                         * TO
                         *
                         * function name() {
                         * }
                         *
                         * const default_0 = name;
                         *
                         */
                        case SyntaxNodeType::FunctionDeclaration: {
                            auto fun_decl = std::dynamic_pointer_cast<FunctionDeclaration>(export_default_decl->declaration);

                            if (fun_decl->id.has_value()) {
                                stmt = fun_decl;

                                auto var_decl = std::make_shared<VariableDeclaration>();
                                var_decl->kind = VarKind::Const;

                                auto dector = std::make_shared<VariableDeclarator>();

                                dector->id = MakeId(new_name);

                                auto right_id = std::make_shared<Identifier>();
                                right_id->name = (*fun_decl->id)->name;

                                dector->init = { right_id };

                                var_decl->declarations.push_back(dector);

                                new_body.push_back(stmt);
                                new_body.push_back(var_decl);
                            } else {
                                fun_decl->id = { MakeId(new_name) };

                                new_body.push_back(fun_decl);
                            }
                            break;
                        }

                        /**
                         * Similar to FunctionDeclaration
                         */
                        case SyntaxNodeType::ClassDeclaration: {
                            auto cls_decl = std::dynamic_pointer_cast<ClassDeclaration>(export_default_decl->declaration);

                            if (cls_decl->id.has_value()) {
                                stmt = cls_decl;

                                auto var_decl = std::make_shared<VariableDeclaration>();
                                var_decl->kind = VarKind::Const;

                                auto dector = std::make_shared<VariableDeclarator>();

                                dector->id = MakeId(new_name);

                                auto right_id = std::make_shared<Identifier>();
                                right_id->name = (*cls_decl->id)->name;

                                dector->init = { right_id };

                                var_decl->declarations.push_back(dector);

                                new_body.push_back(stmt);
                                new_body.push_back(var_decl);
                            } else {
                                cls_decl->id = { MakeId(new_name) };

                                new_body.push_back(cls_decl);
                            }
                            break;
                        }

                        default:
                            break;

                    }

                    break;
                }

                /**
                 * remove export all
                 */
                case SyntaxNodeType::ExportAllDeclaration:
                    continue;

                default:
                    new_body.push_back(stmt);
                    break;

            }
        }

//        ast->body.insert(std::end(ast->body), std::begin(tail), std::end(tail));
        ast->body = std::move(new_body);
    }

    UString ModuleFile::GetModuleVarName() {
        std::string tmp = "mod_" + std::to_string(id);
        return utils::To_UTF16(tmp);
    }

    void ModuleResolver::BeginFromEntry(std::string base_path, std::string target_path) {
        std::string path;
        if (target_path.empty()) {
            return;
        } else if (target_path[0] == Path::PATH_DIV) {
            path = target_path;
        } else {
            Path path(base_path);
            path.Join(target_path);
        }

        auto thread_pool_size = std::thread::hardware_concurrency();
        thread_pool_ = std::make_unique<ThreadPool>(thread_pool_size);

        EnqueueOne([this, &path] {
            try {
                ParseFileFromPath(path);
            } catch (parser::ParseError& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                WorkerError err { path, ex.ErrorMessage() };
                worker_errors_.emplace_back(std::move(err));
            } catch (std::exception& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                WorkerError err { path, ex.what() };
                worker_errors_.emplace_back(std::move(err));
            }
            FinishOne();
        });

        std::unique_lock<std::mutex> lk(main_lock_);
        main_cv_.wait(lk, [this] {
            return finished_files_count_ >= enqueued_files_count_;
        });
    }

    void ModuleResolver::ParseFileFromPath(const std::string &path) {
        if (!utils::IsFileExist(path)) {
            Path exist_path(path);
            if (!exist_path.EndsWith(".js")) {
                exist_path.slices[exist_path.slices.size() - 1] += ".js";
                ParseFileFromPath(exist_path.ToString());
                return;
            }

            WorkerError err { path, std::string("file doen't exist: ") + path };
            worker_errors_.emplace_back(std::move(err));
            return;
        }
        entry_module = std::make_shared<ModuleFile>();
        entry_module->id = mod_counter_++;
        entry_module->module_resolver = shared_from_this();
        entry_module->path = path;

        {
            std::lock_guard<std::mutex> guard(map_mutex_);
            if (modules_map_.find(path) != modules_map_.end()) {
                return;  // exists;
            }
            modules_map_[path] = entry_module;
        }

        ParseFile(entry_module);
    }

    void ModuleResolver::ParseFile(Sp<ModuleFile> mf) {
        ParserContext::Config config = ParserContext::Config::Default();
        auto src = std::make_shared<UString>();
        (*src) = ReadFileStream(mf->path);
        auto ctx = std::make_shared<ParserContext>(src, config);
        Parser parser(ctx);

        parser.OnNewImportLocationAdded([this, &mf] (const UString& path) {
            if (!trace_file) return;

            auto u8path = utils::To_UTF8(path);
            Path module_path(mf->path);
            module_path.Pop();
            module_path.Join(u8path);

            auto source_path = module_path.ToString();

            if (!utils::IsFileExist(source_path)) {
                if (!module_path.EndsWith(".js")) {
                    module_path.slices[module_path.slices.size() - 1] += ".js";
                    source_path = module_path.ToString();
                }

                if (!utils::IsFileExist(source_path)) {
                    WorkerError err { source_path, std::string("file doesn't exist: ") + source_path };
                    worker_errors_.emplace_back(std::move(err));
                    return;
                }
            }

            mf->resolved_map[u8path] = source_path;

            std::shared_ptr<ModuleFile> new_mf;
            {
                std::lock_guard<std::mutex> guard(map_mutex_);
                if (modules_map_.find(source_path) != modules_map_.end()) return;  // exists

                new_mf = std::make_shared<ModuleFile>();
                new_mf->id = mod_counter_++;
                new_mf->module_resolver = shared_from_this();
                new_mf->path = std::move(source_path);
                modules_map_[new_mf->path] = new_mf;
            }

            mf->ref_mods.push_back(new_mf);

            EnqueueOne([this, new_mf] {
                try {
                    ParseFile(new_mf);
                } catch (parser::ParseError& ex) {
                    std::lock_guard<std::mutex> guard(error_mutex_);
                    WorkerError err {new_mf->path, ex.ErrorMessage() };
                    worker_errors_.emplace_back(std::move(err));
                } catch (std::exception& ex) {
                    std::lock_guard<std::mutex> guard(error_mutex_);
                    WorkerError err {new_mf->path, ex.what() };
                    worker_errors_.emplace_back(std::move(err));
                }
                FinishOne();
            });
        });

        mf->ast = parser.ParseModule();
        mf->ast->scope->ResolveAllSymbols();
    }

    std::u16string ModuleResolver::ReadFileStream(const std::string& filename) {
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());
        return utils::To_UTF16(str);
    }

    void ModuleResolver::PrintStatistic() {
        std::unique_lock<std::mutex> lk(error_mutex_);
        if (!worker_errors_.empty()) {
            PrintErrors();
            return;
        }
        lk.unlock();

        ModuleScope& mod_scope = *entry_module->ast->scope;

        json result = json::object();
        result["entry"] = entry_module->path;
        result["importStat"] = GetImportStat();
        result["totalFiles"] = finished_files_count_;
        result["exports"] = GetAllExportVars();

        if (!worker_errors_.empty()) {
            PrintErrors();
            return;
        }

        std::cout << result.dump(2) << std::endl;
    }

    std::vector<UString> ModuleResolver::GetAllExportVars() {
        std::vector<UString> result;

        TraverseModulePushExportVars(result, entry_module, nullptr);

        return result;
    }

    void ModuleResolver::TraverseModulePushExportVars(
            std::vector<UString>& arr, const Sp<rocket_bundle::ModuleFile>& mod,
            std::unordered_set<UString>* white_list) {

        if (mod->visited_mark) {
            return;
        }
        mod->visited_mark = true;

        for (auto& local_name : mod->ast->scope->export_manager.local_export_name) {
            if (white_list && white_list->find(local_name) == white_list->end()) {
                continue;
            }
            arr.push_back(local_name);
        }

        auto external_infos = mod->ast->scope->export_manager.CollectExternalInfos();

        for (auto& info : external_infos) {
            auto u8relative_path = utils::To_UTF8(info.path);
            auto resolved_path = mod->resolved_map.find(u8relative_path);
            if (resolved_path == mod->resolved_map.end()) {
                WorkerError err { mod->path, std::string("resolve path failed: ") + u8relative_path };
                worker_errors_.emplace_back(std::move(err));
                return;
            }

            auto iter = modules_map_.find(resolved_path->second);
            if (iter == modules_map_.end()) {
                WorkerError err { mod->path, "module not found: " + resolved_path->second };
                worker_errors_.emplace_back(std::move(err));
                return;
            }
            if (info.is_export_all) {
                TraverseModulePushExportVars(arr, iter->second, nullptr);
            } else {
                std::unordered_set<UString> new_white_list;
                new_white_list.insert(std::begin(info.names), std::end(info.names));
                TraverseModulePushExportVars(arr, iter->second, &new_white_list);
            }
        }
    }

    void ModuleResolver::PrintErrors() {
        for (auto& error : worker_errors_) {
            std::cerr << "File: " << error.file_path << std::endl;
            std::cerr << "Error: " << error.error_content << std::endl;
        }
    }

    void ModuleResolver::EnqueueOne(std::function<void()> unit) {
        {
            std::lock_guard<std::mutex> lk(main_lock_);
            enqueued_files_count_++;
        }

        thread_pool_->enqueue(std::move(unit));
    }

    void ModuleResolver::FinishOne() {
        {
            std::lock_guard<std::mutex> lk(main_lock_);
            finished_files_count_++;
        }
        main_cv_.notify_one();
    }

    json ModuleResolver::GetImportStat() {
        json result = json::object();

        auto module_ast = entry_module->ast;
        ModuleScope* mod_scope = module_ast->scope.get();

        auto& import_manager = mod_scope->import_manager;

        for (auto& tuple : import_manager.id_map) {
            result[utils::To_UTF8(tuple.first)] = utils::To_UTF8(tuple.second.module_name);
        }

        return result;
    }

    void ModuleResolver::CodeGenAllModules(const std::string& out_path) {
        enqueued_files_count_ = 0;
        finished_files_count_ = 0;


        // BEGIN every modules gen their own code
        for (auto& tuple : modules_map_) {
            EnqueueOne([this, mod = tuple.second] {
                mod->ReplaceAllNamedExports();
                mod->CodeGenFromAst();
                FinishOne();
            });
        }

        std::unique_lock<std::mutex> lk(main_lock_);
        main_cv_.wait(lk, [this] {
            return finished_files_count_ >= enqueued_files_count_;
        });

        // END every modules gen their own code

        std::ofstream out(out_path, std::ios::out);

        for (auto& tuple : modules_map_) {
            tuple.second->visited_mark = false;
        }

        MergeModules(entry_module, out);

        auto final_export = GenFinalExportDecl();
        CodeGen::Config config;
        CodeGen codegen(config, out);
        codegen.Traverse(final_export);

        out.close();
    }

    void ModuleResolver::MergeModules(const Sp<ModuleFile> &mf, std::ofstream &out) {
        if (mf->visited_mark) {
            return;
        }
        mf->visited_mark = true;

        for (auto& ref : mf->ref_mods) {
            auto new_mf = ref.lock();
            MergeModules(new_mf, out);
        }

        out << mf->codegen_result << std::endl;
    }

    Sp<ExportNamedDeclaration> ModuleResolver::GenFinalExportDecl() {
        auto result = std::make_shared<ExportNamedDeclaration>();

        for (auto& tuple : modules_map_) {
            tuple.second->visited_mark = false;
        }

        auto export_names = GetAllExportVars();
        for (auto& name : export_names) {
            auto spec = std::make_shared<ExportSpecifier>();
            spec->local = MakeId(name);
            spec->exported = MakeId(name);
            result->specifiers.push_back(std::move(spec));
        }

        return result;
    }

}
