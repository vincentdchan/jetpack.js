//
// Created by Duzhong Chen on 2021/3/25.
//

#pragma once

#include <cinttypes>
#include "string/UString.h"

struct Position {
    uint32_t line;
    uint32_t column;

    inline Position() noexcept: line(1u), column(0u) {}
    inline Position(uint32_t l, uint32_t c) noexcept:
            line(l), column(c) {}
};

struct SourceLocation {
public:
    int32_t  fileId = -1;
    Position start;
    Position end;

    static SourceLocation NoOrigin;

    inline SourceLocation() noexcept = default;
    inline SourceLocation(int32_t fId, Position s, Position e) noexcept :
            fileId(fId), start(s), end(e) {}
};

static_assert(sizeof(SourceLocation) == 20, "fixed size");
