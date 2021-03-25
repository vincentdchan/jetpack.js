//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <cinttypes>
#include "string/UString.h"

struct Position {
    uint32_t line;
    uint32_t column;

    inline Position(): line(0u), column(0u) {}
    inline Position(uint32_t l, uint32_t c):
            line(l), column(c) {}
};

struct SourceLocation {
public:
    Position start;
    Position end;

    inline SourceLocation() = default;
    inline SourceLocation(Position s, Position e):
            start(s), end(e) {}
};

namespace FileIndex {

    uint32_t fileIndexOfFile(const UString& filePath);
    UString fileOfFileIndex(uint32_t);

    static const char* unknownFileName = "unknown";

}
