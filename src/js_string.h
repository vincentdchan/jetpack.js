//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class JS_String;

class StringContext {
public:

    class StringNode {
    public:
        std::int32_t ref_count_ = 0;
        std::u32string content_;
    };

    static StringContext* Instance();
    static JS_String MakeString(const std::string& str_);
    static JS_String MakeString(const std::u32string& str_);
    static void ReleaseString(JS_String& str_);

    inline std::size_t Size() {
        return data_map_.size();
    }

private:
    static StringContext* instance_;

    JS_String MakeString_(const std::u32string& str_);
    void ReleaseString_(JS_String& str_);

    std::unordered_map<std::u32string, std::unique_ptr<StringNode>> data_map_;

};

class JS_String {
public:
    friend StringContext;

    JS_String() = default;
    JS_String(JS_String&& string) noexcept;
    JS_String(const JS_String& that_);

    JS_String& operator=(const std::string& str_);
    JS_String& operator=(const JS_String& that_);
    JS_String& operator=(JS_String&& that_) noexcept;

    JS_String operator+(const JS_String& that_);

    bool operator==(const JS_String& that_) const;
    char32_t operator[](int32_t index) const;

    std::string ToUTF8String() const;

    std::uint32_t Size() const;

    ~JS_String();

private:
    StringContext::StringNode* node_data_ = nullptr;

};
