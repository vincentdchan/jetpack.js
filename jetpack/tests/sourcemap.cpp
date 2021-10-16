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
#include "SimpleAPI.h"
#include "utils/Path.h"
#include "utils/io/FileIO.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndGenSourceMap(const std::string& content, bool print) {
    auto resolver = std::make_shared<ModuleResolver>();
    Config config = Config::Default();
    resolver->BeginFromEntryString(config, content);

    SourceMapGenerator sourceMapGenerator(resolver, "memory0");

    auto mod = resolver->GetEntryModule();
    CodeGenConfig codegenConfig;
    CodeGen codegen(codegenConfig, mod->mapping_collector_);
    codegen.Traverse(*mod->ast);

    ThreadPool pool(1);
    sourceMapGenerator.Finalize(pool);

    if (print) {
        std::cout << "gen: " << std::endl << sourceMapGenerator.ToPrettyString() << std::endl;
    }

    return sourceMapGenerator.ToPrettyString();
}

static std::string encoding_vlq(const std::vector<int>& array) {
    std::string str;

    for (const auto& value : array) {
        SourceMapGenerator::IntToVLQ(str, value);
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
    SourceMapGenerator::IntToVLQ(str, 16);
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
        SourceMapGenerator::IntToVLQ(vlq, i);

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
    flags |= JetpackFlag::Jsx;
    flags |= JetpackFlag::Sourcemap;
    flags |= JetpackFlag::TraceFile;
    EXPECT_EQ(simple_api::BundleModule(entryPath, outputPath.ToString(), flags), 0);

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
            { 0, 3, 4, 2, 2 },
            { 0, 3, 12, 2, 10 },
            { 0, 3, 16, 2, 14 },
            { 1, 3, 0, 4, 0},
    };

    EXPECT_EQ(expect_mappings.size(), result.content.size());

    for (uint32_t i = 0 ; i < expect_mappings.size(); i++) {
        EXPECT_EQ(expect_mappings[i], result.content[i]);
    }
}
