//
// Created by Duzhong Chen on 2020/7/13.
//

#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <memory>
#include <tsl/ordered_map.h>
#include "string/UString.h"

namespace jetpack {

    template<class Key, class T, class Ignore, class Allocator,
            class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
            class AllocatorPair = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
            class ValueTypeContainer = std::vector<std::pair<Key, T>, AllocatorPair>>
    using ordered_map = tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;

    using json = nlohmann::basic_json<ordered_map>;

    class ModuleResolver;

    class SourceMapGenerator {
    public:
        static void GenerateVLQStr(std::string& ss, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static void IntToVLQ(std::string& ss, int code);

        SourceMapGenerator() = delete;

        SourceMapGenerator(const std::shared_ptr<ModuleResolver>& resolver, // nullable
                           const std::string& filename);

        void SetSourceRoot(const std::string& sr);

        void AddSource(const std::string& src);

        bool AddLocation(const UString& name, int after_col, int fileId, int before_line, int before_col);

        inline void EndLine() {
            mappings.push_back(';');
        }

        void Finalize();

        std::string ToPrettyString();

    private:
        std::string mappings;
        json result;
        int32_t src_counter_ = 0;

        int32_t GetIdOfName(const UString& name);

        int32_t GetFilenameIndexByModuleId(int32_t moduleId);

        HashMap<UString, int32_t> names_map_;
        HashMap<int32_t, int32_t> module_id_to_index_;

        std::shared_ptr<ModuleResolver> module_resolver_;

    };

}
