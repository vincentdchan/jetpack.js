//
// Created by Duzhong Chen on 2020/7/13.
//

#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include "string/UString.h"
#include "MappingCollector.h"

namespace jetpack {
    class ModuleResolver;

    class SourceMapGenerator {
    public:
        static void GenerateVLQStr(std::string& ss, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static void IntToVLQ(std::string& ss, int code);
        static int VLQToInt(const std::string& str);

        SourceMapGenerator() = delete;

        SourceMapGenerator(const std::shared_ptr<ModuleResolver>& resolver, // nullable
                           const std::string& filename);

        void SetSourceRoot(const std::string& sr);

        void AddSource(const std::string& src);

        inline void EndLine() {
            mappings.push_back(';');
        }

        void Finalize();

        std::string ToPrettyString();

        bool DumpFile(const std::string& path, bool pretty = false);

        inline void AddCollector(const Sp<MappingCollector> collector) {
            collectors_.push_back(collector);
        }

    private:
        std::string mappings;
        json result;
        int32_t src_counter_ = 0;

        void FinalizeCollector(const MappingCollector& collector);

        bool AddLocation(const UString& name, int after_col, int fileId, int before_line, int before_col);

        int32_t GetIdOfName(const UString& name);

        int32_t GetFilenameIndexByModuleId(int32_t moduleId);

        HashMap<UString, int32_t> names_map_;
        HashMap<int32_t, int32_t> module_id_to_index_;

        std::shared_ptr<ModuleResolver> module_resolver_;

        Vec<Sp<MappingCollector>> collectors_;

    };

}
