//
// Created by Duzhong Chen on 2019/10/12.
//
#include <memory>
#include <iostream>
#include <fstream>
#include <jemalloc/jemalloc.h>
#include "Utils.h"
#include "artery.h"
#include "parser/Parser.hpp"

using namespace std;

namespace rocket_bundle {

    u16string Artery::ReadFileStream(const string& filename) {
        ifstream t(filename);
        string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());
        return utils::To_UTF16(str);
    }

    void Artery::Enter(const std::string &entry) {
        return;
    }

    void Artery::WaitForParsingFinished() {
        while (true) {
            std::unique_lock<std::mutex> lk(count_change_mutex_);
            count_change_cv_.wait(lk);

            if (processing_count_ == 0) break;

            if (finished_count_ < processing_count_) {
                std::cout << "Processing js module " << finished_count_ + 1 << " of " << processing_count_ << std::endl;
            } else {
                break;
            }
        }
    }

    void Artery::IncreaseProcessingCount() {
        processing_count_++;
        count_change_cv_.notify_all();
    }

    void Artery::IncreaseFinishedCount() {
        finished_count_++;
        count_change_cv_.notify_all();
    }

    void* Artery::ModuleContainer::operator new(std::size_t size) {
        return malloc(size);
    }

    void Artery::ModuleContainer::operator delete(void *chunk){
        free(chunk);
    }

}

