//
// Created by Duzhong Chen on 2020/6/30.
//

#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

#include <string.h>
#include "jsheap.h"

#if defined(__APPLE__)
#define MALLOC_OVERHEAD  0
#else
#define MALLOC_OVERHEAD  8
#endif

#define FIND_0   0b10000000u
#define FIND_1   0b01000000u
#define FIND_2   0b00100000u
#define FIND_3   0b00010000u
#define FIND_4   0b00001000u
#define FIND_5   0b00000100u
#define FIND_6   0b00000010u
#define FIND_7   0b00000001u
#define FULL_BIT 0b11111111u

/* default memory allocation functions with memory limitation */
static inline size_t js_trace_malloc_usable_size(void *ptr)
{
#if defined(__APPLE__)
    return malloc_size(ptr);
#elif defined(_WIN32)
    return _msize(ptr);
#elif defined(EMSCRIPTEN)
    return 0;
#elif defined(__linux__)
    return malloc_usable_size(ptr);
#else
    /* change this to `return 0;` if compilation fails */
    return malloc_usable_size(ptr);
#endif
}

JsHeap* JsHeap_New() {
    JsHeap* result = (JsHeap*)JSRT_MALLOC(sizeof(JsHeap));
    memset(result, 0 , sizeof(JsHeap));
    result->allocated_size = 0u;
    memset(result->smo_heap, 0, JS_SMO_HEAP_SIZE);
    return result;
}

void FreeJsHeap(JsHeap* heap) {
    JSRT_FREE(heap->smo_heap);

    JSRT_FREE(heap);
}

static inline int FindAvailableIndexAndMark(uint8_t* used_mem_bits, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        uint8_t bit = used_mem_bits[i];

        if (bit == FULL_BIT) {
            continue;
        }

        int pos = -1;
        if ((bit & FIND_0) == 0) {
            pos = 0;
            used_mem_bits[i] |= FIND_0;
        } else if ((bit & FIND_1) == 0) {
            pos = 1;
            used_mem_bits[i] |= FIND_1;
        } else if ((bit & FIND_2) == 0) {
            pos = 2;
            used_mem_bits[i] |= FIND_2;
        } else if ((bit & FIND_3) == 0) {
            pos = 3;
            used_mem_bits[i] |= FIND_3;
        } else if ((bit & FIND_4) == 0) {
            pos = 4;
            used_mem_bits[i] |= FIND_4;
        } else if ((bit & FIND_5) == 0) {
            pos = 5;
            used_mem_bits[i] |= FIND_5;
        } else if ((bit & FIND_6) == 0) {
            pos = 6;
            used_mem_bits[i] |= FIND_6;
        } else if ((bit & FIND_7) == 0) {
            pos = 7;
            used_mem_bits[i] |= FIND_7;
        }

        if (pos >= 0) {
            return (int)i * 8 + pos;
        }
    }
    return -1;
}

uint8_t* JsHeap_Allocate(JsHeap* heap, uint64_t size) {
    if (size <= JS_MINIMAL_OBJ_SIZE) {  // find in smo
        int pos = FindAvailableIndexAndMark(heap->used_mem, JS_ALLOCATED_BIT);
        if (pos < 0) {
            return NULL;
        }
        heap->allocated_size += JS_MINIMAL_OBJ_SIZE;
        heap->malloc_count++;
        return heap->smo_heap + pos * JS_MINIMAL_OBJ_SIZE;
    }
    uint8_t* result = JSRT_MALLOC(size);
    if (result == NULL) {
        return NULL;
    }
    heap->allocated_size += js_trace_malloc_usable_size(result) + MALLOC_OVERHEAD;
    heap->malloc_count++;
    return result;
}

void JsHeap_Free(JsHeap* heap, void* ptr) {
    uint64_t uptr = (uint64_t)ptr;
    uint64_t smo_start = (uint64_t)heap->smo_heap;
    uint64_t smo_end = (uint64_t)(heap->smo_heap + JS_SMO_HEAP_SIZE);
    if (uptr >= smo_start && uptr < smo_end) {
        uint64_t ptr_pos = uptr - smo_start;
        uint64_t area = ptr_pos / JS_MINIMAL_OBJ_SIZE;
        uint64_t bit_index = area / 8;
        uint64_t bit_offset = area & 8;

        switch (bit_offset) {
            case 0u:
                heap->smo_heap[bit_index] &= ~FIND_0;
                break;

            case 1u:
                heap->smo_heap[bit_index] &= ~FIND_1;
                break;

            case 2u:
                heap->smo_heap[bit_index] &= ~FIND_2;
                break;

            case 3u:
                heap->smo_heap[bit_index] &= ~FIND_3;
                break;

            case 4u:
                heap->smo_heap[bit_index] &= ~FIND_4;
                break;

            case 5u:
                heap->smo_heap[bit_index] &= ~FIND_5;
                break;

            case 6u:
                heap->smo_heap[bit_index] &= ~FIND_6;
                break;

            case 7u:
                heap->smo_heap[bit_index] &= ~FIND_7;
                break;

            default:
                ; // ignore

        }
        heap->allocated_size -= JS_MINIMAL_OBJ_SIZE;
        heap->malloc_count++;
        return;
    }
    heap->allocated_size += js_trace_malloc_usable_size(ptr) + MALLOC_OVERHEAD;
    heap->malloc_count++;
    JSRT_FREE(ptr);
}
