//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#include <ThreadPool.h>
#include <memory>
#include <string>
#include <robin_hood.h>
#include <parser/Parser.hpp>

namespace rocket_bundle {

    class ModuleResolver;

    class ModuleFile {
    public:

        std::string path;

        std::weak_ptr<ModuleResolver> module_resolver;

        Sp<Module> ast;

    };

    class ModuleResolver : public std::enable_shared_from_this<ModuleResolver> {
    public:
        ModuleResolver() = default;

        void BeginFromEntry(std::string path);

        void ParseFileFromPath(const std::string& path);

        void ParseFile(Sp<ModuleFile>);

        inline void SetTraceFile(bool val) {
            trace_file = val;
        }

        inline bool GetTraceFile() const {
            return trace_file;
        }

        void PrintStatistic();

    private:
        std::mutex map_mutex_;
        robin_hood::unordered_map<std::string, Sp<ModuleFile>> modules_map_;

        Sp<ModuleFile> entry_module;

        std::unique_ptr<ThreadPool> thread_pool_;

        bool trace_file = true;

    };

}
