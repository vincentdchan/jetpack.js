//
// Created by Duzhong Chen on 2020/3/22.
//

#pragma once

#include <set>
#include <vector>
#include <robin_hood.h>
#include <optional>
#include <memory>
#include "utils/Common.h"
#include "utils/string/UString.h"
#include "parser/NodeTypes.h"

namespace jetpack {

    class SyntaxNode;

    class ExternalVariable {
    public:
        bool is_export_all;
        std::string source_name;

        std::vector<std::string> export_names;

    };

    struct LocalExportInfo {
    public:
        std::string export_name;
        std::string local_name;
        std::optional<ExportDefaultDeclaration*> default_export_ast;

    };

    struct ExternalExportAlias {
    public:
        std::string source_name;
        std::string export_name;

    };

    struct ExternalExportInfo {
    public:
        std::string relative_path;
        bool is_export_all = false;
        std::vector<ExternalExportAlias> names;

    };

    /**
     * Ref: https://github.com/vincentdchan/webpack-deep-scope-analysis-plugin/blob/master/packages/deep-scope-analyser/src/exportManager.ts
     */
    class ExportManager {
    public:
        enum EC {
            Ok = 0,

            UnknownSpecifier = -1,
            FunctionHasNoId = -2,
            ClassHasNoId = -3,

            UnsupportExport = -4,

        };

        static const char* ECToStr(EC ec);

        ExportManager() = default;
        ExportManager(const ExportManager&) = delete;

        ExportManager& operator=(const ExportManager) = delete;

        EC ResolveAllDecl(ExportAllDeclaration* decl);
        EC ResolveDefaultDecl(ExportDefaultDeclaration* decl);
        EC ResolveNamedDecl(ExportNamedDeclaration* decl);

        void AddLocalExport(const std::shared_ptr<LocalExportInfo>& info);

        // key: export name
        HashMap<std::string, Sp<LocalExportInfo>> local_exports_name;

        // key: local_name
        HashMap<std::string, Sp<LocalExportInfo>> local_exports_by_local_name;

        // key: absolute path
        HashMap<std::string, ExternalExportInfo> external_exports_map;

        std::vector<ExternalExportInfo> CollectExternalInfos();

    };

}
