//
// Created by Duzhong Chen on 2020/7/13.
//

#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <ThreadPool.h>
#include "Slice.h"
#include "utils/string/UString.h"
#include "MappingCollector.h"
#include "ModuleFile.h"

namespace jetpack {
    class ModuleResolver;

    /**
     * unify all mappings from different modules
     */
    class SourceMapGenerator {
    public:
        static void GenerateVLQStr(std::string& ss, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static void IntToVLQ(std::string& ss, int code);
        static int VLQToInt(const char* str, const char*& next);

        SourceMapGenerator() = delete;

        SourceMapGenerator(const std::shared_ptr<ModuleResolver>& resolver, // nullable
                           const std::string& filename);

        void AddSource(const Sp<ModuleFile>& src);

        inline void EndLine() {
            mappings.push_back(';');
        }

        /**
         * Unify all collectors together
         */
        void Finalize(Slice<const MappingItem> mapping_items);

        std::string ToPrettyString();

        bool DumpFile(const std::string& path, bool pretty = false);

        inline void AddCollector(const Sp<MappingCollector>& collector) {
            collectors_.push_back(collector);
        }

    private:
        std::stringstream ss;
        std::string mappings;
        int32_t src_counter_ = 0;
        int32_t line_counter_ = 1;

        int32_t l_after_col_ = 0;
        int32_t l_file_index_ = 0;
        int32_t l_before_line_ = 1;
        int32_t l_before_col_ = 0;

        void AddEnoughLines(int32_t target_line);

        void FinalizeMapping(Slice<const MappingItem> items);

        void FinalizeSources();

        void FinalizeSourcesContent();

        bool AddLocation(const std::string& name, int after_col, int fileId, int before_line, int before_col);

        int32_t GetFilenameIndexByModuleId(int32_t moduleId);

        HashMap<int32_t, int32_t>   module_id_to_index_;
        std::vector<Sp<ModuleFile>> sources_;

        std::shared_ptr<ModuleResolver> module_resolver_;

        Vec<Sp<MappingCollector>> collectors_;

    };

}
