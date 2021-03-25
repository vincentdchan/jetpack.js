//
// Created by Duzhong Chen on 2020/7/13.
//

#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <memory>
#include <robin_hood.h>
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
        static bool GenerateVLQStr(std::stringstream& ss, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static bool IntToVLQ(std::stringstream& ss, int code);
        static bool IntToBase64(int, char& ch);

        std::stringstream ss;

        SourceMapGenerator();

        SourceMapGenerator(const std::shared_ptr<ModuleResolver>& resolver,
                           const std::string& filename);

        void SetSourceRoot(const std::string& sr);

        void AddSource(const std::string& src);

        bool AddLocation(const UString& name, int after_col, int file_index, int before_line, int before_col);

        inline void EndLine() {
            mappings.push_back(';');
        }

        void Finalize();

    private:
        std::string mappings;
        json result;

        int32_t GetIdOfName(const UString& name);

        robin_hood::unordered_map<UString, int32_t> names_map_;

        std::shared_ptr<ModuleResolver> module_resolver_;

    };

}
