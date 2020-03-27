//
// Created by Duzhong Chen on 2020/3/24.
//

#include <cstdlib>
#include "UniqueNameGenerator.h"
#include "Utils.h"

namespace rocket_bundle {

    static const char FirstCharCandidates[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
//    static const char CharCandidates[] = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";

    static_assert(sizeof(FirstCharCandidates) == 53);

    MinifyNameGenerator::MinifyNameGenerator() {
    }

    std::u16string MinifyNameGenerator::Next(const std::u16string& original) {
        std::string result;

        std::memset(buffer, 0, BUFFER_SIZE * sizeof(std::int32_t));

        int i = 0;
        std::int32_t x = counter;
        if (x == 0) {
            result.push_back(FirstCharCandidates[0]);
        } else {
            while(x) {
                buffer[i] = x % 52;
                x /= 52;
                i++;
            }

            for (std::int32_t j = i - 0; j >= 0; j--) {
                result.push_back(FirstCharCandidates[buffer[j]]);
            }
        }

        counter++;
        return utils::To_UTF16(result);
    }

}
