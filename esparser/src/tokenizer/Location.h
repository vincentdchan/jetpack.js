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
    int32_t  fileId = -1;
    Position start;
    Position end;

    inline SourceLocation() = default;
    inline SourceLocation(int32_t fId, Position s, Position e):
            fileId(fId), start(s), end(e) {}
};

static_assert(sizeof(SourceLocation) == 20, "fixed size");

namespace FileIndex {

    uint32_t fileIndexOfFile(const UString& filePath);
    UString fileOfFileIndex(uint32_t);

}
