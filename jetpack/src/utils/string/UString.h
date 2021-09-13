//
// Created by Duzhong Chen on 2021/3/23.
//

#pragma once

#include <memory>
#include <cinttypes>
#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <fmt/format.h>

#include "utils/Common.h"

std::string StringFromCodePoint(char32_t cp);
std::string StringFromUtf32(const char32_t* content, std::size_t size);

char32_t ReadCodepointFromUtf8(const uint8_t buf[], uint32_t *idx, size_t strlen);
std::size_t UTF16LenOfUtf8(std::string_view u8);
