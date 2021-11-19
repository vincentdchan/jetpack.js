//
// Created by Duzhong Chen on 2020/7/14.
//

#include <gtest/gtest.h>
#include <parser/ParserContext.h>
#include <ThreadPool.h>
#include "sourcemap/SourceMapGenerator.h"
#include "sourcemap/SourceMapDecoder.h"
#include "codegen/CodeGen.h"
#include "ModuleResolver.h"
#include "ModuleCompositor.h"
#include "SimpleAPI.h"
#include "utils/Path.h"
#include "utils/io/FileIO.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndGenSourceMap(const std::string& content, bool print) {
    auto resolver = std::make_shared<ModuleResolver>();
    Config config = Config::Default();
    resolver->BeginFromEntryString(config, content);

    std::string sourcemap;
    io::StringWriter sourcemap_writer(sourcemap);
    auto sourcemap_generator = std::make_shared<SourceMapGenerator>(resolver, sourcemap_writer, "memory0");

    CodeGenConfig codegen_config;
    codegen_config.sourcemap = true;
    std::string bundle;
    io::StringWriter bundle_writer(bundle);
    ModuleCompositor module_compositor(bundle_writer, codegen_config);
    module_compositor.DumpSources(sourcemap_generator);

    {
        CodeGenFragment fragment;
        auto mod = resolver->GetEntryModule();
        CodeGen codegen(codegen_config, fragment);
        codegen.Traverse(*mod->ast);
        module_compositor.Append(fragment);
    }

    auto fut = module_compositor.DumpSourcemap(sourcemap_generator);

    if (print) {
        std::cout << "gen: " << std::endl << sourcemap << std::endl;
    }

    fut.wait();

    return sourcemap;
}

static std::string encoding_vlq(const std::vector<int>& array) {
    std::string str;
    io::StringWriter writer(str);

    for (const auto& value : array) {
        SourceMapGenerator::IntToVLQ(writer, value);
    }

    return str;
}

static std::vector<int> decoding_vlq(const std::string& str) {
    std::vector<int> result;

    const char* next = str.c_str();
    while (next < str.c_str() + str.size()) {
        result.push_back(SourceMapGenerator::VLQToInt(next, next));
    }

    return result;
}

TEST(SourceMap, VLQEncoding) {
    std::string str;
    io::StringWriter writer(str);
    SourceMapGenerator::IntToVLQ(writer, 16);
    EXPECT_STREQ(str.c_str(), "gB");

    EXPECT_EQ(encoding_vlq({ 0, 0, 0, 0 }), "AAAA");
    EXPECT_EQ(encoding_vlq({ 0, 0, 16, 1 }), "AAgBC");
    EXPECT_EQ(encoding_vlq({ 0, 0, 0, 0 }), "AAAA");
    EXPECT_EQ(encoding_vlq({ -1 }), "D");
    EXPECT_EQ(encoding_vlq({ 1,2,3,4,5,6,7,8,9,11 }), "CEGIKMOQSW");
}

TEST(SourceMap, Decode) {
    const char* next;
    EXPECT_EQ(SourceMapGenerator::VLQToInt("gB", next), 16);

    std::vector<int> testCases { 10, 1000, 1234, 100000 };
    for (auto i : testCases) {
        std::string vlq;
        io::StringWriter writer(vlq);
        SourceMapGenerator::IntToVLQ(writer, i);

        EXPECT_TRUE(!vlq.empty());
        int back = SourceMapGenerator::VLQToInt(vlq.c_str(), next);
        EXPECT_EQ(back, i);
    }

    auto vec = decoding_vlq("D");
    EXPECT_EQ(vec[0], -1);
}

TEST(SourceMap, Simple) {
    std::string src(""
                "function main() {\n"
                "    console.log('hello world');\n"
                "}\n"
    );
    auto result = ParseAndGenSourceMap(std::move(src), true);

    std::cout << result << std::endl;
    auto resultJson = nlohmann::json::parse(result);
    EXPECT_EQ(resultJson["version"].get<int>(), 3);
    EXPECT_STREQ(resultJson["file"].get<std::string>().c_str(), "memory0");
    EXPECT_TRUE(resultJson["sources"].is_array());
    EXPECT_TRUE(resultJson["names"].is_array());

    SourceMapDecoder decoder(resultJson);
    auto decodeResult = decoder.Decode();

    for (const auto& item : resultJson["sources"]) {
        std::cout << "source: " << item.get<std::string>() << std::endl;
    }

    for (const auto& map : decodeResult.content) {
        std::cout << map.ToString() << std::endl;
    }
}

TEST(SourceMap, Complex) {
    Path path(JETPACK_TEST_RUNNING_DIR);
    path.Join("tests/fixtures/sourcemap/index.js");

    auto entryPath = path.ToString();
    std::cout << "dir: " << entryPath << std::endl;

    EXPECT_TRUE(io::IsFileExist(entryPath));

    Path outputPath(JETPACK_BUILD_DIR);
    outputPath.Join("sourcemap_bundle_test.js");

    std::cout << "output dir: " << outputPath.ToString() << std::endl;

    JetpackFlags flags;
    flags |= JETPACK_JSX;
    flags |= JETPACK_SOURCEMAP;
    flags |= JETPACK_TRACE_FILE;
    std::string output_str = outputPath.ToString();
    EXPECT_EQ(jetpack_bundle_module(entryPath.c_str(), output_str.c_str(), static_cast<int>(flags), nullptr), 0);

    std::string sourcemapContent;
    EXPECT_EQ(io::ReadFileToStdString(outputPath.ToString() + ".map", sourcemapContent), io::IOError::Ok);

    auto sourcemapJson = nlohmann::json::parse(sourcemapContent);
    std::string mapping = sourcemapJson["mappings"];
    std::cout << "mapping: " << mapping << std::endl;

    SourceMapDecoder decoder(sourcemapJson);
    auto result = decoder.Decode();

    EXPECT_EQ(sourcemapJson["sources"].size(), 2);
    EXPECT_EQ(sourcemapJson["sourcesContent"].size(), 2);

    for (const auto& item : sourcemapJson["sources"]) {
        std::cout << "source: " << item.get<std::string>() << std::endl;
    }

    for (const auto& map : result.content) {
        std::cout << map.ToString() << std::endl;
    }

    std::vector<SourceMapDecoder::ResultMapping> expect_mappings {
            { 1, 3, 4, 2, 2 },
            { 1, 3, 12, 2, 10 },
            { 1, 3, 16, 2, 14 },
            { 0, 3, 0, 4, 0},
    };

    EXPECT_EQ(expect_mappings.size(), result.content.size());

    for (uint32_t i = 0 ; i < expect_mappings.size(); i++) {
        EXPECT_EQ(expect_mappings[i], result.content[i]);
    }
}
