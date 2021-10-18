//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <ThreadPool.h>
#include <boost/thread/synchronized_value.hpp>
#include <condition_variable>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <functional>
#include <fstream>
#include <mutex>

#include "ModuleProvider.h"
#include "ModuleFile.h"
#include "ModulesTable.h"
#include "GlobalImportHandler.h"
#include "WorkerError.h"
#include "sourcemap/SourceMapGenerator.h"
#include "utils/JetFlags.h"

namespace jetpack {

    class ModuleResolveException : std::exception {
    public:
        std::string file_path;
        std::string error_content;

        ModuleResolveException(const std::string& path, const std::string& content);

        virtual void PrintToStdErr();

    };

    class WorkerErrorCollection : public ModuleResolveException {
    public:
        std::vector<WorkerError> errors;

        WorkerErrorCollection(): ModuleResolveException("", "") {}

        void PrintToStdErr() override;

    };

    /**
     * Parsing source file in different threads.
     * The error messages would be collected.
     */
    class ModuleResolver : public std::enable_shared_from_this<ModuleResolver> {
    public:
        enum LocationAddOption {
            LocationImported = 0x1,
            LocationExported = 0x2,
            LocationIsCommonJS = 0x4,
        };

        JET_DECLARE_FLAGS(LocationAddOptions, LocationAddOption)

        ModuleResolver() {
            name_generator = ReadableNameGenerator::Make();
            id_logger_ = std::make_shared<UnresolvedNameCollector>();
        }

        void BeginFromEntry(const parser::Config& config,
                            const std::string& originPath,
                            const std::string& basePathOverride="");

        void BeginFromEntryString(const parser::Config& config,
                                  const std::string& str);

        void ParseFileFromPath(const Sp<ModuleProvider>& rootProvider,
                               const parser::Config& config,
                               const std::string& path);

        void ParseFile(const parser::Config& config,
                       Sp<ModuleFile>);

        inline void SetTraceFile(bool val) {
            trace_file = val;
        }

        inline bool GetTraceFile() const {
            return trace_file;
        }

        void PrintStatistic();

        void PrintErrors(const Vec<WorkerError>& errors);

        void CodeGenAllModules(JetpackFlags flags, const std::string& out_path);

        void RenameAllInnerScopes();

        inline void ClearAllVisitedMark() {
            for (auto& tuple : modules_table_.path_to_module) {
                tuple.second->visited_mark = false;
            }
        }

        inline void SetNameGenerator(std::shared_ptr<UniqueNameGenerator> generator) {
            name_generator = std::move(generator);
        }

        inline int32_t ModCount() const {
            return modules_table_.ModCount();
        }

        // nullable!
        inline Sp<ModuleFile> findModuleById(int32_t id) {
            return modules_table_.FindModuleById(id);
        }

        std::optional<std::string> FindPathOfPackageJson(const std::string& entry_path);

        ModulesTable modules_table_;

        json GetImportStat();
        Vec<std::tuple<Sp<ModuleFile>, std::string>> GetAllExportVars();

        void RenameAllRootLevelVariable();

        inline Sp<ModuleFile> GetEntryModule() {
            return entry_module;
        }

        inline ThreadPool& InternalThreadPool() {
            return *thread_pool_;
        }

    private:
        void pBeginFromEntry(const Sp<ModuleProvider>& rootProvider, const parser::Config& config, const std::string& resolvedPath);

        void TraverseModulePushExportVars(
                std::vector<std::tuple<Sp<ModuleFile>, std::string>>& arr,
                const Sp<ModuleFile>&,
                HashSet<std::string>* white_list);

        void RenameAllRootLevelVariableTraverser(const Sp<ModuleFile>& mf,
                                                 std::int32_t& counter);

        Sp<ModuleFile> HandleNewLocationAdded(const parser::Config& config,
                                    const Sp<ModuleFile>& mf,
                                    LocationAddOptions flags,
                                    const std::string& path);

        void DumpAllResult(JetpackFlags flags,
                           const Vec<std::tuple<Sp<ModuleFile>, std::string>>& final_export_vars,
                           const std::string& outPath);


        void CodeGenModule(const Sp<ModuleFile>& mod, CodeGen& codegen, SourceMapGenerator& sourcemap);

        std::future<bool> DumpSourceMap(std::string outPath, Sp<SourceMapGenerator> gen);

    public:
        void ReplaceExports(const Sp<ModuleFile>& mf);

    private:
        void EnqueueOne(std::function<void()> unit);
        void FinishOne();

        void TraverseRenameAllImports(const Sp<ModuleFile>& mf);

        void ReplaceImports(const Sp<ModuleFile>& mf);

        void RenameExternalImports(const Sp<ModuleFile>& mf, ImportDeclaration* import_decl);

        void HandleImportDeclaration(const Sp<ModuleFile>& mf,
                                     ImportDeclaration* import_decl,
                                     std::vector<VariableDeclaration*>& result);

        bool IsExternalImportModulePath(const std::string& path);

        std::optional<Sp<LocalExportInfo>>
        FindLocalExportByPath(const std::string& path, const std::string& export_name, std::set<int32_t>& visited);

        Sp<ExportNamedDeclaration> GenFinalExportDecl(const std::vector<std::tuple<Sp<ModuleFile>, std::string>>&);

        // return nullable
        std::pair<Sp<ModuleProvider>, std::string> FindProviderByPath(const Sp<ModuleFile>& parent, const std::string& path);

        GlobalImportHandler global_import_handler_;

        AstContext module_ast_ctx_;

        Sp<UniqueNameGenerator> name_generator;

        Sp<UnresolvedNameCollector> id_logger_;

        Sp<ModuleFile> entry_module;

        std::unique_ptr<ThreadPool> thread_pool_;

        Vec<Sp<ModuleProvider>> providers_;

        boost::synchronized_value<Vec<WorkerError>> worker_errors_;

        int32_t enqueued_files_count_ = 0;
        int32_t finished_files_count_ = 0;

        std::atomic<bool> has_common_js_{ false };

        std::mutex main_lock_;
        std::condition_variable main_cv_;

        bool trace_file = true;

    };

}
