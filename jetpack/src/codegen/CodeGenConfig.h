//
// Created by Duzhong Chen on 2021/9/11.
//

#pragma once

namespace jetpack {

    struct CodeGenConfig {
    public:
        CodeGenConfig() = default;

        bool     minify = false;
        uint32_t start_indent_level = 0;
        std::string  indent = "  ";
        std::string  line_end = "\n";
        bool     sourcemap = false;
        bool     comments = true;

    };

}
