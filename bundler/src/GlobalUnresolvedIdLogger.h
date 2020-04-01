//
// Created by Duzhong Chen on 2020/4/1.
//

#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <robin_hood.h>

#include "parser/SyntaxNodes.h"
#include "Utils.h"

namespace rocket_bundle {

    class GlobalUnresolvedIdLogger {
    public:
        std::mutex logger_mutex;
        robin_hood::unordered_set<UString> ids;

        inline void InsertByList(std::vector<std::shared_ptr<Identifier>> list) {
            std::lock_guard<std::mutex> lk(logger_mutex);
            for (auto& id : list){
                ids.insert(id->name);
            }
        }

    };

}
