//
// Created by Duzhong Chen on 2021/9/24.
//

#pragma once

#include <type_traits>
#include <vector>
#include "utils/Alloc.h"

namespace jetpack {
    class SyntaxNode;

    class AstContext {
    public:

        template<typename T, typename ...Args>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, T*>::type
        Alloc(Args && ...args) {
            void* space = alloc_.Alloc(sizeof(T));
            T* result = new (space) T(std::forward<Args>(args)...);
            nodes_.push_back(result);
            return result;
        }

        ~AstContext() noexcept;

    private:
        NoReleaseAllocator alloc_;
        std::vector<SyntaxNode*> nodes_;

    };


}
