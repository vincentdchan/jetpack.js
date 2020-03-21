//
// Created by Duzhong Chen on 2020/3/20.
//

#include "./ModuleResolver.h"
#include "./Utils.h"
#include "./Error.h"
#include <artery.h>
#include <parser/ParserCommon.h>

namespace rocket_bundle {
    using parser::ParserContext;
    using parser::Parser;

    void ModuleResolver::BeginFromEntry(std::string path) {
        auto thread_pool_size = std::thread::hardware_concurrency();
        thread_pool_ = std::make_unique<ThreadPool>(thread_pool_size);

        auto fut = thread_pool_->enqueue([this, &path] {
            ParseFileFromPath(path);
        });

        fut.wait();
    }

    void ModuleResolver::ParseFileFromPath(const std::string &path) {
        if (!IsFileExist(path)) {
            throw Error("file not exist: " + path);
        }
        auto mf = std::make_shared<ModuleFile>();
        mf->module_resolver = shared_from_this();
        mf->path = path;

        {
            std::lock_guard<std::mutex> guard(map_mutex_);
            if (modules_map_.find(path) != modules_map_.end()) {
                return;  // exists;
            }
            modules_map_[path] = mf;
        }

        ParseFile(mf);
    }

    void ModuleResolver::ParseFile(Sp<ModuleFile> mf) {
        ParserContext::Config config = ParserContext::Config::Default();
        auto src = std::make_shared<UString>();
        (*src) = Artery::ReadFileStream(mf->path);
        auto ctx = std::make_shared<ParserContext>(src, config);
        Parser parser(ctx);

        std::vector<std::future<void>> futures;

        parser.OnImportDeclarationCreated([this, &futures] (const Sp<ImportDeclaration>& node) {
            auto source_path = parser_utils::To_UTF8(node->source->raw);

            std::shared_ptr<ModuleFile> mf;
            {
                std::lock_guard<std::mutex> guard(map_mutex_);
                if (modules_map_.find(source_path) != modules_map_.end()) return;  // exists

                mf = std::make_shared<ModuleFile>();
                mf->module_resolver = shared_from_this();
                mf->path = std::move(source_path);
                modules_map_[mf->path] = mf;
            }

            futures.push_back(thread_pool_->enqueue([this, mf] {
                ParseFile(mf);
            }));
        });

        mf->ast = parser.ParseModule();

        for (auto& fut : futures) {
            fut.wait();
        }
    }

}
