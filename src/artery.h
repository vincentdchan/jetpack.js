//
// Created by Duzhong Chen on 2019/10/12.
//
#pragma once

#include <mutex>
#include <string>
#include <google/sparse_hash_map>
#include <memory>
#include "parser/syntax_nodes.h"
#include "lazy_thread_pool.h"

class Artery {
public:
    static std::u16string ReadFileStream(const std::string& filename);

    class ModuleContainer;

    Artery() = default;
    Artery(const Artery&) = delete;
    Artery(Artery&&) = delete;

    Artery& operator=(const Artery& that) = delete;
    Artery& operator=(Artery&&) = delete;

    void Enter(const std::string& entry);

    ~Artery() = default;

private:
    google::sparse_hash_map<std::string, std::shared_ptr<ModuleContainer>> modules_;
    std::mutex modules_mutex_;

};

class Artery::ModuleContainer {
public:
    Sp<Module> node;

    void* operator new(std::size_t size);
    void operator delete(void* chunk);

};
