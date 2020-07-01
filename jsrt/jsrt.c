//
// Created by Duzhong Chen on 2020/6/28.
//

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "jsrt.h"
#include "./cutils.h"

#define JSRT_HASHLIMIT      5
#define STATIC_BUFFER_SIZE  4096
#define STATIC_TRUE         (ctx->js_static->J_true)
#define STATIC_FALSE        (ctx->js_static->J_false)

#define CHECK_NOT_SMI(x) if (IS_SMI(x)) { \
        return JS_ERR_TYPE_MIS; \
    }

static uint8_t JSRT_StaticBuffer[STATIC_BUFFER_SIZE];

static BOOL JsStrEq(JSRT_CTX* ctx, JsStrCommon* val1, JsStrCommon* val2);

static BOOL JsArrSetVal(JSRT_CTX* ctx, JsArray* arr, uint32_t index, JS_VAL val);

static BOOL JsStdObjectGetVal(JSRT_CTX* ctx, JsStdObject* std_obj, JsStrCommon* key, JS_VAL* result);

static void ReleaseStr(JSRT_CTX* ctx, JsStrCommon* str);
static void ReleaseArray(JSRT_CTX* ctx, JsArray* arr);
static void ReleaseStdObject(JSRT_CTX* ctx, JsStdObject* std_obj);
static void ReleaseFunction(JSRT_CTX* ctx, JsFunction* fun);
static void ReleaseClosure(JSRT_CTX* ctx, JsFunClosureEnv* closure);
static inline JsObjectHead* RetainObj(JSRT_CTX* ctx, JsObjectHead* obj);
static inline void ReleaseObj(JSRT_CTX* ctx, JsObjectHead* obj);

static inline uint32_t JsStrCalSimpleHashIfNotExist(JSRT_CTX* ctx, JsStrCommon * str) {
    if (str->simple_hash == JS_INVALID_STR_HASH) {
        JS_TY_FLAGS str_type = JS_STR_TYPE(str);
        if (str_type == JS_FLAGS_STD_STR) {
            JsStdStr* std_str = (JsStdStr*)str;
            std_str->simple_hash = JSRT_SimpleHash(std_str->str_val, std_str->str_size, ctx->seed);
        } else {
            // TODO: flat
        }
        return str->simple_hash;
    }
    return str->simple_hash;
}


JS_VAL JSRT_NewUndefined(JSRT_CTX* ctx, JS_TY_FLAGS flags) {
    JsObject* J_undefined = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    J_undefined->flags = JS_UNDEFINED | flags;
    J_undefined->bool_val = 0;
    J_undefined->rc = 1;
    return PTR_TO_JS_VAL(J_undefined);
}

JS_VAL JSRT_NewBool(JSRT_CTX* ctx, JS_TY_FLAGS flags, int val) {
    JsObject* obj = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    obj->flags = JS_BOOL | flags;
    obj->bool_val = val;
    obj->rc = 1;
    return PTR_TO_JS_VAL(obj);
}

JsStaticVal *NewJsStaticVal(struct JSRT_CTX* ctx) {
    JsStaticVal *sv = (JsStaticVal*)malloc(sizeof(JsStaticVal));
    memset(sv, 0, sizeof(JsStaticVal));

    sv->J_undefined = JSRT_NewUndefined(ctx, JS_FLAGS_STATIC_VAL);
    sv->J_true = JSRT_NewBool(ctx, JS_FLAGS_STATIC_VAL, TRUE);
    sv->J_false = JSRT_NewBool(ctx, JS_FLAGS_STATIC_VAL, FALSE);

#define DEF(NAME, VALUE) \
    sv->S_##NAME = JSRT_NewStrUTF8Auto(ctx, JS_FLAGS_STATIC_VAL, VALUE);

#include "static-str.h"

#undef DEF

    return sv;
}

/**
 * DO NOT use JSRT_Release, use JsHeap_Free directly
 * to clear the heap
 */
