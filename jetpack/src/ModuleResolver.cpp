//
// Created by Duzhong Chen on 2020/3/20.
//

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <stack>
#include <set>

#include "utils/JetJSON.h"
#include "utils/Path.h"
#include "utils/io/FileIO.h"
#include "parser/ParserCommon.h"
#include "parser/NodesMaker.h"
#include "ModuleResolver.h"
#include "Benchmark.h"

static const char* COMMON_JS_CODE =
    "let __commonJS = (callback, module) => () => {\n"
    "  if (!module) {\n"
    "    module = {exports: {}};\n"
    "    callback(module.exports, module);\n"
    "  }\n"
    "  return module.exports;\n"
    "};";

static HashSet<std::string> NODE_JS_BUILTIN_MODULE = {
    "assert", "buffer", "child_process", "cluster",
    "crypto", "dgram", "dns", "events", "fs", "http",
    "https", "net", "os", "path", "querystring" "readline",
    "stream", "string_decoder", "timers", "tls", "tty",
    "url", "util", "v8", "vm", "zlib",
};

namespace jetpack {
    using fmt::format;
    using parser::ParserContext;
    using parser::Parser;

    static const char* PackageJsonName = "package.json";

    ModuleResolveException::ModuleResolveException(const std::string& path, const std::string& content)
    : file_path(path), error_content(content) {

    }

    void WorkerErrorCollection::PrintToStdErr() {
        for (auto& err : errors) {
            std::cerr << "File: " << err.file_path << std::endl;
            std::cerr << "Error: " << err.error_content << std::endl;
        }
    }

    void ModuleResolveException::PrintToStdErr() {
        std::cerr << "File: " << file_path << std::endl;
        std::cerr << "Error: " << error_content << std::endl;
    }

    void ModuleResolver::BeginFromEntry(const parser::Config& config, const std::string& targetPath, const std::string& basePathOverride) {
        std::string absolutePath;
        if (targetPath.empty()) {
            return;
        } else if (targetPath[0] == Path::PATH_DIV) {
            absolutePath = targetPath;
        } else {
            Path p(utils::GetRunningDir());
            p.Join(targetPath);
            absolutePath = p.ToString();
        }

        auto basePath = basePathOverride.empty() ? FindPathOfPackageJson(absolutePath) : basePathOverride;
        if (unlikely(!basePath.has_value())) {
            basePath = { utils::GetRunningDir() };
        }

        // push file provider
        auto fileProvider = std::make_shared<FileModuleProvider>(*basePath);
        providers_.push_back(fileProvider);

        pBeginFromEntry(fileProvider, config, absolutePath.substr(basePath->size() + 1));
    }

    void ModuleResolver::BeginFromEntryString(const parser::Config& config,
                                              const std::string& src) {
        std::string m0("memory0");

        // push file provider
        providers_.push_back(std::make_shared<FileModuleProvider>(utils::GetRunningDir()));
        auto memProvider = std::make_shared<MemoryModuleProvider>(m0, src);
        providers_.push_back(memProvider);

        pBeginFromEntry(memProvider, config, m0);
    }

    void ModuleResolver::ParseFileFromPath(const Sp<ModuleProvider>& rootProvider,
                                           const parser::Config& config,
                                           const std::string &path) {
        bool isNew = false;
        entry_module = modules_table_.createNewIfNotExists(path, isNew);
        if (!isNew) {
            return;  // exists;
        }

        entry_module->provider = rootProvider;
        entry_module->module_resolver = weak_from_this();

        ParseFile(config, entry_module);
    }

    bool ModuleResolver::IsExternalImportModulePath(const std::string &path) {
        // TODO: if alias path

        if (path.empty()) {
            return false;
        }

        return path[0] != '.' && path[0] != '/';
    }

