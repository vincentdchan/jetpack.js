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
#include <functional>
#include <robin_hood.h>
#include <parser/Parser.hpp>
#include <ghc/filesystem.hpp>

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

        std::string path;

        std::weak_ptr<ModuleResolver> module_resolver;

        Sp<Module> ast;

        robin_hood::unordered_map<std::string, std::string> resolved_map;

        bool visited_mark = false;

        std::string codegen_result;

        void CodeGenFromAst();

    };

    /**
     * Parsing source file in different threads.
     * The error messages would be collected.
     */
    class ModuleResolver : public std::enable_shared_from_this<ModuleResolver> {
    public:
        static const char PATH_DIV = '/';
        static std::u16string ReadFileStream(const std::string& filename);

        ModuleResolver() = default;

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

    private:
        void EnqueueOne(std::function<void()> unit);
        void FinishOne();

        json GetImportStat();
        json GetAllExportVars();

        void TraverseModulePushExportVars(json& arr,
                const Sp<ModuleFile>&, std::unordered_set<std::string>* white_list);

        std::mutex map_mutex_;
        robin_hood::unordered_map<std::string, Sp<ModuleFile>> modules_map_;

        Sp<ModuleFile> entry_module;

        std::unique_ptr<ThreadPool> thread_pool_;

        std::vector<WorkerError> worker_errors_;
        std::mutex error_mutex_;

        std::int32_t enqueued_files_count_ = 0;
        std::int32_t finished_files_count_ = 0;

        std::mutex main_lock_;
        std::condition_variable main_cv_;

        bool trace_file = true;

    };

}
