//
// Created by Duzhong Chen on 2020/6/28.
//

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "jsrt.h"
#include "./cutils.h"

#define JSRT_HASHLIMIT      5
#define STATIC_BUFFER_SIZE  4096

#define CHECK_NOT_SMI(x) if (IS_SMI(x)) { \
        return JS_ERR_TYPE_MIS; \
    }

static uint8_t JSRT_StaticBuffer[STATIC_BUFFER_SIZE];

JsStaticVal *NewJsStaticVal(struct JSRT_CTX* ctx) {
    JsStaticVal *sv = (JsStaticVal*)malloc(sizeof(JsStaticVal));
    memset(sv, 0, sizeof(JsStaticVal));

    JsObject* J_undefined = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    J_undefined->flags = JS_UNDEFINED | JS_FLAGS_NO_GC;
    J_undefined->bool_val = 0;
    sv->J_undefined = PTR_TO_JS_VAL(J_undefined);

    JsObject* J_true = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    J_true->flags = JS_BOOL | JS_FLAGS_NO_GC;
    J_true->bool_val = TRUE;
    sv->J_true = PTR_TO_JS_VAL(J_true);

    JsObject* J_false = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    J_true->flags = JS_BOOL | JS_FLAGS_NO_GC;
    J_true->bool_val = FALSE;
    sv->J_false = PTR_TO_JS_VAL(J_false);

    return sv;
}

void FreeJsStaticVal(struct JSRT_CTX* ctx, JsStaticVal* jsv) {
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_undefined));
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_true));
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_false));
    JSRT_FREE(jsv);
}

JSRT_CTX* JSRT_NewCtx() {
    JSRT_CTX* ctx = (JSRT_CTX*)JSRT_MALLOC(sizeof(JSRT_CTX));

    srand(time(NULL));
    ctx->seed = (uint32_t)(rand());

    ctx->js_heap = JsHeap_New();

    ctx->js_static = NewJsStaticVal(ctx);

    return ctx;
}

void JSRT_FreeCtx(JSRT_CTX* ctx) {
    FreeJsStaticVal(ctx, ctx->js_static);
    FreeJsHeap(ctx->js_heap);

    JSRT_FREE(ctx);
}

JS_VAL JSRT_NewSmi(JSRT_CTX* ctx, int32_t value) {
    JS_VAL result = 0;
    result = (JS_VAL)value << 1;
    return result;
}

JS_VAL JSRT_NewStrUTF8(JSRT_CTX* ctx, const uint8_t* utf8) {
    int str_size = strlen((const char*)utf8);
    uint64_t size = sizeof(JsStdStr) + (str_size + 1) * sizeof(JS_CHAR_RAW);
    int* buffer = NULL;
    uint32_t buffer_size = 0;

    if (str_size < STATIC_BUFFER_SIZE / 4) {  // small str, use static area to convert
        memset(JSRT_StaticBuffer, 0, STATIC_BUFFER_SIZE);
        buffer = (int*)JSRT_StaticBuffer;
        buffer_size = STATIC_BUFFER_SIZE;
    } else {  // big str, allocate new space to convert
        buffer_size = (str_size + 1) * sizeof(JS_CHAR_RAW);
        buffer = (int*)JSRT_MALLOC(buffer_size);
        memset(buffer, 0, buffer_size);
    }

    const uint8_t* start_pos = utf8;
    int* buffer_ptr = buffer;
    while (1) {
        int distance = utf8 - start_pos;
        int max_len = str_size - distance;
        if (max_len < 1) {
            break;
        }
        int unicode = unicode_from_utf8(utf8, max_len, &utf8);
        if (unicode < 0) {
            if ((uint8_t*)buffer != JSRT_StaticBuffer) {  // indicate that buffer is on heap, free it
                JSRT_FREE(buffer);
            }
            return JS_INVALID_PTR;
        }
        (*buffer_ptr++) = unicode;
    }
    (*buffer_ptr) = 0;
    uint32_t unicode_size = buffer_ptr - buffer;

    JsStdStr* obj = (JsStdStr*)JSRT_MALLOC(size);

    obj->flags = JS_STR | JS_FLAGS_STD_STR;
    obj->str_size = unicode_size;
    obj->simple_hash = JS_INVALID_STR_HASH;
    for (uint32_t i = 0; i < unicode_size; i++) {  // copy buffer
        obj->str_val[i] = buffer[i];
    }
    obj->str_val[unicode_size] = 0;

    if ((uint8_t*)buffer != JSRT_StaticBuffer) {  // indicate that buffer is on heap, free it
        JSRT_FREE(buffer);
    }

    return PTR_TO_JS_VAL(obj);
}

