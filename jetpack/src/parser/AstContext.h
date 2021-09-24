//
// Created by Duzhong Chen on 2021/9/24.
//

#pragma once

#include "utils/Alloc.h"

namespace jetpack {
    class SyntaxNode;

    class AstContext {
    public:
        template<typename T, typename ...Args>
        typename std::enable_if<std::is_base_of<SyntaxNode, T>::value, T*>::type
        Alloc(Args && ...args) {
            void* space = alloc_.Alloc(sizeof(T));
            return new (space) T(std::forward<Args>(args)...);
        }

    private:
        NoReleaseAllocator alloc_;

    };

}
