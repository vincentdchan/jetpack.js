//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <ThreadPool.h>
#include <filesystem.hpp>
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
#include "utils/WaitGroup.h"

namespace jetpack {

    struct WorkerErrors {
    public:
        WorkerErrors() = default;

        inline void add(const WorkerError& err) {
            std::lock_guard<std::mutex> guard(m_);
            errors_.push_back(err);
        }

        bool print();

        void clear();

        void throw_collection_if_not_empty();

    private:
        Vec<WorkerError> errors_;
        std::mutex m_;

    };

    class ModuleCompositor;

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

    using ExportVariable = std::tuple<Sp<ModuleFile>, std::string>;

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

        void CodeGenAllModules(const CodeGenConfig& config, const std::string& out_path);

        void RenameAllInnerScopes();

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

        std::optional<ghc::filesystem::path> FindPathOfPackageJson(const std::string& entry_path);

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
                uint8_t* visited_marks,
                HashSet<std::string>* white_list);

        void RenameAllRootLevelVariableTraverser(const Sp<ModuleFile>& mf,
                                                 uint8_t* visited_marks,
                                                 std::int32_t& counter);

        Sp<ModuleFile> HandleNewLocationAdded(const parser::Config& config,
                                    const Sp<ModuleFile>& mf,
                                    LocationAddOptions flags,
                                    const std::string& path);

        void DumpAllResult(const CodeGenConfig& config,
                           Slice<const ExportVariable> final_export_vars,
                           const std::string& outPath);

        void CodeGenGlobalImport(ModuleCompositor& mc);

        void CodeGenFinalExport(
                ModuleCompositor& mc,
                Slice<const ExportVariable> final_export_vars);

        void ConcatModules(const Sp<ModuleFile>& root, ModuleCompositor& mc);

    public:
        void ReplaceExports(const Sp<ModuleFile>& mf);

        inline void SetEscapeFile(bool v) {
            escape_file_ = v;
        }

    private:
        void TraverseRenameAllImports(const Sp<ModuleFile>& mf, uint8_t* visited_marks);

        void ReplaceImports(const Sp<ModuleFile>& mf);

        void RenameExternalImports(const Sp<ModuleFile>& mf, ImportDeclaration* import_decl);

        void HandleImportDeclaration(const Sp<ModuleFile>& mf,
                                     ImportDeclaration* import_decl,
                                     std::vector<VariableDeclaration*>& result);

        bool IsExternalImportModulePath(const std::string& path);

        std::optional<Sp<LocalExportInfo>>
        FindLocalExportByPath(const std::string& path, const std::string& export_name, std::set<int32_t>& visited);

        Sp<ExportNamedDeclaration> GenFinalExportDecl(Slice<const ExportVariable> export_names);

        // return nullable
        std::pair<Sp<ModuleProvider>, ghc::filesystem::path> FindProviderByPath(const Sp<ModuleFile>& parent, const std::string& path);

        GlobalImportHandler global_import_handler_;

        AstContext module_ast_ctx_;

        Sp<UniqueNameGenerator> name_generator;

        Sp<UnresolvedNameCollector> id_logger_;

        Sp<ModuleFile> entry_module;

        std::unique_ptr<ThreadPool> thread_pool_;

        Vec<Sp<ModuleProvider>> providers_;

        WorkerErrors worker_errors_;

        std::atomic<bool> has_common_js_{ false };

        WaitGroup parsing_group_;
        std::atomic<uint32_t> total_files_{0};

        bool trace_file = true;
        bool escape_file_ = false;

    };

}
