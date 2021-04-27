//
// Created by Duzhong Chen on 2020/3/23.
//

#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace jetpack {

    /**
     * a mutable absolute path
     * for performance
     */
    class Path {
    public:
        static const char PATH_DIV = '/';

        Path() = default;

        inline explicit Path(const std::string& origin) {
            if (origin.empty()) {
                return;
            }
#ifndef WIN32
            if (origin[0] != PATH_DIV) {
#else
            if (origin.length() < 2 || origin[1] != ':') {
#endif
                throw std::runtime_error("path should be absolute");
            }

#ifndef WIN32
            std::size_t i = 1;
#else
            std::size_t i = 0;
#endif
            std::string buffer;

            while (i < origin.size()) {
                if (origin[i] != PATH_DIV) {
                    buffer.push_back(origin[i]);
                } else {
                    slices.emplace_back(std::move(buffer));
                }
                i++;
            }

            if (!buffer.empty()) {
                slices.emplace_back(std::move(buffer));
            }
        }

        inline bool EndsWith(const std::string& str) {
            if (slices.empty()) {
                return false;
            }

            auto& last_str = slices[slices.size() - 1];
            if (last_str.size() < str.size()) {
                return false;
            }

            std::int32_t i = str.size() - 1;

            while (i >= 0) {
                if (str[i] != last_str[last_str.size() - str.size() + i]) {
                    return false;
                }
                i--;
            }
            return true;
        }

        inline void Join(const std::string& path) {
            if (path.empty()) {
                return;
            }
            std::size_t i = 0;
            std::string buffer;
            while (i < path.size()) {
                if (path[i] == '.') {
                    i++;

                    // ..
                    if (i < path.size() && path[i] == '.') {
                        slices.pop_back();
                        i++;
                    }
                } else {
                    while (path[i] != PATH_DIV && i < path.size()) {
                        buffer.push_back(path[i++]);
                    }
                    slices.emplace_back(std::move(buffer));
                }

                while (path[i] == PATH_DIV && i < path.size()) {
                    i++;
                }
            }
        }

        [[nodiscard]] inline std::string ToString() const {
            std::stringstream ss;

#ifdef WIN32
            auto it = slices.begin();
            // First element is the drive letter, e.g. C:
            if (it != slices.end()) {
                ss << *it;

                while (++it != slices.end()) {
                    ss << PATH_DIV;
                    ss << *it;
                }
            }
#else
            for (auto& item : slices) {
                ss << PATH_DIV;
                ss << item;
            }
#endif
            return ss.str();
        }

        inline void Pop() {
            slices.pop_back();
        }

        std::vector<std::string> slices;

    };

}
