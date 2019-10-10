//
// Created by Duzhong Chen on 2019/10/10.
//

#include <jemalloc/jemalloc.h>
#include "base_nodes.h"

void* SyntaxNode::operator new(std::size_t count) {
    return malloc(count);
}

void SyntaxNode::operator delete(void *ptr){
    free(ptr);
}