void FreeJsStaticVal(struct JSRT_CTX* ctx, JsStaticVal* jsv) {
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_undefined));
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_true));
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->J_false));

#define DEF(NAME, VALUE) \
    JsHeap_Free(ctx->js_heap, (void*)JS_VAL_TO_PTR(jsv->S_##NAME));

#include "static-str.h"

#undef DEF

    JSRT_FREE(jsv);
}

JSRT_CTX* JSRT_NewCtx() {
    JSRT_CTX* ctx = (JSRT_CTX*)JSRT_MALLOC(sizeof(JSRT_CTX));

    srand(time(NULL));
    ctx->seed = (uint32_t)(rand());

    ctx->js_heap = JsHeap_New();

    ctx->js_static = NewJsStaticVal(ctx);

    ctx->stack_start = ctx->stack_end = NULL;

    return ctx;
}

void JSRT_FreeCtx(JSRT_CTX* ctx) {
    FreeJsStaticVal(ctx, ctx->js_static);
    FreeJsHeap(ctx->js_heap);

    JSRT_FREE(ctx);
}

JS_VAL JSRT_NewI32(JSRT_CTX* ctx, int32_t value) {
    return I32_TO_SMI(value);
}

static inline BOOL IsDoubleAInt(double value) {
    return round(value) == value;
}

static inline JsObject* NewReallyDouble(JSRT_CTX* ctx, double value) {
    JsObject* obj = (JsObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsObject));
    obj->flags = JS_NUM;
    obj->rc = 1;
    obj->num_val = value;
    return obj;
}

JS_VAL JSRT_NewDouble(JSRT_CTX* ctx, double value) {
    if (IsDoubleAInt(value)) {
        int64_t i64v = (int64_t)value;
        if (i64v >= JSRT_MinSmiValue && i64v <= JSRT_MaxSmiValue) { // is smi
            return JSRT_NewI32(ctx, (int32_t)i64v);
        }
    }
    JsObject* obj = NewReallyDouble(ctx, value);
    return PTR_TO_JS_VAL(obj);
}

JS_VAL JSRT_NewNaN(JSRT_CTX* ctx) {
    JsObject* obj = NewReallyDouble(ctx, NAN);
    return PTR_TO_JS_VAL(obj);
}

JS_VAL JSRT_NewStrUTF8Auto(JSRT_CTX* ctx, JS_TY_FLAGS flags, const char* utf8) {
    uint32_t str_size = strlen(utf8);
    return JSRT_NewStrUTF8(ctx, flags, (const uint8_t*)utf8, str_size);
}

