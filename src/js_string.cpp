//
// Created by Duzhong Chen on 2019/9/3.
//
#include "js_string.h"
#include "utils.h"

StringContext* StringContext::Instance() {
    if (instance_) return instance_;
    instance_ = new StringContext();
    return instance_;
}

JS_String StringContext::MakeString(const std::string &str_) {
    auto w_str = utils::To_UTF16(str_);
    return Instance()->MakeString_(w_str);
}

JS_String StringContext::MakeString(const UString& str_) {
    return Instance()->MakeString_(str_);
}

void StringContext::ReleaseString(JS_String& str_) {
    return Instance()->ReleaseString_(str_);
}

JS_String::JS_String(const JS_String &that_) {
    node_data_ = that_.node_data_;
    if (node_data_) {
        node_data_->ref_count_++;
    }
}

JS_String::JS_String(JS_String &&that_) noexcept {
    node_data_ = that_.node_data_;
    that_.node_data_ = nullptr;
}

JS_String& JS_String::operator=(const std::string &str_) {
    StringContext::ReleaseString(*this);

    auto new_str = StringContext::MakeString(str_);
    (*this) = std::move(new_str);

    return *this;
}

JS_String& JS_String::operator=(const JS_String &that_) {
    StringContext::ReleaseString(*this);

    node_data_ = that_.node_data_;
    if (node_data_) {
        node_data_->ref_count_++;
    }

    return *this;
}

JS_String& JS_String::operator=(JS_String &&that_) noexcept {
    node_data_ = that_.node_data_;

    that_.node_data_ = nullptr;

    return *this;
}

bool JS_String::operator==(const JS_String &that_) const {
    return node_data_ == that_.node_data_;
}

char32_t JS_String::operator[](int32_t index) const {
    if (node_data_ == nullptr) return 0;
    return node_data_->content_[index];
}

std::string JS_String::ToUTF8String() const {
    if (node_data_ == nullptr) return std::string();

    return utils::To_UTF8(node_data_->content_);
}

JS_String JS_String::operator+(const JS_String &that_) {
    UString content;

    if (node_data_) {
        for (auto ch : node_data_->content_) {
            content.push_back(ch);
        }
    }

    if (that_.node_data_) {
        for (auto ch : that_.node_data_->content_) {
            content.push_back(ch);
        }
    }

    return StringContext::MakeString(content);
}

std::uint32_t JS_String::Size() const {
    if (node_data_) {
        return node_data_->content_.size();
    }
    return 0;
}

JS_String::~JS_String() {
    StringContext::ReleaseString(*this);
}

JS_String StringContext::MakeString_(const UString &str_) {
    JS_String value;

    if (str_.empty()) {
        return value;
    }

    auto node = std::make_unique<StringNode>();
    node->content_ = str_;
    node->ref_count_++;

    value.node_data_ = node.get();
    data_map_.emplace(str_, std::move(node));

    return value;
}

void StringContext::ReleaseString_(JS_String &str_) {
    if (str_.node_data_ == nullptr) return;
    if (--str_.node_data_->ref_count_ == 0) {
        data_map_.erase(str_.node_data_->content_);
    }
}

StringContext* StringContext::instance_ = nullptr;
