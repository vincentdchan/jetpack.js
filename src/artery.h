//
// Created by Duzhong Chen on 2019/10/12.
//
#pragma once

#include <mutex>
#include <string>
#include <memory>
#include <robin_hood.h>
#include <atomic>
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

    void WaitForParsingFinished();

    ~Artery() = default;

private:
    void IncreaseProcessingCount();
    void IncreaseFinishedCount();

    robin_hood::unordered_map<std::string, std::shared_ptr<ModuleContainer>> modules_;
    std::mutex modules_mutex_;

    std::mutex count_change_mutex_;
    std::condition_variable count_change_cv_;
    std::atomic<std::size_t> processing_count_ = 0;
    std::atomic<std::size_t> finished_count_ = 0;

};

class Artery::ModuleContainer {
public:
    enum class ProcessState {
        INIT = 0,
        PROCEED = 1,
        ERROR = 2,
    };

    Sp<Module> node;
    std::atomic<ProcessState> state = ProcessState::INIT;
    std::string error_message;

    void* operator new(std::size_t size);
    void operator delete(void* chunk);

};
