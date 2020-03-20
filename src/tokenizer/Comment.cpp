//
// Created by Duzhong Chen on 2019/10/10.
//
#include <jemalloc/jemalloc.h>
#include "Comment.h"

void* Comment::operator new(std::size_t size) {
    return malloc(size);
}

void Comment::operator delete(void *chunk){
    free(chunk);
}
