//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include <ThreadPool.h>
#include <condition_variable>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <functional>
#include <robin_hood.h>
#include <fstream>
#include <parser/Parser.hpp>

namespace rocket_bundle {

    template<class Key, class T, class Ignore, class Allocator,
        class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
        class AllocatorPair = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
        class ValueTypeContainer = std::vector<std::pair<Key, T>, AllocatorPair>>
    using ordered_map = tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;
    using json = nlohmann::basic_json<ordered_map>;

    class ModuleResolver;

    class WorkerError {
    public:
        std::string file_path;
        std::string error_content;

    };

    class ModuleFile {
    public:
        /**
         * Unique id in module resolver
         */
        std::int32_t id = 0;

        /**
         * Abosolute path
         */
        std::string path;

        UString default_export_name;

        std::weak_ptr<ModuleResolver> module_resolver;

        Sp<Module> ast;

        /**
         * relative path -> absolute path
         */
        robin_hood::unordered_map<std::string, std::string> resolved_map;

        /**
         * Temp for parallel codegen
         */
        std::string codegen_result;

        /**
         * For Postorder traversal
         */
        bool visited_mark = false;

        /**
         * For Postorder traversal
         */
        std::vector<std::weak_ptr<ModuleFile>> ref_mods;

        void CodeGenFromAst();

        UString GetModuleVarName();

        inline ExportManager& GetExportManager() {
            return ast->scope->export_manager;
        }

    };

    /**
     * Parsing source file in different threads.
     * The error messages would be collected.
     */
    class ModuleResolver : public std::enable_shared_from_this<ModuleResolver> {
    public:
        static std::u16string ReadFileStream(const std::string& filename);

        ModuleResolver() : mod_counter_(0) {
        }

        void BeginFromEntry(std::string base_path, std::string origin_path);

        void ParseFileFromPath(const std::string& path);

        void ParseFile(Sp<ModuleFile>);

        inline void SetTraceFile(bool val) {
            trace_file = val;
        }

        inline bool GetTraceFile() const {
            return trace_file;
        }

        void PrintStatistic();

        void PrintErrors();

        void CodeGenAllModules(const std::string& out_path);

        void MergeModules(const Sp<ModuleFile>& mf, std::ofstream& out_path);

        inline std::int32_t NextNameId() {
            std::lock_guard<std::mutex> guard(main_lock_);
            return name_counter_++;
        }

        inline void ClearAllVisitedMark() {
            for (auto& tuple : modules_map_) {
                tuple.second->visited_mark = false;
            }
        }

        robin_hood::unordered_map<std::string, Sp<ModuleFile>> modules_map_;

    private:
        void EnqueueOne(std::function<void()> unit);
        void FinishOne();

        json GetImportStat();
        std::vector<std::tuple<Sp<ModuleFile>, UString>> GetAllExportVars();

        void TraverseModulePushExportVars(
                std::vector<std::tuple<Sp<ModuleFile>, UString>>& arr,
                const Sp<ModuleFile>&,
                std::unordered_set<UString>* white_list);

        void RenameAllRootLevelVariable(std::unordered_set<UString>& used_name);
        void RenameAllRootLevelVariableTraverser(const Sp<ModuleFile>& mf,
                                                 std::int32_t& counter,
                                                 std::unordered_set<UString>& used_name);

    public:
        void ReplaceExports(const Sp<ModuleFile>& mf);

    private:
        bool TraverseRenameAllImports(const Sp<ModuleFile>& mf);

        bool ReplaceImports(const Sp<ModuleFile>& mf);

        bool HandleImportDeclaration(const Sp<ModuleFile>& mf,
                                     Sp<ImportDeclaration>& import_decl,
                                     std::vector<Sp<VariableDeclaration>>& result);

        std::optional<Sp<LocalExportInfo>>
        FindLocalExportByPath(const std::string& path, const UString& export_name, std::set<std::int32_t>& visited);

        Sp<ExportNamedDeclaration> GenFinalExportDecl(const std::vector<std::tuple<Sp<ModuleFile>, UString>>&);

        std::mutex map_mutex_;

        Sp<ModuleFile> entry_module;

        std::unique_ptr<ThreadPool> thread_pool_;

        std::vector<WorkerError> worker_errors_;
        std::mutex error_mutex_;

        std::int32_t enqueued_files_count_ = 0;
        std::int32_t finished_files_count_ = 0;

        std::int32_t name_counter_ = 0;

        std::mutex main_lock_;
        std::condition_variable main_cv_;

        std::atomic<std::int32_t> mod_counter_;

        bool trace_file = true;

    };

}
