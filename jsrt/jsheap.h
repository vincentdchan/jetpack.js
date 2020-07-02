//
// Created by Duzhong Chen on 2020/6/30.
//

#ifndef JSRT_JSHEAP_H
#define JSRT_JSHEAP_H

#include <stdlib.h>
#include <inttypes.h>

#define JSRT_MALLOC          malloc
#define JSRT_FREE            free

// small object
#ifndef JS_SMO_HEAP_SIZE
#define JS_SMO_HEAP_SIZE      4096
#endif

#define JS_MINIMAL_OBJ_SIZE   16

// JS_SMO_HEAP_SIZE / JS_MINIMAL_OBJ_SIZE / 8
#define JS_ALLOCATED_BIT      32

/**
 * Object's size in JSRT is dynamic,
 * minimal size of a Object in JSRT is 16.
 * variable such as boolean/number is small object,
 * which is regarded as SMO.
 * All SMOs are stored in smo_heap.
 */
typedef struct {
    uint64_t        allocated_size;
    uint8_t*        smo_heap;
    uint8_t         used_mem[JS_ALLOCATED_BIT];
    uint32_t        malloc_count;
} JsHeap;

JsHeap* JsHeap_New();
void FreeJsHeap(JsHeap* heap);

uint8_t* JsHeap_Allocate(JsHeap* heap, uint64_t size);
void JsHeap_Free(JsHeap* heap, void* ptr);

#endif //JSRT_JSHEAP_H
