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

    inline int64_t GetCurrentMs() {
        using namespace std::chrono;
        milliseconds ms = duration_cast< milliseconds >(
                system_clock::now().time_since_epoch()
        );
        return ms.count();
    }

    inline std::string To_UTF8(const std::u16string &s) {
#ifndef _WIN32
        std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
        return conv.to_bytes(s);
#else
        std::size_t buffer_size = s.size() * 2;
        char* buffer = new char[buffer_size];
        memset(buffer, 0, buffer_size);

        int ret = WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK,
            reinterpret_cast<const wchar_t*>(s.c_str()),
            s.size(), buffer, buffer_size, NULL, NULL);
        
        if (ret == 0) {
            delete[] buffer;
            return "";
        }

        delete[] buffer;
        return std::string(buffer, ret);
#endif
    }

    inline bool IsFileExist(const std::string& path) {
#ifndef _WIN32
        return access(path.c_str(), F_OK) == 0;
#else
        DWORD dwAttrib = GetFileAttributesA(path.c_str());

        return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
    }

}

template <typename T>
using Sp = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using Vec = std::vector<T>;
