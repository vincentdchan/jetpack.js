//
// Created by Duzhong Chen on 2019/10/12.
//
#include <memory>
#include <iostream>
#include <fstream>
#include <jemalloc/jemalloc.h>
#include "utils.h"
#include "artery.h"
#include "parser/Parser.hpp"

using namespace std;

namespace rocket_bundle {

    u16string Artery::ReadFileStream(const string& filename) {
        ifstream t(filename);
        string str((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());
        return parser_utils::To_UTF16(str);
    }

    void Artery::Enter(const std::string &entry) {
        std::shared_ptr<ModuleContainer> module_container;
        try {
            {
                std::lock_guard guard(modules_mutex_);

                if (modules_.find(entry) != modules_.end()) return; // module exists, quite;

                module_container.reset(new ModuleContainer);
                modules_.insert({ entry, module_container });
            }
            IncreaseProcessingCount();

            auto src = make_shared<UString>();
            (*src) = Artery::ReadFileStream(entry);

            auto config = parser::ParserContext::Config::Default();
            auto ctx = make_shared<parser::ParserContext>(src, config);
            parser::Parser parser(ctx);

            parser.OnImportDeclarationCreated([this] (const Sp<ImportDeclaration>& node) {
                auto source_path = parser_utils::To_UTF8(node->source->raw);
                Enter(source_path);
            });

            if (module_container->state.is_lock_free()) {
                throw std::runtime_error("module_contianer is no lock free.");
            }

            module_container->node = parser.ParseModule();
            module_container->state = ModuleContainer::ProcessState::PROCEED;
        } catch (parser::ParseError& err) {
            module_container->error_message = err.ErrorMessage();
            module_container->state = ModuleContainer::ProcessState::ERROR;
        } catch (std::exception& ex) {
            module_container->error_message = ex.what();
            module_container->state = ModuleContainer::ProcessState::ERROR;
        }

        IncreaseFinishedCount();
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

