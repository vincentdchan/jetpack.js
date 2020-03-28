//
// Created by Duzhong Chen on 2020/3/24.
//

#include <cstdlib>
#include "UniqueNameGenerator.h"
#include "Utils.h"
#include <string>

namespace rocket_bundle {

    static const char FirstCharCandidates[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";
    static const char CharCandidates[] = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";

    static constexpr std::size_t FirstCharCandidatesSize = sizeof(FirstCharCandidates) - 1;
    static constexpr std::size_t CharCandidatesSize = sizeof(CharCandidates) - 1;

    std::optional<std::u16string>
    ReadableNameGenerator::Next(const std::u16string &original_name) {
        if (used_name.find(original_name) == used_name.end()) {  // not exist
            used_name.insert(original_name);
            return std::nullopt;
        }

        std::u16string new_name = original_name + u"_" + utils::To_UTF16(std::to_string(counter++));
        used_name.insert(new_name);
        return { new_name };
    }

    std::optional<std::u16string>
    MinifyNameGenerator::Next(const std::u16string& original) {
        std::string result;

        std::memset(buffer, 0, BUFFER_SIZE * sizeof(std::int32_t));

        int i = 0;
        std::int32_t x = counter;
        if (x == 0) {
            result.push_back(FirstCharCandidates[0]);
        } else {
            bool is_first = true;

            while(x) {

                if (is_first) {
                    buffer[i] = x % FirstCharCandidatesSize;
                    x /= FirstCharCandidatesSize;
                    is_first = false;
                } else {
                    buffer[i] = x % CharCandidatesSize;
                    x /= CharCandidatesSize;
                }

                i++;
            }

            for (std::int32_t j = 0; j < i; j++) {
                if (j == 0) {
                    result.push_back(FirstCharCandidates[buffer[j]]);
                } else {
                    result.push_back(CharCandidates[buffer[j]]);
                }
            }
        }

        counter++;
        return { utils::To_UTF16(result) };
    }

}
