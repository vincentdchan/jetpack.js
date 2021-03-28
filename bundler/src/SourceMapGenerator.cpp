//
// Created by Duzhong Chen on 2020/7/13.
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "SourceMapGenerator.h"
#include "ModuleResolver.h"

#define IntToVLQBufferSize 8

namespace jetpack {
    using std::stringstream;
    using std::memset;

    static char Base64EncodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                         'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                         'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                         'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                         'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                         'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                         'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                         '4', '5', '6', '7', '8', '9', '+', '/'};

    inline char IntToBase64(int code) {
        J_ASSERT(code >= 0 && code <= 0b111111);
        return Base64EncodingTable[code];
    }

    void SourceMapGenerator::IntToVLQ(std::string& ss, int code) {
        int buffer[IntToVLQBufferSize];

        J_ASSERT(code >= 0);
        int s1 = code << 1;
        int counter = 0;
        if (unlikely(s1 > 0b11111)) {
            memset(buffer, 0, IntToVLQBufferSize * sizeof(int));
            while (s1 > 0b11111) {
                buffer[counter++] = s1 & 0b11111;
                s1 >>= 5;
            }
            buffer[counter++] = s1;

            for (int i = 0; i < counter; i++) {
                if (i != counter - 1) {
                    buffer[i] |= 0b100000;
                }
                char ch = IntToBase64(buffer[i]);
                ss.push_back(ch);
            }
        } else {
            char ch = IntToBase64(s1);
            ss.push_back(ch);
        }
    }

    void SourceMapGenerator::GenerateVLQStr(std::string& ss, int transformed_column,
                                            int file_index, int before_line, int before_column, int var_index) {
        IntToVLQ(ss, transformed_column);
        IntToVLQ(ss, file_index);
        IntToVLQ(ss, before_line);
        IntToVLQ(ss, before_column);
        IntToVLQ(ss, var_index);
    }

//    static constexpr size_t TableSize = sizeof Base64EncodingTable / sizeof(char);
//

    SourceMapGenerator::SourceMapGenerator(
            const std::shared_ptr<ModuleResolver>& resolver,
            const std::string& filename
    ): module_resolver_(resolver) {
        result["version"] = 3;
        result["file"] = filename;
        result["sourceRoot"] = "";
        result["sources"] = json::array();
        result["names"] = json::array();
    }

    void SourceMapGenerator::SetSourceRoot(const std::string &sr) {
        result["sourceRoot"] = sr;
    }

    void SourceMapGenerator::AddSource(const std::string &src) {
        result["sources"].push_back(src);
    }

    int32_t SourceMapGenerator::GetIdOfName(const UString& name) {
        auto iter = names_map_.find(name);
        if (iter != names_map_.end()) {
            return iter->second;
        }

        int32_t next_id = names_map_.size();
        names_map_[name] = next_id;
        result["names"].push_back(name.toStdString());
        return next_id;
    }

    bool SourceMapGenerator::AddLocation(const UString& name, int after_col, int fileId, int before_line, int before_col) {
        int32_t var_index = GetIdOfName(name);
        int32_t file_index = GetFilenameIndexByModuleId(fileId);
        if (unlikely(file_index < 0)) {
            return false;
        }
        GenerateVLQStr(mappings, after_col, file_index, before_line, before_col, var_index);
        mappings.push_back(',');
        return true;
    }

    int32_t SourceMapGenerator::GetFilenameIndexByModuleId(int32_t moduleId) {
        auto iter = module_id_to_index_.find(moduleId);
        if (iter != module_id_to_index_.end()) {
            // found;
            return iter->second;
        }
        auto mod = module_resolver_->findModuleById(moduleId);
        if (unlikely(mod == nullptr)) {
            // error;
            std::cerr << "sourcemap: get module by id failed: " << moduleId << std::endl;
            return -1;
        }

        AddSource(mod->path);

        int32_t index = src_counter_++;
        module_id_to_index_[mod->id] = index;
        return index;
    }

    void SourceMapGenerator::Finalize() {
        result["mappings"] = mappings;
    }

    std::string SourceMapGenerator::ToPrettyString() {
        try {
            Finalize();
            return result.dump(2);
        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return {};
        }
    }

}
