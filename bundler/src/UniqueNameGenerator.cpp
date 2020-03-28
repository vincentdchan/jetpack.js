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

    static auto JsKeywords = {
        "await", "break", "case", "catch", "class", "const", "continue", "debugger",
        "default", "delete", "do", "else", "enum", "export", "extends", "false", "finally",
        "for", "function", "if", "import", "ininstanceof", "new", "null", "return", "super",
        "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with",
        "yield"
    };

    void UniqueNameGeneratorWithUsedName::InsertJsKeywords() {
        for (auto& keyword : JsKeywords) {
            used_name.insert(utils::To_UTF16(keyword));
        }
    };

    std::shared_ptr<ReadableNameGenerator> ReadableNameGenerator::Make() {
        std::shared_ptr<ReadableNameGenerator> result(new ReadableNameGenerator);
        result->InsertJsKeywords();
        result->weak_self = result;
        return result;
    }

    std::optional<std::u16string>
    ReadableNameGenerator::Next(const std::u16string &original_name) {
        if (IsNameUsed(original_name)) {  // not exist
            used_name.insert(original_name);
            return std::nullopt;
        }

        std::u16string new_name = original_name + u"_" + utils::To_UTF16(std::to_string(counter++));
        used_name.insert(new_name);
        return { new_name };
    }

    std::shared_ptr<UniqueNameGenerator> ReadableNameGenerator::Fork() {
        std::shared_ptr<ReadableNameGenerator> result(new ReadableNameGenerator);
        result->weak_self = result;
        result->prev = this->weak_self.lock();
        result->counter = this->counter;
        // used_name is empty
        return result;
    }

    bool ReadableNameGenerator::IsNameUsed(const std::u16string &name) {
        std::shared_ptr<ReadableNameGenerator> self = weak_self.lock();

        while (self != nullptr) {
            if (self->used_name.find(name) != self->used_name.end()) {  // found
                return true;
            }
            self = self->prev;
        }

        return false;
    }

    std::shared_ptr<MinifyNameGenerator> MinifyNameGenerator::Make() {
        std::shared_ptr<MinifyNameGenerator> result(new MinifyNameGenerator);
        result->InsertJsKeywords();
        result->weak_self = result;
        return result;
    }

    MinifyNameGenerator::MinifyNameGenerator() {
        std::memset(buffer, 0, BUFFER_SIZE * sizeof(std::int32_t));
    }

    std::optional<std::u16string>
    MinifyNameGenerator::Next(const std::u16string& original) {
        std::u16string result;

        do {
            result = GenAName();
        } while(IsNameUsed(result));

        return { result };
    }

    std::shared_ptr<UniqueNameGenerator> MinifyNameGenerator::Fork() {
        std::shared_ptr<MinifyNameGenerator> result(new MinifyNameGenerator);
        result->weak_self = result;
        result->prev = this->weak_self.lock();
        result->counter = this->counter;
        // used_name is empty
        return result;
    }

    bool MinifyNameGenerator::IsNameUsed(const std::u16string &name) {
        std::shared_ptr<MinifyNameGenerator> self = weak_self.lock();

        while (self != nullptr) {
            if (self->used_name.find(name) != self->used_name.end()) {  // found
                return true;
            }
            self = self->prev;
        }

        return false;
    }

    std::u16string MinifyNameGenerator::GenAName() {
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

        return utils::To_UTF16(result);
    }

}
