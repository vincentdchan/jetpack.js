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
#include "utils/io/FileIO.h"
#include "MappingCollector.h"
#include "ModuleFile.h"

namespace jetpack {
    class ModuleResolver;

    /**
     * unify all mappings from different modules
     */
    class SourceMapGenerator {
    public:
        enum class LastWriteType {
            None,
            Item,
            LineBreak,
        };

        static void GenerateVLQStr(io::Writer& writer, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static void IntToVLQ(io::Writer& writer, int code);
        static int VLQToInt(const char* str, const char*& next);

        SourceMapGenerator() = delete;

        SourceMapGenerator(const std::shared_ptr<ModuleResolver>& resolver, // nullable
                           io::Writer& writer,
                           const std::string& filename);

        void EndLine();

        /**
         * Unify all collectors together
         */
        void Finalize(Slice<const MappingItem> mapping_items);

    private:
        LastWriteType last_write_ = LastWriteType::None;
        io::Writer& writer_;
//        std::string mappings_;
//        int32_t src_counter_ = 0;
        int32_t line_counter_ = 1;

        int32_t l_after_col_ = 0;
        int32_t l_file_index_ = 0;
        int32_t l_before_line_ = 1;
        int32_t l_before_col_ = 0;

        void AddEnoughLines(int32_t target_line);

        void FinalizeMapping(Slice<const MappingItem> items);

        void FinalizeSources();

        void FinalizeSourcesContent();

        bool AddLocation(const std::string& name, int after_col, int file_id, int before_line, int before_col);

//        int32_t GetFilenameIndexByModuleId(int32_t module_id);

        std::shared_ptr<ModuleResolver> module_resolver_;

    };

}
