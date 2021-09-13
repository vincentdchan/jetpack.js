//
// Created by Duzhong Chen on 2021/9/13.
//

#pragma once

#include <string>
#include <string_view>

class MemoryViewOwner {
public:
    virtual std::string_view View() = 0;

    virtual ~MemoryViewOwner() noexcept = default;

};

class RawMemoryViewOwner : public MemoryViewOwner {
public:
    explicit RawMemoryViewOwner(std::string_view data): data_(data) {}

    std::string_view View() override {
        return data_;
    }

private:
    std::string_view data_;

};

class StringMemoryOwner : public MemoryViewOwner {
public:
    explicit StringMemoryOwner(std::string data): data_(std::move(data)) {}

    std::string_view View() override {
        return std::string_view(data_);
    }

private:
    std::string data_;

};
