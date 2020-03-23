//
// Created by Duzhong Chen on 2020/3/20.
//

#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <ghc/filesystem.hpp>
#include <artery.h>
#include <parser/ParserCommon.h>
#include <iostream>

#include "Path.h"
#include "./ModuleResolver.h"
#include "./Utils.h"
#include "./Error.h"

namespace rocket_bundle {
    using parser::ParserContext;
    using parser::Parser;

    void ModuleResolver::BeginFromEntry(std::string base_path, std::string target_path) {
        std::string path;
        if (target_path.empty()) {
            return;
        } else if (target_path[0] == PATH_DIV) {
            path = target_path;
        } else {
            Path path(base_path);
            path.Join(target_path);
        }

        auto thread_pool_size = std::thread::hardware_concurrency();
        thread_pool_ = std::make_unique<ThreadPool>(thread_pool_size);

        EnqueueOne();
        thread_pool_->enqueue([this, &path] {
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
        (*src) = Artery::ReadFileStream(mf->path);
        auto ctx = std::make_shared<ParserContext>(src, config);
        Parser parser(ctx);

        parser.OnNewImportLocationAdded([this, &mf] (const UString& path) {
            if (!trace_file) return;

            Path module_path(mf->path);
            module_path.Join(utils::To_UTF8(path));

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

            std::shared_ptr<ModuleFile> mf;
            {
                std::lock_guard<std::mutex> guard(map_mutex_);
                if (modules_map_.find(source_path) != modules_map_.end()) return;  // exists

                mf = std::make_shared<ModuleFile>();
                mf->module_resolver = shared_from_this();
                mf->path = std::move(source_path);
                modules_map_[mf->path] = mf;
            }

            EnqueueOne();
            thread_pool_->enqueue([this, mf] {
                try {
                    ParseFile(mf);
                } catch (parser::ParseError& ex) {
                    std::lock_guard<std::mutex> guard(error_mutex_);
                    WorkerError err { mf->path, ex.ErrorMessage() };
                    worker_errors_.emplace_back(std::move(err));
                } catch (std::exception& ex) {
                    std::lock_guard<std::mutex> guard(error_mutex_);
                    WorkerError err { mf->path, ex.what() };
                    worker_errors_.emplace_back(std::move(err));
                }
                FinishOne();
            });
        });

        mf->ast = parser.ParseModule();
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

        std::cout << result.dump(2) << std::endl;
    }

    void ModuleResolver::PrintErrors() {
        for (auto& error : worker_errors_) {
            std::cerr << "File: " << error.file_path << std::endl;
            std::cerr << "Error: " << error.error_content << std::endl;
        }
    }

    void ModuleResolver::EnqueueOne() {
        std::lock_guard<std::mutex> lk(main_lock_);
        enqueued_files_count_++;
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

}
