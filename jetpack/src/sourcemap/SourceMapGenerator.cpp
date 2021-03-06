//
// Created by Duzhong Chen on 2020/7/13.
//

#include <cstring>
#include <iostream>
#include "JetJSON.h"
#include "io/FileIO.h"
#include "Benchmark.h"
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

    static std::once_flag back_encoding_init_;

    static int Base64BackEncodingTable[127];

    inline char IntToBase64(int code) {
        J_ASSERT(code >= 0 && code <= 0b111111);
        return Base64EncodingTable[code];
    }

    void SourceMapGenerator::IntToVLQ(std::string& ss, int code) {
        int buffer[IntToVLQBufferSize];

        J_ASSERT(code >= 0);
        int s1 = code << 1;
        int counter = 0;
        if (s1 > 0b11111) {
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

    int SourceMapGenerator::VLQToInt(const char* str, const char*& next) {
        std::call_once(back_encoding_init_, [] {
            memset(Base64BackEncodingTable, 0, sizeof(Base64BackEncodingTable));
            static constexpr size_t tableSize = sizeof(Base64EncodingTable) / sizeof(char);

            for (int i = 0; i < tableSize; i++) {
                char ch = Base64EncodingTable[i];
                Base64BackEncodingTable[ch] = i;
            }
        });

        uint32_t len = 0;
        int result = 0;

        std::vector<int> buffer;

        while (str[len] != '\0') {
            char ch = str[len++];
            J_ASSERT(ch < 127);
            int intValue = Base64BackEncodingTable[ch];
            if ((intValue & 0b100000) == 0) {  // has no next
                buffer.push_back(intValue & 0b11111);
                break;
            }
            buffer.push_back(intValue & 0b11111);
        }

        for (int32_t i = buffer.size() - 1; i >= 0; i--) {
            result = result << 5 | buffer[i];
        }

        next = str + len;

        return result >> 1;
    }

    void SourceMapGenerator::GenerateVLQStr(std::string& ss, int transformed_column,
                                            int file_index, int before_line, int before_column, int var_index) {
        IntToVLQ(ss, transformed_column);
        IntToVLQ(ss, file_index);
        IntToVLQ(ss, before_line);
        IntToVLQ(ss, before_column);
        if (var_index >= 0) {
            IntToVLQ(ss, var_index);
        }
    }

//    static constexpr size_t TableSize = sizeof Base64EncodingTable / sizeof(char);
//

    SourceMapGenerator::SourceMapGenerator(
            const std::shared_ptr<ModuleResolver>& resolver,
            const std::string& filename
    ): module_resolver_(resolver) {
        ss << "{" << std::endl;
        ss << R"(  "version": 3,)" << std::endl;
        ss << R"(  "file": ")" << EscapeJSONString(filename) << "\"," << std::endl;
        ss << R"(  "sourceRoot": "",)" << std::endl;
        ss << R"(  "names": [],)" << std::endl;
    }

    void SourceMapGenerator::AddSource(const Sp<ModuleFile>& moduleFile) {
        sources_.push_back(moduleFile);
//        result["sources"].push_back(moduleFile.Path());
//        result["sourcesContent"].push_back(moduleFile.src_content.toStdString());
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

        AddSource(mod);

        int32_t index = src_counter_++;
        module_id_to_index_[mod->id()] = index;
        return index;
    }

    void SourceMapGenerator::Finalize() {
        for (const auto& collector : collectors_) {
            FinalizeCollector(*collector);
        }

        benchmark::BenchMarker b(benchmark::BENCH_FINALIZE_SOURCEMAP);

        FinalizeSources();
        FinalizeSourcesContent();

        b.Submit();

        ss << R"(  "mappings": ")" << EscapeJSONString(mappings) << "\"" << std::endl;
        ss << "}";
    }

    void SourceMapGenerator::FinalizeSources() {
        if (sources_.empty()) {
            ss << R"(  "sources": [],)" << std::endl;
            return;
        }

        ss << R"(  "sources": [)" << std::endl;
        uint32_t counter = 0;
        for (auto& module : sources_) {
            ss << "    \"" << module->escaped_path.get() << "\"";
            if (counter++ < sources_.size() - 1) {
                ss << ",";
            }
            ss << std::endl;
        }
        ss << "  ]," << std::endl;
    }

    void SourceMapGenerator::FinalizeSourcesContent() {
        if (sources_.empty()) {
            ss << R"(  "sourcesContent": [],)" << std::endl;
            return;
        }
        ss << R"(  "sourcesContent": [)" << std::endl;

        uint32_t counter = 0;
        for (auto& module : sources_) {
            ss << "    \"" << module->escaped_src_content.get() << "\"";
            if (counter++ < sources_.size() - 1) {
                ss << ",";
            }
            ss << std::endl;
        }

        ss << "  ]," << std::endl;
    }

    void SourceMapGenerator::FinalizeCollector(const MappingCollector& mappingCollector) {
        for (const auto& item : mappingCollector.items_) {
            AddEnoughLines(item.dist_line);
            bool ec = AddLocation(item.name, item.dist_column,
                                  item.origin.fileId, item.origin.start.line, item.origin.start.column
                                  );
            J_ASSERT(ec);
        }
    }

    void SourceMapGenerator::AddEnoughLines(int32_t target_line) {
        while (line_counter_ < target_line) {
            line_counter_++;
            EndLine();
        }
    }

    bool SourceMapGenerator::AddLocation(const UString& name, int after_col, int fileId, int before_line, int before_col) {
        if (unlikely(fileId < 0)) {
            J_ASSERT(fileId != -1);
            return true;
        }
//        int32_t var_index = GetIdOfName(name);
        int32_t filenameIndex = GetFilenameIndexByModuleId(fileId);
        if (unlikely(filenameIndex < 0)) {
            return false;
        }
        GenerateVLQStr(mappings, after_col, filenameIndex, before_line, before_col, -1);
        mappings.push_back(',');
        return true;
    }

    std::string SourceMapGenerator::ToPrettyString() {
        return ss.str();
    }

    bool SourceMapGenerator::DumpFile(const std::string &path, bool pretty) {
        benchmark::BenchMarker sm(benchmark::BENCH_DUMP_SOURCEMAP);
        std::string finalStr = ToPrettyString();
        sm.Submit();

        benchmark::BenchMarker writeMark(benchmark::BENCH_WRITING_IO);
        io::IOError err = io::WriteBufferToPath(path, finalStr.c_str(), finalStr.size());
        writeMark.Submit();
        return err == io::IOError::Ok;
    }

}