    void ModuleResolver::ParseFile(const parser::Config& config,
                                   Sp<ModuleFile> mf) {
        WorkerError error;
        if (!mf->GetSource(error)) {
            std::lock_guard<std::mutex> guard(error_mutex_);
            worker_errors_.push_back(error);
            return;
        }

        Parser parser(mf->ast_context, mf->src_content, config);
        auto ctx = parser.Context();
        ctx->SetFileIndex(mf->id());

        if (mf->IsCommonJS()) {
            ctx->is_common_js_ = true;
        }

        parser.import_decl_created_listener.On([this, &config, &mf] (ImportDeclaration* import_decl) {
            const auto& u8path = import_decl->source->str_;
            if (IsExternalImportModulePath(u8path)) {
                global_import_handler_.HandleImport(import_decl);
                return;
            }
            HandleNewLocationAdded(config, mf, LocationImported, u8path);
        });
        parser.export_named_decl_created_listener.On([this, &config, &mf] (ExportNamedDeclaration* export_decl) {
            if (export_decl->source.has_value()) {
                const auto& u8path = (*export_decl->source)->str_;
                HandleNewLocationAdded(config, mf, LocationExported, u8path);
            }
        });
        parser.export_all_decl_created_listener.On([this, &config, &mf] (ExportAllDeclaration* export_decl) {
            const auto& u8path = export_decl->source->str_;
            HandleNewLocationAdded(config, mf, LocationExported, u8path);
        });
        if (config.common_js) {
            parser.require_call_created_listener.On([this, &config, &mf](CallExpression* call) -> std::optional<SyntaxNode*> {
                auto lit = dynamic_cast<Literal*>(call->arguments[0]);
                const auto& u8path = lit->str_;
                if (NODE_JS_BUILTIN_MODULE.find(u8path) != NODE_JS_BUILTIN_MODULE.end()) {
                    return std::nullopt;
                }
                auto child_mod = HandleNewLocationAdded(
                        config,
                        mf,
                        LocationAddOptions(LocationImported | LocationIsCommonJS),
                        u8path
                        );
                auto new_call = mf->ast_context.Alloc<CallExpression>();
                new_call->callee = MakeId(mf->ast_context, SourceLocation(-2, Position(), Position()), child_mod->cjs_call_name);
                return { new_call };
            });
        }

        benchmark::BenchMarker bench(benchmark::BENCH_PARSING);
        mf->ast = parser.ParseModule();
        bench.Submit();

        std::vector<Identifier*> unresolved_ids;
        mf->ast->scope->ResolveAllSymbols(&unresolved_ids);

        id_logger_->InsertByList(unresolved_ids);
    }

