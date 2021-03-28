//
// Created by Duzhong Chen on 2020/3/21.
//

#pragma once
#include <locale>
#include <codecvt>
#include <string>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include <cassert>

#ifndef _WIN32
#include <unistd.h>
#include <robin_hood.h>
#else
#include <unordered_set>
#include <unordered_map>
#include <Windows.h>
#endif

#ifndef _WIN32
template <typename Key>
using HashSet = robin_hood::unordered_set<Key>;

template <typename Key, typename Value>
using HashMap = robin_hood::unordered_map<Key, Value>;
#else
template <typename Key>
using HashSet = std::unordered_set<Key>;

template <typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;
#endif

#ifdef _WIN32
#include <BaseTsd.h>
#include <intrin.h>
#include <Windows.h>
#include "win_patch.h"
#define ATTR_FORMAT(N, M)
#define ATTR_UNUSED
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#define likely(x)       !!(x)
#define unlikely(x)     !!(x)
#define popen(x, y) _popen(x, y)
#define pclose(x) _pclose(x)
#define force_inline __forceinline
#define no_inline __declspec(noinline)
#define __exception
#define __maybe_unused
typedef SSIZE_T ssize_t;
#else
#define ATTR_FORMAT(N, M) __attribute__((format(printf, N, M)))
#define ATTR_UNUSED __attribute((unused))
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define force_inline inline __attribute__((always_inline))
#define no_inline __attribute__((noinline))
#define __maybe_unused __attribute__((unused))
#endif

namespace jetpack::utils {
    using std::int64_t;

    inline std::string GetRunningDir() {
        static char buffer[1024];
        std::memset(buffer, 0, 1024);
#ifndef _WIN32
        auto result = getcwd(buffer, 1024);
        if (result == nullptr) {
            return "";
        }
        return result;
#else
        if (GetCurrentDirectoryA(1024, buffer) == 0) {
            return "";
        }

        return buffer;
#endif
    }

}

template <typename T>
using Sp = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using Vec = std::vector<T>;

#if defined(_DEBUG)
#define J_ASSERT(s) assert(s)
#else
#define J_ASSERT(s)
#endif
