//
// Created by Duzhong Chen on 2020/7/14.
//

#include <gtest/gtest.h>
#include <parser/ParserContext.h>
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

inline std::string ParseAndGenSourceMap(const UString& content, bool print) {
    auto resolver = std::make_shared<ModuleResolver>();
    ParserContext::Config config = ParserContext::Config::Default();
    resolver->BeginFromEntryString(config, content);

    SourceMapGenerator sourceMapGenerator(resolver, "memory0");

    auto mod =  resolver->GetEntryModule();
    CodeGen::Config codegenConfig;
    CodeGen codegen(codegenConfig, mod->mapping_collector_);
    codegen.Traverse(mod->ast);
    mod->codegen_result = codegen.GetResult();

    resolver->EscapeAllContent();

    ModuleCompositor compositor(sourceMapGenerator);
    compositor.append(mod->codegen_result.content, mod->mapping_collector_);
    auto composition = compositor.Finalize();

    if (print) {
        std::cout << "gen: " << std::endl << UStringToUtf8(composition) << std::endl;
    }

    return sourceMapGenerator.ToPrettyString();
}

TEST(SourceMap, VLQEncoding) {
    std::string str;
    SourceMapGenerator::IntToVLQ(str, 16);
    EXPECT_STREQ(str.c_str(), "gB");

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
}

TEST(SourceMap, Simple) {
    UString src(u""
                "function main() {\n"
                "    console.log('hello world');\n"
                "}\n"
    );
    auto result = ParseAndGenSourceMap(src, true);

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

    simple_api::Flags flags;
    flags.setJsx(true);
    flags.setMinify(false);
    flags.setSourcemap(true);
    EXPECT_EQ(simple_api::BundleModule(entryPath, outputPath.ToString(), flags), 0);

    std::string sourcemapContent;
    EXPECT_EQ(io::ReadFileToStdString(outputPath.ToString() + ".map", sourcemapContent), io::IOError::Ok);

    auto sourcemapJson = nlohmann::json::parse(sourcemapContent);
    std::string mapping = sourcemapJson["mappings"];
    std::cout << "mapping: " << mapping << std::endl;

    SourceMapDecoder decoder(sourcemapJson);
    auto result = decoder.Decode();

    for (const auto& item : sourcemapJson["sources"]) {
        std::cout << "source: " << item.get<std::string>() << std::endl;
    }

    for (const auto& map : result.content) {
        std::cout << map.ToString() << std::endl;
    }

    std::vector<SourceMapDecoder::ResultMapping> expect_mappings {
            { 0, 3, 0, 1, 0 },
            { 1, 3, 4, 4, 2 },
            { 1, 3, 12, 4, 10},
    };

    EXPECT_EQ(expect_mappings.size(), result.content.size());

    for (uint32_t i = 0 ; i < expect_mappings.size(); i++) {
        EXPECT_EQ(expect_mappings[i], result.content[i]);
    }
}
