//
// Created by Duzhong Chen on 2020/3/24.
//

#include <cstdlib>
#include <string>
#include <vector>
#include "UniqueNameGenerator.h"
#include "Utils.h"

namespace rocket_bundle {

    static const char FirstCharCandidates[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";
    static const char CharCandidates[] = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";

    static constexpr std::size_t FirstCharCandidatesSize = sizeof(FirstCharCandidates) - 1;
    static constexpr std::size_t CharCandidatesSize = sizeof(CharCandidates) - 1;

    static auto TwoCharsJsKeywords = {
        "do", "if", "in",
    };

    static auto ThreeCharsJSKeywords = {
        "try", "new", "var", "for",
    };

    static auto LongJsKeywords = {
        "await", "break", "case", "catch", "class", "const", "continue", "debugger",
        "default", "delete", "else", "enum", "export", "extends", "false", "finally",
        "function", "import", "instanceof", "null", "return", "super",
        "switch", "this", "throw", "true", "typeof", "void", "while", "with",
        "yield"
    };

    UniqueNameGeneratorWithUsedName::UniqueNameGeneratorWithUsedName() {
        std::call_once(init_once_, [] {
            for (auto& keyword : LongJsKeywords) {
                long_keywords_set.insert(utils::To_UTF16(keyword));
            }
        });
    }

    bool UniqueNameGeneratorWithUsedName::IsJsKeyword(const std::u16string name) {
        if (name.size() == 1) {
            return false;
        } else if (name.size() == 2) {
            switch (name[0]) {
                case u'd':
                    return name[1] == 'o';

                case u'i':
                    return name[1] == 'f' || name[1] == 'n';

                default:
                    return false;

            }
        } else if (name.size() == 3) {
            switch (name[0]) {
                case u't':
                    return name == u"try";

                case u'n':
                    return name == u"new";

                case u'v':
                    return name == u"var";

                case u'f':
                    return name == u"for";

                default:
                    return false;

            }
        } else {
            return long_keywords_set.find(name) != long_keywords_set.end();
        }
    }

    std::once_flag UniqueNameGeneratorWithUsedName::init_once_;
    std::unordered_set<std::u16string> UniqueNameGeneratorWithUsedName::long_keywords_set;

    std::shared_ptr<ReadableNameGenerator> ReadableNameGenerator::Make() {
        std::shared_ptr<ReadableNameGenerator> result(new ReadableNameGenerator);
        result->weak_self = result;
        return result;
    }

    std::optional<std::u16string>
    ReadableNameGenerator::Next(const std::u16string &original_name) {
        if (!IsNameUsed(original_name)) {  // not exist
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
        if (IsJsKeyword(name)) {
            return true;
        }

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
        result->weak_self = result;
        return result;
    }

    std::shared_ptr<MinifyNameGenerator>
    MinifyNameGenerator::Merge(std::vector<std::shared_ptr<MinifyNameGenerator>> &vec) {
        auto result = Make();

        for (auto& ptr : vec) {
            if (ptr->counter > result->counter) {
                result->counter = ptr->counter;
            }
        }

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
        if (IsJsKeyword(name)) {
            return true;
        }

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