JS_VAL JSRT_NewStrUTF8(JSRT_CTX* ctx, JS_TY_FLAGS flags, const uint8_t* utf8, uint32_t str_size) {
    uint64_t size = sizeof(JsStdStr) + (str_size + 1) * sizeof(JS_TY_CHAR);
    int* buffer = NULL;
    uint32_t buffer_size = 0;

    if (str_size < STATIC_BUFFER_SIZE / 4) {  // small str, use static area to convert
        memset(JSRT_StaticBuffer, 0, STATIC_BUFFER_SIZE);
        buffer = (int*)JSRT_StaticBuffer;
        buffer_size = STATIC_BUFFER_SIZE;
    } else {  // big str, allocate new space to convert
        buffer_size = (str_size + 1) * sizeof(JS_TY_CHAR);
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

    JsStdStr* obj = (JsStdStr*)JsHeap_Allocate(ctx->js_heap, size);

    obj->flags = JS_STR | JS_FLAGS_STD_STR | flags;
    obj->str_size = unicode_size;
    obj->simple_hash = JS_INVALID_STR_HASH;
    obj->rc = 1;
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

    JS_PTR obj_ptr = JS_VAL_TO_PTR(val);

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

uint16_t JsStrDepth(JsStrCommon* str) {
    if ((str->flags & JS_STR_FLAGS_TYPE) == JS_FLAGS_STD_STR) {
        return 1;
    }

    JsConcatStr *concat_str = (JsConcatStr*)str;
    return concat_str->depth;
}

uint16_t JsStrNodeSize(JsStrCommon* str) {
    if ((str->flags & JS_STR_FLAGS_TYPE) == JS_FLAGS_STD_STR) {
        return 1;
    }

    JsConcatStr *concat_str = (JsConcatStr*)str;
    return concat_str->node_size;
}

static inline JsStrCommon* NormalizeConcatStr(JSRT_CTX* ctx, JsConcatStr* input) {
    return (JsStrCommon*)input;
}

JsStrCommon* JSRT_ConcatStr(JSRT_CTX* ctx, JsStrCommon* left, JsStrCommon* right) {
    uint32_t bytes_size = sizeof(JsConcatStr);
    JsConcatStr* str = (JsConcatStr*)malloc(bytes_size);
    memset(str, 0, bytes_size);

    str->flags = JS_STR | JS_FLAGS_CONCAT_STR;
    str->rc = 1;
    str->str_size = left->str_size + right->str_size;
    str->simple_hash = JS_INVALID_STR_HASH;

    uint32_t left_depth = JsStrDepth(left);
    uint32_t right_depth = JsStrDepth(right);
    str->depth = (uint16_t)(max_uint32(left_depth, right_depth) + 1);

    uint32_t left_node_size = JsStrNodeSize(left);
    uint32_t right_node_size = JsStrNodeSize(right);
    str->node_size = left_node_size + right_node_size;

    // increase counter
    RetainObj(ctx, (JsObjectHead*)left);
    RetainObj(ctx, (JsObjectHead*)right);

    str->left = left;
    str->right = right;

    return NormalizeConcatStr(ctx, str);
}

static void ReleaseStr(JSRT_CTX* ctx, JsStrCommon* str) {
    if ((str->flags & JS_STR_FLAGS_TYPE) == JS_FLAGS_STD_STR) {
        JsHeap_Free(ctx->js_heap, str);
        return;
    }

    JsConcatStr* concat_str = (JsConcatStr*)str;

    ReleaseObj(ctx, (JsObjectHead*)concat_str->left);
    ReleaseObj(ctx, (JsObjectHead*)concat_str->right);

    JsHeap_Free(ctx->js_heap, str);
}

JS_VAL JSRT_NewArray(JSRT_CTX* ctx, JS_TY_FLAGS flags, uint32_t cap) {
    JsArray* arr = (JsArray*)JsHeap_Allocate(ctx->js_heap, sizeof(JsArray));
    memset(arr, 0 , sizeof(JsArray));

    arr->flags = JS_ARR | flags;
    arr->rc = 1;
    arr->size = 0;
    arr->capacity = cap;

    uint32_t data_bytes_size = sizeof(JS_VAL) * arr->capacity;
    arr->data = (JS_VAL*)JsHeap_Allocate(ctx->js_heap, data_bytes_size);
    memset(arr->data, 0, data_bytes_size);

    return PTR_TO_JS_VAL(arr);
}

static void ReleaseArray(JSRT_CTX* ctx, JsArray* arr) {
    for (uint32_t i = 0; i < arr->size; i++) {
        JSRT_Release(ctx, arr->data[i]);
    }
    JsHeap_Free(ctx->js_heap, arr->data);
    JsHeap_Free(ctx->js_heap, arr);
}

JS_VAL JSRT_NewStdObject(JSRT_CTX* ctx, JS_TY_FLAGS flags) {
    JsStdObject* std_obj = (JsStdObject*)JsHeap_Allocate(ctx->js_heap, sizeof(JsStdObject));

    std_obj->flags = JS_OBJ | flags;
    std_obj->rc = 1;
    std_obj->prop_size = 0u;
    std_obj->bucket_cap = JS_OBJ_BUCKET_CAP;
    std_obj->start_prop = NULL;
    std_obj->end_prop = NULL;
    std_obj->bucket = NULL;

    return PTR_TO_JS_VAL(std_obj);
}

JS_VAL JSRT_NewFunction(JSRT_CTX* ctx, JSRT_RawFunction raw, JsFunClosureEnv* env) {
    JsFunction* fun = (JsFunction*)JsHeap_Allocate(ctx->js_heap, sizeof(JsFunction));

    fun->flags = JS_FUN;
    fun->rc = 1;
    fun->raw_function = raw;
    fun->closure_env = env;

    return PTR_TO_JS_VAL(fun);
}

JsVirtualStack* JSRT_NewVirtualStack(JSRT_CTX* ctx, const char* name, JS_VAL arg, uint32_t var_size) {
    JsVirtualStack* st = (JsVirtualStack*)JsHeap_Allocate(ctx->js_heap, sizeof(JsVirtualStack));
    memset(st, 0, sizeof(JsVirtualStack));

    st->name = name;
    st->var_size = var_size;
    st->argument = arg;

    return st;
}

JsVirtualStack* JSRT_NewVirtualStack2(JSRT_CTX* ctx, const char* name, JS_VAL arg, uint32_t var_size, JsVirtualStack* prev) {
    JsVirtualStack* st = JSRT_NewVirtualStack(ctx, name, var_size, arg);

    st->prev = prev;
    st->height = prev->height + 1;
    prev->next = st;

    return st;
}

void JSRT_ReleaseVirtualStack(struct JSRT_CTX*ctx, JsVirtualStack* st) {
    // release temp var and stack var
    for (uint32_t i = 0; i < JS_STACK_TMP_SIZE; i++) {
        if (st->temp_val[i] != 0) {
            JSRT_Release(ctx, st->temp_val[i]);
        }
    }
    for (uint32_t i = 0; i < st->var_size; i++) {
        JSRT_Release(ctx, st->variable[i]);
    }
    // release arguments
    JSRT_Release(ctx, st->argument);
    JsHeap_Free(ctx->js_heap, st);
}

static void ReleaseFunction(JSRT_CTX* ctx, JsFunction* fun) {
    if (fun->closure_env != NULL) {
        ReleaseClosure(ctx, fun->closure_env);
    }

    JsHeap_Free(ctx->js_heap, fun);
}


static void ReleaseClosure(JSRT_CTX* ctx, JsFunClosureEnv* closure) {
    for (uint32_t i = 0; i < closure->val_size; i++) {
        JSRT_Release(ctx, closure->data[i]);
    }

    JsHeap_Free(ctx->js_heap, closure);
}

/**
 * Clean jobs:
 * 1. decrease ref of (key, value)
 * 2. clean all the bucket
 * 3. clean the bucket array
 * 4. free std_obj itself
 *
 * start_propt* -> prop1
 *                   |
 *                 prop2
 *                   |
 * end_prop*    -> prop3
 *
 * bucket** ->
 *   [0] NULL
 *   [1] -> bucket1
 *   [2] NULL
 *   [3] -> bucket1 -> bucket2 -> bucket3
 */
static void ReleaseStdObject(JSRT_CTX* ctx, JsStdObject* std_obj) {
    JsStdObjectProp* ptr = std_obj->start_prop;
    while (ptr != std_obj->end_prop) {
        ReleaseObj(ctx, (JsObjectHead*)ptr->key);
        JSRT_Release(ctx, ptr->value);
        ptr = ptr->next;
    }

    if (std_obj->bucket != NULL) {
        // free all the buckets on the list
        for (uint32_t i = 0; i < std_obj->bucket_cap; i++) {
            JsStdObjectPropBucket* bucket_ptr = std_obj->bucket[i].next;

            while (bucket_ptr != NULL) {
                JsStdObjectPropBucket* next = bucket_ptr->next;

                JsHeap_Free(ctx->js_heap, bucket_ptr);

                bucket_ptr = next;
            }
        }
        JsHeap_Free(ctx->js_heap, std_obj->bucket);
    }

    JsHeap_Free(ctx->js_heap, std_obj);
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

JS_VAL JSRT_StrictEq(JSRT_CTX* ctx, JS_VAL val1, JS_VAL val2) {
    if (IS_SMI(val1) && IS_SMI(val2)) {
        if (SMI_TO_I32(val1) == SMI_TO_I32(val2)) {
            return STATIC_TRUE;
        } else {
            return STATIC_FALSE;
        }
    }

    JS_PTR p1 = JS_VAL_TO_PTR(val1);
    JS_PTR p2 = JS_VAL_TO_PTR(val2);

    JS_TY_FLAGS t1 = PTR_TYPE(p1);
    JS_TY_FLAGS t2 = PTR_TYPE(p2);

    BOOL result;
    if (t1 == JS_STR && t2 == JS_STR) {
        JsStrCommon *str1 = (JsStrCommon *) p1;
        JsStrCommon *str2 = (JsStrCommon *) p2;
        result = JsStrEq(ctx, str1, str2);
    } else if (t1 == JS_NUM && t2 == JS_NUM) {
        result = (((JsObject*)p1)->num_val == ((JsObject*)p2)->num_val);
    } else {
        result = (p1 == p2);
    }

    return result ? STATIC_TRUE : STATIC_FALSE;
}

static inline BOOL JsArrGiveEnoughCap(JSRT_CTX* ctx, JsArray* arr, uint32_t index) {
    uint32_t expect_cap = arr->capacity;

    do {
        expect_cap = expect_cap * 2;
    } while (expect_cap <= index);


    uint32_t expect_size = expect_cap * sizeof(JS_VAL);
    JS_VAL* new_space = (JS_VAL*)JsHeap_Allocate(ctx->js_heap, expect_size);
    memset(new_space, 0, expect_size);

    memcpy(new_space, arr->data, arr->size * sizeof(JS_VAL));

    JsHeap_Free(ctx->js_heap, arr->data);
    arr->data = new_space;
    arr->capacity = expect_cap;
    return TRUE;
}

static BOOL JsArrSetVal(JSRT_CTX* ctx, JsArray* arr, uint32_t index, JS_VAL val) {
    if (index < arr->size) {
        JSRT_Release(ctx, arr->data[index]);
        arr->data[index] = val;
        JSRT_Retain(ctx, val);
        return TRUE;
    }

    if (index >= arr->capacity) {
        if (!JsArrGiveEnoughCap(ctx, arr, index)) {
            return FALSE;
        }
    }

    for (uint32_t i = arr->size; i <= index; i++) {
        arr->data[i] = ctx->js_static->J_undefined;
    }

    arr->data[index] = val;
    JSRT_Retain(ctx, val);

    arr->size = index + 1;

    return TRUE;
}

static BOOL JsStrEq(JSRT_CTX * ctx, JsStrCommon* val1, JsStrCommon* val2) {
    if (val1 == val2) {
        return TRUE;
    } else if (JS_STR_TYPE(val1) == JS_FLAGS_STD_STR && JS_STR_TYPE(val2) == JS_FLAGS_STD_STR) {

        JsStdStr *std_str1 = (JsStdStr*)val1;
        JsStdStr *std_str2 = (JsStdStr*)val2;

        if (std_str1->str_size != std_str2->str_size) {
            return TRUE;
        }

        uint32_t hash1 = JsStrCalSimpleHashIfNotExist(ctx, (JsStrCommon*)std_str1);
        uint32_t hash2 = JsStrCalSimpleHashIfNotExist(ctx, (JsStrCommon*)std_str2);

        // if hash is different, it's different
        if (hash1 != hash2) {
            return FALSE;
        }

        for (uint32_t i = 0; i < std_str1->str_size; i++) {
            if (std_str1->str_val[i] != std_str2->str_val[i]) {
                return FALSE;
            }
        }

        return TRUE;
    }

    return FALSE;
}

static BOOL JsStdObjectGetVal(JSRT_CTX* ctx, JsStdObject* std_obj, JsStrCommon* key, JS_VAL* result) {
    if (std_obj->bucket != NULL) {
        uint32_t find_hash = JsStrCalSimpleHashIfNotExist(ctx, key);
        uint32_t mod_hash = find_hash % std_obj->bucket_cap;

        JsStdObjectPropBucket* bucket_ptr = std_obj->bucket + mod_hash;
        if (bucket_ptr == NULL) {
            return FALSE;
        }

        do {
            if (JsStrEq(ctx, bucket_ptr->prop->key, key)) {
                *result = bucket_ptr->prop->value;
                return TRUE;
            }
            bucket_ptr = bucket_ptr->next;
        } while (bucket_ptr != NULL);

        return FALSE;
    }
    JsStdObjectProp* prop_ptr = std_obj->start_prop;
    while (prop_ptr != std_obj -> end_prop) {
        if (JsStrEq(ctx, prop_ptr->key, key)) {
            *result = prop_ptr->value;
            return TRUE;
        }
        prop_ptr = prop_ptr->next;
    }
    return FALSE;
}

// copy from lua source code
uint32_t JSRT_SimpleHash(const JS_TY_CHAR *str, size_t l, uint32_t seed) {
    uint32_t h = seed ^ (uint32_t)(l);
    size_t step = (l >> JSRT_HASHLIMIT) + 1;
    for (; l >= step; l -= step)
        h ^= ((h<<5) + (h>>2) + str[l - 1]);
    return h;
}

JS_VAL JSRT_TypeOf(JSRT_CTX* ctx, JS_VAL val) {
    if (IS_SMI(val)) {
        return ctx->js_static->S_number;
    }
    JS_PTR ptr = JS_VAL_TO_PTR(val);
    JS_TY_FLAGS ty = PTR_TYPE(ptr);

    switch (ty) {
        case JS_UNDEFINED:
            return ctx->js_static->S_undefined;

        case JS_NUM:
            return ctx->js_static->S_number;

        case JS_STR:
            return ctx->js_static->S_string;

        case JS_BOOL:
            return ctx->js_static->S_boolean;

        case JS_ARR:
        case JS_OBJ:
            return ctx->js_static->S_object;

        case JS_FUN:
            return ctx->js_static->S_function;

        default:
            return JS_INVALID_PTR;

    }
}

JS_TY_RC JSRT_GetRetainCount(JS_VAL val) {
    if (IS_SMI(val)) {
        return 0;
    }
    JsObjectHead* obj = (JsObjectHead*)JS_VAL_TO_PTR(val);
    return obj->rc;
}

JS_VAL JSRT_Retain(JSRT_CTX* ctx, JS_VAL val) {
    if (IS_SMI(val)) {
        return val;
    }
    JsObjectHead* obj = (JsObjectHead*)JS_VAL_TO_PTR(val);
    RetainObj(ctx, obj);
    return val;
}

void JSRT_Release(JSRT_CTX* ctx, JS_VAL val) {
    if (IS_SMI(val)) {
        return;
    }
    JsObjectHead* obj = (JsObjectHead*)JS_VAL_TO_PTR(val);
    ReleaseObj(ctx, obj);
}

static inline JsObjectHead* RetainObj(JSRT_CTX* ctx, JsObjectHead* obj) {
    obj->rc++;
    return obj;
}

static inline void ReleaseObj(JSRT_CTX* ctx, JsObjectHead* obj) {
    if (--obj->rc == 0) {
        if (likely(obj->flags & JS_FLAGS_STATIC_VAL)) {
            return;
        }
        JS_TY_FLAGS obj_type = obj->flags & JS_OBJ_FLAGS_TYPE;
        switch (obj_type) {
            case JS_STR:
                ReleaseStr(ctx, (JsStrCommon*)obj);
                break;

            case JS_ARR:
                ReleaseArray(ctx, (JsArray*)obj);
                break;

            case JS_OBJ:
                ReleaseStdObject(ctx, (JsStdObject*) obj);
                break;

            case JS_FUN:
                ReleaseFunction(ctx, (JsFunction*)obj);
                break;

            default:
                JsHeap_Free(ctx->js_heap, obj);

        }
    }
}
