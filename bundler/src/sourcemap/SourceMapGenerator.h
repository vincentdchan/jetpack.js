//
// Created by Duzhong Chen on 2020/7/13.
//

#pragma once

#include <sstream>
#include <vector>
#include <string>

namespace jetpack {

    class SourceMapGenerator {
    public:
        static bool GenerateVLQStr(std::stringstream& ss, int transformed_column, int file_index, int before_line, int before_column, int var_index);
        static bool IntToVLQ(std::stringstream& ss, int code);
        static bool IntToBase64(int, char& ch);

        std::string output_filename;
        std::vector<std::string> sources;
        std::vector<std::string> names;
        std::stringstream ss;

        SourceMapGenerator() = default;

        SourceMapGenerator(std::string filename): output_filename(std::move(filename)) {
        }

        bool AddLocation(const std::string& name, int after_col, int file_index, int before_line, int before_col);

    };

}