int JSRT_StdStrToUTF8(JSRT_CTX* ctx, JS_VAL val, uint8_t* bytes, uint32_t size) {
    CHECK_NOT_SMI(val)

    uint64_t obj_ptr = JS_VAL_TO_PTR(val);

    if (JS_TYPE(obj_ptr) != JS_STR) {
        return JS_ERR_TYPE_MIS;
    }

    if (JS_STR_TYPE(obj_ptr) != 0) {
        return JS_ERR_TYPE_MIS;
    }

    JsStdStr *js_str = (JsStdStr*)obj_ptr;

    uint8_t *start_pos = bytes;

#define DISTANCE (bytes - start_pos)
    for (uint32_t i = 0 ; i < js_str->str_size; i++) {
        if (size - UTF8_CHAR_LEN_MAX < DISTANCE) {
            return JS_ERR_BUF_NOT_ENOUGH;
        }

        int unicode = js_str->str_val[i];
        int byte_size = unicode_to_utf8(bytes, unicode);

        bytes += byte_size;
    }

    return (int)DISTANCE;
}
#undef DISTANCE

static inline void __StdStrCalSimpleHashIfNotExist(JSRT_CTX* ctx, JsStdStr* std_str) {
    if (std_str->simple_hash == JS_INVALID_STR_HASH) {
        std_str->simple_hash = JSRT_SimpleHash(std_str->str_val, std_str->str_size, ctx->seed);
    }
}

JS_VAL JSRT_ValAdd(JSRT_CTX* ctx, JS_VAL val1, JS_VAL val2) {
    if (IS_SMI(val1) && IS_SMI(val2)) {
        int32_t smi1 = SMI_TO_I32(val1);
        int32_t smi2 = SMI_TO_I32(val2);

        int64_t result = (int64_t)smi1 + (int64_t)smi2;

        if (result > JSRT_MaxSmiValue || result < JSRT_MinSmiValue) {
            goto not_implemented;
        }

        return I32_TO_SMI(result);
    }

not_implemented:
    JSRT_Errno = JS_ERR_NOT_IMPLEMENTED;
    return JS_INVALID_PTR;
}

JS_VAL JSRT_StrEq(JSRT_CTX * ctx, JS_VAL val1, JS_VAL val2) {
    uint64_t str1 = JS_VAL_TO_PTR(val1);
    uint64_t str2 = JS_VAL_TO_PTR(val2);

    if (JS_STR_TYPE(val1) == JS_FLAGS_STD_STR && JS_STR_TYPE(val2) == JS_FLAGS_STD_STR) {

        JsStdStr *std_str1 = (JsStdStr*)str1;
        JsStdStr *std_str2 = (JsStdStr*) str2;

        if (std_str1->str_size != std_str2->str_size) {
            return ctx->js_static->J_false;
        }

        __StdStrCalSimpleHashIfNotExist(ctx, std_str1);
        __StdStrCalSimpleHashIfNotExist(ctx, std_str2);

        // if hash is different, it's different
        if (std_str1->simple_hash != std_str2->simple_hash) {
            return ctx->js_static->J_false;
        }

        for (uint32_t i = 0; i < std_str1->str_size; i++) {
            if (std_str1->str_val[i] != std_str2->str_val[i]) {
                return ctx->js_static->J_false;
            }
        }

        return ctx->js_static->J_true;
    }

    JSRT_Errno = JS_ERR_NOT_IMPLEMENTED;
    return JS_INVALID_PTR;
}

// copy from lua source code
uint32_t JSRT_SimpleHash(const JS_CHAR_RAW *str, size_t l, uint32_t seed) {
    uint32_t h = seed ^ (uint32_t)(l);
    size_t step = (l >> JSRT_HASHLIMIT) + 1;
    for (; l >= step; l -= step)
        h ^= ((h<<5) + (h>>2) + str[l - 1]);
    return h;
}
