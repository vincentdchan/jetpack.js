//
// Created by Duzhong Chen on 2020/3/20.
//

#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <parser/ParserCommon.h>
#include <codegen/CodeGen.h>
#include <fstream>
#include <iostream>

#include "Path.h"
#include "./ModuleResolver.h"
#include "./Utils.h"
#include "./Error.h"

namespace rocket_bundle {
    using parser::ParserContext;
    using parser::Parser;

    void ModuleFile::CodeGenFromAst() {
        std::stringstream ss;
        CodeGen::Config config;
        CodeGen codegen(config, ss);
        codegen.Traverse(ast);
        codegen_result = ss.str();
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
                new_mf->module_resolver = shared_from_this();
                new_mf->path = std::move(source_path);
                modules_map_[new_mf->path] = new_mf;
            }

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

        std::cout << result.dump(2) << std::endl;
    }

    json ModuleResolver::GetAllExportVars() {
        json result = json::array();

        TraverseModulePushExportVars(result, entry_module, nullptr);

        return result;
    }

    void ModuleResolver::TraverseModulePushExportVars(
            rocket_bundle::json &arr, const Sp<rocket_bundle::ModuleFile>& mod,
            std::unordered_set<std::string>* white_list) {

        if (mod->visited_mark) {
            return;
        }
        mod->visited_mark = true;

        for (auto& local_name : mod->ast->scope->export_manager.local_export_name) {
            auto u8name = utils::To_UTF8(local_name);
            if (white_list && white_list->find(u8name) == white_list->end()) {
                continue;
            }
            arr.push_back(std::move(u8name));
        }

        for (auto& item : mod->ast->scope->export_manager.external_export_vars) {
            auto u8relative_path = utils::To_UTF8(item.first);
            auto resolved_path = mod->resolved_map.find(u8relative_path);
            if (resolved_path == mod->resolved_map.end()) {
                throw std::runtime_error("resolve path failed: " + u8relative_path);
            }

            auto iter = modules_map_.find(resolved_path->second);
            if (iter == modules_map_.end()) {
                throw std::runtime_error("module not found: " + resolved_path->second);
            }
            if (item.second.is_export_all) {
                TraverseModulePushExportVars(arr, iter->second, nullptr);
            } else {
                std::unordered_set<std::string> new_white_list;
                for (auto& name : item.second.export_names) {
                    new_white_list.insert(utils::To_UTF8(name));
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

        for (auto& tuple : modules_map_) {
            EnqueueOne([this, mod = tuple.second] {
                mod->CodeGenFromAst();
                FinishOne();
            });
        }

        std::unique_lock<std::mutex> lk(main_lock_);
        main_cv_.wait(lk, [this] {
            return finished_files_count_ >= enqueued_files_count_;
        });

        std::ofstream out(out_path, std::ios::out);

        for (auto& tuple : modules_map_) {
            out << tuple.second->codegen_result << std::endl;
        }

        out.close();
    }

}