    Sp<ModuleFile> ModuleResolver::HandleNewLocationAdded(const jetpack::parser::Config &config,
                                                const Sp<jetpack::ModuleFile> &mf, LocationAddOptions flags,
                                                const std::string &path) {
        if (unlikely(!trace_file)) return nullptr;

        auto matchResult = FindProviderByPath(mf, path);
        if (matchResult.first == nullptr) {
            WorkerError err {mf->Path(), std::string("module can't be resolved: ") + path };
            worker_errors_.emplace_back(std::move(err));
            return nullptr;
        }

        mf->resolved_map[path] = matchResult.second;

        bool isNew = false;
        Sp<ModuleFile> childMod = modules_table_.createNewIfNotExists(matchResult.second, isNew);
        if (!isNew) {
            mf->ref_mods.push_back(childMod);
            return childMod;
        }
        childMod->provider = matchResult.first;
        childMod->module_resolver = shared_from_this();
        if (!!(flags & LocationAddOption::LocationIsCommonJS)) {
            std::lock_guard<std::mutex> guard(main_lock_);
            childMod->SetIsCommonJS(true);
            auto name = name_generator->Next("jp_require");
            if (name.has_value()) {
                childMod->cjs_call_name = *name;
            } else {
                childMod->cjs_call_name = "jp_require";
            }
        }
        if (childMod->IsCommonJS()) {
            has_common_js_.store(true);
        }

        mf->ref_mods.push_back(childMod);

        EnqueueOne([this, &config, childMod] {
            try {
                ParseFile(config, childMod);
            } catch (parser::ParseError& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                worker_errors_.push_back({childMod->Path(), ex.ErrorMessage() });
            } catch (VariableExistsError& err) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                std::string message = format("variable '{}' has been defined, location: {}:{}",
                                             err.name,
                                             err.exist_var->location.start.line + 1,
                                             err.exist_var->location.start.column);
                worker_errors_.push_back({childMod->Path(), std::move(message) });
            } catch (std::exception& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                worker_errors_.push_back({childMod->Path(), ex.what() });
            }
            FinishOne();
        });
        return childMod;
    }

    void ModuleResolver::PrintStatistic() {
        std::unique_lock<std::mutex> lk(error_mutex_);
        if (!worker_errors_.empty()) {
            PrintErrors();
            return;
        }
        lk.unlock();

        ModuleScope& mod_scope = *entry_module->ast->scope;

        json exports = json::array();
        for (auto& tuple : GetAllExportVars()) {
            exports.push_back(std::get<1>(tuple));
        }

        json result = json::object();
        result["entry"] = entry_module->Path();
        result["importStat"] = GetImportStat();
        result["totalFiles"] = finished_files_count_;
        result["exports"] = std::move(exports);

        if (!worker_errors_.empty()) {
            PrintErrors();
            return;
        }

        std::cout << result.dump(2) << std::endl;
    }

    std::vector<std::tuple<Sp<ModuleFile>, std::string>> ModuleResolver::GetAllExportVars() {
        std::vector<std::tuple<Sp<ModuleFile>, std::string>> result;

        TraverseModulePushExportVars(result, entry_module, nullptr);

        return result;
    }

    void ModuleResolver::pBeginFromEntry(const Sp<ModuleProvider>& rootProvider,
                                         const parser::Config &config,
                                         const std::string &resolvedPath) {
        auto thread_pool_size = std::thread::hardware_concurrency();
        thread_pool_ = std::make_unique<ThreadPool>(thread_pool_size);

        benchmark::BenchMarker ps(benchmark::BENCH_PARSING_STAGE);
        EnqueueOne([this, &config, &resolvedPath, &rootProvider] {
            try {
                ParseFileFromPath(rootProvider, config, resolvedPath);
            } catch (parser::ParseError& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                worker_errors_.push_back({ resolvedPath, ex.ErrorMessage() });
            } catch (VariableExistsError& err) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                std::string message = format("variable '{}' has been defined, location: {}:{}",
                                             err.name,
                                             err.exist_var->location.start.line,
                                             err.exist_var->location.start.column);
                worker_errors_.push_back({ resolvedPath, std::move(message) });
            } catch (std::exception& ex) {
                std::lock_guard<std::mutex> guard(error_mutex_);
                worker_errors_.push_back({ resolvedPath, ex.what() });
            }
            FinishOne();
        });

        std::unique_lock<std::mutex> lk(main_lock_);
        main_cv_.wait(lk, [this] {
            return finished_files_count_ >= enqueued_files_count_;
        });
        ps.Submit();

        if (!worker_errors_.empty()) {
            WorkerErrorCollection col;
            col.errors = std::move(worker_errors_);
            throw std::move(col);
        }
    }

    /**
     * white_list: only export specific names, example:
     *
     * export { a as b } from './another';
     *
     * Then a would be in white list
     */
    void ModuleResolver::TraverseModulePushExportVars(std::vector<std::tuple<Sp<ModuleFile>, std::string>>& arr,
                                                      const Sp<jetpack::ModuleFile>& mod,
                                                      HashSet<std::string>* white_list) {

        if (mod->visited_mark) {
            return;
        }
        mod->visited_mark = true;

        // 1. handle all local exports
        for (auto& tuple : mod->GetExportManager().local_exports_name) {
            if (white_list && white_list->find(tuple.first) == white_list->end()) {
                continue;
            }
            arr.emplace_back(mod, tuple.first);
        }

        // 2. handle all external exports
        auto external_infos = mod->GetExportManager().CollectExternalInfos();
        for (auto& info : external_infos) {
            const auto& u8relative_path = info.relative_path;
            auto resolved_path = mod->resolved_map.find(u8relative_path);
            if (resolved_path == mod->resolved_map.end()) {
                WorkerError err {
                        mod->Path(),
                    format("resolve path failed: {}", u8relative_path)
                };
                worker_errors_.emplace_back(std::move(err));
                return;
            }

            auto iter = modules_table_.pathToModule.find(resolved_path->second);
            if (iter == modules_table_.pathToModule.end()) {
                WorkerError err {
                        mod->Path(),
                    format("module not found: {}", resolved_path->second)
                };
                worker_errors_.emplace_back(std::move(err));
                return;
            }
            if (info.is_export_all) {
                TraverseModulePushExportVars(arr, iter->second, nullptr);
            } else {
                HashSet<std::string> new_white_list;
                for (auto& item : info.names) {
                    new_white_list.insert(item.source_name);
                }
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

#ifdef JETPACK_SINGLE_THREAD
        unit();
#else
        thread_pool_->enqueue(std::move(unit));
#endif
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
            const auto& str = tuple.first;
            result[str] = tuple.second.module_name;
        }

        return result;
    }

    /**
     * 1. rename all first root variables in all modules
     * 2. replace all export declarations
     * 3. replace all import declarations
     * 4. generate final export declaration
     */
    void ModuleResolver::CodeGenAllModules(const CodeGenConfig& config, const std::string& out_path) {
        enqueued_files_count_ = 0;
        finished_files_count_ = 0;

        auto final_export_vars = GetAllExportVars();

        // distribute root level var name
        if (config.minify) {
            benchmark::BenchMarker benchMinify(benchmark::BENCH_MINIFY);
            RenameAllInnerScopes();
            benchMinify.Submit();
        }

        global_import_handler_.GenAst(name_generator);  // global import

        RenameAllRootLevelVariable();

        ClearAllVisitedMark();
        TraverseRenameAllImports(entry_module);

        DumpAllResult(config, final_export_vars, out_path);
    }

    // final stage
    void ModuleResolver::DumpAllResult(const CodeGenConfig& config, const Vec<std::tuple<Sp<ModuleFile>, std::string>>& final_export_vars, const std::string& outPath) {
        auto mappingCollector = std::make_shared<MappingCollector>();

        benchmark::BenchMarker codegenMarker(benchmark::BENCH_CODEGEN);
        auto sourcemapGenerator = std::make_shared<SourceMapGenerator>(shared_from_this(), outPath);

        // codegen all result begin
        CodeGen codegen(config, mappingCollector);
        sourcemapGenerator->AddCollector(mappingCollector);

        global_import_handler_.GenCode(codegen);

        if (has_common_js_.load()) {
            codegen.AddSnippet(COMMON_JS_CODE);
        }

        CodeGenModule(entry_module, codegen, *sourcemapGenerator);
        // codegen all result end

        if (!final_export_vars.empty()) {
            auto final_export = GenFinalExportDecl(final_export_vars);
            codegen.Traverse(*final_export);
        }

        const std::string finalResult = codegen.GetResult().content;
        codegenMarker.Submit();

        std::future<bool> srcFut;
        if (config.sourcemap) {
            sourcemapGenerator->Finalize();
            srcFut = DumpSourceMap(outPath, sourcemapGenerator);
        }

        benchmark::BenchMarker writeMarker(benchmark::BENCH_WRITING_IO);
        io::IOError err = io::WriteBufferToPath(outPath, finalResult.c_str(), finalResult.size());
        J_ASSERT(err == io::IOError::Ok);
        writeMarker.Submit();

        if (config.sourcemap) {
            if (unlikely(!srcFut.get())) {   // wait to finished
                std::cerr << "dump source map failed: " << outPath << std::endl;
            }
        }
    }

    void ModuleResolver::CodeGenModule(const Sp<ModuleFile> &root, CodeGen &codegen, SourceMapGenerator& sourcemap) {
        ClearAllVisitedMark();
        std::stack<Sp<ModuleFile>> stack;
        std::stack<Sp<ModuleFile>> spare_stack;

        stack.push(root);

        while (!stack.empty()) {
            auto top = stack.top();
            stack.pop();
            if (top->visited_mark) {
                continue;
            }

            top->visited_mark = true;
            spare_stack.push(top);

            for (auto& ref : top->ref_mods) {
                auto child = ref.lock();
                if (!child) {
                    continue;
                }
                stack.push(child);
            }
        }

        while (!spare_stack.empty()) {
            const auto& mod = spare_stack.top();

            if (mod->IsCommonJS()) {
                WrapModuleWithCommonJsTemplate(
                        mod->ast_context,
                        *mod->ast,
                        mod->cjs_call_name,
                        "__commonJS");
            }
            codegen.Traverse(*mod->ast);

            spare_stack.pop();
        }
    }

    // dump parallel
    std::future<bool> ModuleResolver::DumpSourceMap(std::string outPath, Sp<SourceMapGenerator> gen) {
        return thread_pool_->enqueue([outPath, gen]() -> bool {
            std::string pathStr = outPath + ".map";
            return gen->DumpFile(pathStr);
        });
    }

    void ModuleResolver::RenameAllInnerScopes() {
        ClearAllVisitedMark();
        RenamerCollection collection;
        collection.idLogger = id_logger_;

        for (auto& tuple : modules_table_.pathToModule) {
            EnqueueOne([this, mod = tuple.second, &collection] {
                mod->RenameInnerScopes(collection);
                FinishOne();
            });
        }
        std::unique_lock<std::mutex> lk(main_lock_);
        main_cv_.wait(lk, [this] {
            return finished_files_count_ >= enqueued_files_count_;
        });
        if (!worker_errors_.empty()) {
            WorkerErrorCollection col;
            col.errors = std::move(worker_errors_);
            throw std::move(col);
        }
        name_generator = MinifyNameGenerator::Merge(collection.content, id_logger_);
    }

    void ModuleResolver::RenameAllRootLevelVariable() {
        ClearAllVisitedMark();

        std::int32_t counter = 0;
        RenameAllRootLevelVariableTraverser(entry_module, counter);
    }

    void ModuleResolver::RenameAllRootLevelVariableTraverser(const std::shared_ptr<ModuleFile> &mf,
                                                             std::int32_t& counter) {
        if (mf->visited_mark) {
            return;
        }
        mf->visited_mark = true;

        for (auto& weak_child : mf->ref_mods) {
            auto child = weak_child.lock();
            RenameAllRootLevelVariableTraverser(child, counter);
        }

        // do your own work
        std::vector<Scope::PVar> variables;
        for (auto& tuple : mf->ast->scope->own_variables) {
            variables.push_back(tuple.second);
        }

        std::sort(std::begin(variables), std::end(variables), [] (const Scope::PVar& p1, const Scope::PVar& p2) {
            return p1->identifiers.size() > p2->identifiers.size();
        });

        // RenameSymbol() will change iterator, call it later
        std::vector<std::tuple<std::string, std::string>> rename_vec;

        // Distribute new name to root level variables
        for (auto& var : variables) {
            auto new_name_opt = name_generator->Next(var->name);

            if (new_name_opt.has_value()) {
                rename_vec.emplace_back(var->name, *new_name_opt);
            }
        }

        mf->ast->scope->BatchRenameSymbols(rename_vec);

        // replace exports to variable declaration
        ReplaceExports(mf);
    }

    /**
     * Turn import and export statements into variable declarations
     */
    void ModuleResolver::ReplaceExports(const Sp<ModuleFile>& mf) {
        std::vector<SyntaxNode*> new_body;

        for (auto& stmt : mf->ast->body) {
            switch (stmt->type) {

                /**
                 * export const name = 3;
                 *
                 * TO
                 *
                 * const name = 3;
                 *
                 * AND
                 *
                 * REMOVE export {}
                 */
                case SyntaxNodeType::ExportNamedDeclaration: {
                    auto export_named_decl = dynamic_cast<ExportNamedDeclaration*>(stmt);
                    if (export_named_decl->declaration.has_value()) {  // is local
                        new_body.push_back(*export_named_decl->declaration);
                    }
                    break;
                }

                /**
                 * export default 1 + 1;
                 *
                 * TO
                 *
                 * var default_0 = 1 + 1;
                 */
                case SyntaxNodeType::ExportDefaultDeclaration: {
                    auto export_default_decl = dynamic_cast<ExportDefaultDeclaration*>(stmt);
                    std::string new_name = "_default";
                    auto new_name_opt = name_generator->Next(new_name);
                    if (new_name_opt.has_value()) {
                        new_name = *new_name_opt;
                    }

                    mf->default_export_name = new_name;

                    if (export_default_decl->declaration->IsExpression()) {
                        auto exist_id = dynamic_cast<Expression*>(export_default_decl->declaration);

                        auto var_decl = module_ast_ctx_.Alloc<VariableDeclaration>();
                        var_decl->kind = VarKind::Var;

                        auto var_dector = module_ast_ctx_.Alloc<VariableDeclarator>(std::make_unique<Scope>(module_ast_ctx_));

                        auto new_id = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, new_name);
                        var_dector->id = new_id;
                        var_dector->init = { exist_id };

                        mf->ast->scope->CreateVariable(new_id, VarKind::Var);

                        var_decl->declarations.push_back(var_dector);

                        new_body.push_back(var_decl);
                    } else {
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
                                auto fun_decl = dynamic_cast<FunctionDeclaration*>(export_default_decl->declaration);

                                auto new_id = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, new_name);
                                mf->ast->scope->CreateVariable(new_id, VarKind::Var);
                                if (fun_decl->id.has_value()) {
                                    stmt = fun_decl;

                                    auto var_decl = module_ast_ctx_.Alloc<VariableDeclaration>();
                                    var_decl->kind = VarKind::Var;

                                    auto dector = module_ast_ctx_.Alloc<VariableDeclarator>(std::make_unique<Scope>(module_ast_ctx_));

                                    dector->id = new_id;


                                    auto right_id = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, (*fun_decl->id)->name);
                                    mf->ast->scope->CreateVariable(right_id, VarKind::Var);

                                    dector->init = { right_id };

                                    var_decl->declarations.push_back(dector);

                                    new_body.push_back(stmt);
                                    new_body.push_back(var_decl);
                                } else {
                                    fun_decl->id = { MakeId(module_ast_ctx_, SourceLocation::NoOrigin, new_name) };

                                    new_body.push_back(fun_decl);
                                }
                                break;
                            }

                            /**
                             * Similar to FunctionDeclaration
                             */
                            case SyntaxNodeType::ClassDeclaration: {
                                auto cls_decl = dynamic_cast<ClassDeclaration*>(export_default_decl->declaration);

                                auto new_id = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, new_name);
                                mf->ast->scope->CreateVariable(new_id, VarKind::Var);

                                if (cls_decl->id.has_value()) {
                                    stmt = cls_decl;

                                    auto var_decl = module_ast_ctx_.Alloc<VariableDeclaration>();
                                    var_decl->kind = VarKind::Var;

                                    auto dector = module_ast_ctx_.Alloc<VariableDeclarator>(std::make_unique<Scope>(module_ast_ctx_));

                                    dector->id = new_id;

                                    auto right_id = module_ast_ctx_.Alloc<Identifier>();
                                    right_id->name = (*cls_decl->id)->name;

                                    dector->init = { right_id };

                                    var_decl->declarations.push_back(dector);

                                    new_body.push_back(stmt);
                                    new_body.push_back(var_decl);
                                } else {
                                    cls_decl->id = { new_id };

                                    new_body.push_back(cls_decl);
                                }
                                break;
                            }

                            default:
                                break;

                        }
                    }

                    auto export_info = mf->GetExportManager().local_exports_name["default"];
                    if (export_info) {
                        export_info->local_name = new_name;
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

        mf->ast->body = std::move(new_body);
    }

    void ModuleResolver::TraverseRenameAllImports(const Sp<jetpack::ModuleFile> &mf) {
        if (mf->visited_mark) {
            return;
        }
        mf->visited_mark = true;

        for (auto& weak_ptr : mf->ref_mods) {
            auto ptr = weak_ptr.lock();
            TraverseRenameAllImports(ptr);
        }

        ReplaceImports(mf);
    }

    void ModuleResolver::ReplaceImports(const Sp<jetpack::ModuleFile> &mf) {
        std::vector<SyntaxNode*> new_body;

        for (auto& stmt : mf->ast->body) {
            switch (stmt->type) {
                case SyntaxNodeType::ImportDeclaration: {
                    auto import_decl = dynamic_cast<ImportDeclaration*>(stmt);
                    if (global_import_handler_.IsImportExternal(import_decl)) {  // remove from body
                        RenameExternalImports(mf, import_decl);
                        continue;
                    }

                    std::vector<VariableDeclaration*> result;
                    HandleImportDeclaration(mf, import_decl, result);
                    new_body.insert(std::end(new_body), std::begin(result), std::end(result));
                    continue;
                }

                default:
                    new_body.push_back(stmt);
                    break;

            }
        }

        mf->ast->body = std::move(new_body);
    }

    void ModuleResolver::RenameExternalImports(const Sp<jetpack::ModuleFile> &mf,
                                               jetpack::ImportDeclaration* import_decl) {
        std::vector<std::tuple<std::string, std::string>> renames;

        auto& source_path = import_decl->source->str_;
        auto& info = global_import_handler_.import_infos[source_path];

        for (auto& spec : import_decl->specifiers) {
            switch (spec->type) {
                case SyntaxNodeType::ImportDefaultSpecifier: {
                    auto import_default = spec->As<ImportDefaultSpecifier>();
                    auto& local_name = import_default->local->name;

                    if (local_name != info.default_local_name) {
                        renames.emplace_back(local_name, info.default_local_name);
                    }
                    break;
                }

                case SyntaxNodeType::ImportSpecifier: {
                    auto import_spec = spec->As<ImportSpecifier>();

                    if (auto iter = info.alias_map.find(import_spec->local->name); iter != info.alias_map.end()) {
                        renames.emplace_back(import_spec->local->name, iter->second);
                    }
                    break;
                }

                case SyntaxNodeType::ImportNamespaceSpecifier: {
                    auto import_ns = spec->As<ImportNamespaceSpecifier>();
                    auto& local_name = import_ns->local->name;

                    if (local_name != info.ns_import_name) {
                        renames.emplace_back(local_name, info.ns_import_name);
                    }
                    break;
                }

                default:
                    break;

            }

        }

        mf->ast->scope->BatchRenameSymbols(renames);
    }

    /**
     * import { a, b as c } from './mod1';
     *
     * TO
     *
     * const { a, b: c } = mod_1;
     */
    void ModuleResolver::HandleImportDeclaration(const Sp<ModuleFile>& mf,
                                                 jetpack::ImportDeclaration* import_decl,
                                                 std::vector<VariableDeclaration*>& result) {
        auto full_path_iter = mf->resolved_map.find(import_decl->source->str_);
        if (full_path_iter == mf->resolved_map.end()) {
            throw ModuleResolveException(
                    mf->Path(),
                    format("can not resolver path: {}", import_decl->source->str_));
        }

        auto target_module = modules_table_.pathToModule[full_path_iter->second];

        if (target_module == nullptr) {
            throw ModuleResolveException(mf->Path(), format("can not find module: {}", full_path_iter->second));
        }

//        auto target_mode_name = target_module->GetModuleVarName();

        if (import_decl->specifiers.empty()) {
            return;
        }

        if (import_decl->specifiers[0]->type == SyntaxNodeType::ImportNamespaceSpecifier) {
            auto import_ns = dynamic_cast<ImportNamespaceSpecifier*>(import_decl->specifiers[0]);

            auto decl = module_ast_ctx_.Alloc<VariableDeclaration>();
            decl->kind = VarKind::Var;

            auto declarator = module_ast_ctx_.Alloc<VariableDeclarator>(std::make_unique<Scope>(module_ast_ctx_));

            auto new_id = MakeId(module_ast_ctx_, import_ns->local->location, import_ns->local->name);

            // debug
//            std::cout << utils::To_UTF8(ast->scope->own_variables[import_ns->local->name].name) << std::endl;

            auto pvar = mf->ast->scope->own_variables[import_ns->local->name];
            if (pvar == nullptr) {
                return;
            }
            pvar->identifiers.push_back(new_id);

            declarator->id = new_id;  // try not to use old ast

            auto obj = module_ast_ctx_.Alloc<ObjectExpression>();

            {
                auto proto = module_ast_ctx_.Alloc<Property>();

                proto->key = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, "__proto__");
                proto->value = MakeNull(module_ast_ctx_);

                obj->properties.push_back(proto);
            }

            {
                const auto& relative_path = import_decl->source->str_;
                std::string absolute_path = mf->resolved_map[relative_path];

                auto ref_mod_iter = modules_table_.pathToModule.find(absolute_path);
                if (ref_mod_iter == modules_table_.pathToModule.end()) {
                    throw ModuleResolveException(mf->Path(), format("can not resolve path: {}", absolute_path));
                }
                auto& ref_mod = ref_mod_iter->second;

                auto& export_manager = ref_mod->GetExportManager();

                for (auto& tuple : export_manager.local_exports_name) {
                    auto prop = module_ast_ctx_.Alloc<Property>();

                    prop->kind = VarKind::Get;
                    prop->key = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, tuple.first);

                    auto fun = module_ast_ctx_.Alloc<FunctionExpression>();
                    auto block = module_ast_ctx_.Alloc<BlockStatement>();
                    auto ret_stmt = module_ast_ctx_.Alloc<ReturnStatement>();
                    ret_stmt->argument = { MakeId(module_ast_ctx_, SourceLocation::NoOrigin, tuple.second->local_name) };

                    block->body.push_back(ret_stmt);
                    fun->body = block;
                    prop->value = fun;
                    obj->properties.push_back(prop);
                }
            }

            declarator->init = { obj };

            decl->declarations.push_back(declarator);
            result.push_back(decl);
        } else {
            for (auto& spec : import_decl->specifiers) {
                std::string absolute_path;
                std::string target_export_name;
                std::string import_local_name;
                switch (spec->type) {
                    case SyntaxNodeType::ImportDefaultSpecifier: {
                        auto default_spec = dynamic_cast<ImportDefaultSpecifier*>(spec);
                        const auto& relative_path = import_decl->source->str_;
                        absolute_path = mf->resolved_map[relative_path];
                        target_export_name = "default";
                        import_local_name = default_spec->local->name;
                        break;
                    }

                    case SyntaxNodeType::ImportSpecifier: {
                        auto import_spec = dynamic_cast<ImportSpecifier*>(spec);
                        const auto& relative_path = import_decl->source->str_;
                        absolute_path = mf->resolved_map[relative_path];
                        target_export_name = import_spec->imported->name;
                        import_local_name = import_spec->local->name;
                        break;
                    }

                    default:
                        throw ModuleResolveException(mf->Path(), "unknown specifier type");

                }

                auto ref_mod_iter = modules_table_.pathToModule.find(absolute_path);
                if (ref_mod_iter == modules_table_.pathToModule.end()) {
                    throw ModuleResolveException(mf->Path(), format("can not resolve path: {}", absolute_path));
                }
                auto& ref_mod = ref_mod_iter->second;

                std::set<int32_t> visited_mods;
                auto local_export_opt = FindLocalExportByPath(absolute_path, target_export_name, visited_mods);

                if (!local_export_opt.has_value()) {
                    throw ModuleResolveException(
                            mf->Path(),
                        format("can not find export variable '{}' from {}", target_export_name, absolute_path)
                    );
                }

                std::vector<std::tuple<std::string, std::string>> changeset;
                changeset.emplace_back(import_local_name, (*local_export_opt)->local_name);
                if (!mf->ast->scope->BatchRenameSymbols(changeset)) {
                    throw ModuleResolveException(
                            mf->Path(),
                        format("rename symbol failed: {}", import_local_name)
                    );
                }
            }
        }
    }

    std::optional<Sp<LocalExportInfo>>
    ModuleResolver::FindLocalExportByPath(const std::string &path,
                                          const std::string& export_name,
                                          std::set<int32_t>& visited) {
        auto modIter = modules_table_.pathToModule.find(path);
        if (modIter == modules_table_.pathToModule.end()) {
            return std::nullopt;
        }
        auto& mod = modIter->second;

        if (visited.find(mod->id()) != visited.end()) {
            return std::nullopt;
        }
        visited.insert(mod->id());

        auto local_iter = mod->GetExportManager().local_exports_name.find(export_name);
        if (local_iter != mod->GetExportManager().local_exports_name.end()) {  // found
            return { local_iter->second };
        }

        // not in local export
        for (auto& tuple : mod->GetExportManager().external_exports_map) {
            const auto& relative_path = tuple.second.relative_path;
            auto absolute_path = mod->resolved_map[relative_path];

            if (tuple.second.is_export_all) {
                auto tmp_result = FindLocalExportByPath(absolute_path, export_name, visited);
                if (tmp_result.has_value()) {  // the variable you find is in this opt
                    return tmp_result;
                }
            } else {
                for (auto& alias : tuple.second.names) {
                    if (alias.export_name == export_name) {  // eventually find you!
                        return FindLocalExportByPath(absolute_path, alias.source_name, visited);
                    }
                }
            }
        }

        return std::nullopt;
    }

    Sp<ExportNamedDeclaration> ModuleResolver::GenFinalExportDecl(const std::vector<std::tuple<Sp<ModuleFile>, std::string>>& export_names) {
        auto result = std::make_shared<ExportNamedDeclaration>();

        for (auto& tuple : modules_table_.pathToModule) {
            tuple.second->visited_mark = false;
        }

        for (auto& tuple : export_names) {
            const std::string& export_name = std::get<1>(tuple);
            const Sp<ModuleFile>& mf = std::get<0>(tuple);

            auto iter = mf->GetExportManager().local_exports_name.find(export_name);
            if (iter == mf->GetExportManager().local_exports_name.end()) {
                WorkerError err {
                        mf->Path(),
                        format("symbol not found failed: {}", export_name)
                };
                worker_errors_.emplace_back(std::move(err));
                break;
            }
            auto info = iter->second;

            auto spec = module_ast_ctx_.Alloc<ExportSpecifier>();
            spec->local = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, info->local_name);
            spec->exported = MakeId(module_ast_ctx_, SourceLocation::NoOrigin, export_name);
            result->specifiers.push_back(spec);
        }

        return result;
    }

    std::pair<Sp<ModuleProvider>, std::string> ModuleResolver::FindProviderByPath(const Sp<ModuleFile>& parent, const std::string &path) {
        for (auto iter = providers_.rbegin(); iter != providers_.rend(); iter++) {
            auto matchResult = (*iter)->Match(*parent, path);
            if (matchResult.has_value()) {
                return { *iter, *matchResult };
            }
        }
        return { nullptr, "" };
    }

    std::optional<std::string> ModuleResolver::FindPathOfPackageJson(const std::string &entryPath) {
        Path path(entryPath);
        path.slices.pop_back();

        while (likely(!path.slices.empty())) {
            path.slices.emplace_back(PackageJsonName);

            auto full_path = path.ToString();
            if (likely(io::IsFileExist(full_path))) {
                path.slices.pop_back();
                return { path.ToString() };
            }

            path.slices.pop_back();
            path.slices.pop_back();
        }

        return std::nullopt;
    }

}
