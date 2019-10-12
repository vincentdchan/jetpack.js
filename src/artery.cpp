//
// Created by Duzhong Chen on 2019/10/12.
//
#include <memory>
#include <iostream>
#include <fstream>
#include <jemalloc/jemalloc.h>
#include "artery.h"
#include "parser/parser.hpp"

using namespace std;

u16string Artery::ReadFileStream(const string& filename) {
    ifstream t(filename);
    string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return utils::To_UTF16(str);
}

void Artery::Enter(const std::string &entry) {
    lazy_thread_pool::get()->enqueue([this, entry] {
        try {
            auto src = make_shared<UString>();
            (*src) = Artery::ReadFileStream(entry);

            parser::Parser parser(src);

            auto module = std::shared_ptr<ModuleContainer>(new ModuleContainer);
            module->node = parser.ParseModule();

            {
                std::lock_guard guard(modules_mutex_);
                modules_.insert({ entry, module });
            }
        } catch (parser::ParseError& err) {
            std::cerr << err.ErrorMessage() << std::endl;
        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
        }

    });
}

void* Artery::ModuleContainer::operator new(std::size_t size) {
    return malloc(size);
}

void Artery::ModuleContainer::operator delete(void *chunk){
    free(chunk);
}
