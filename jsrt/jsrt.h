//
// Created by Duzhong Chen on 2020/6/28.
//

#ifndef JSRT
#define JSRT

#include <stdint.h>
#include <stdlib.h>
#include <locale.h>

#include "jsheap.h"

#define JS_DOUBLE_RAW        double
#define JS_CHAR_RAW          unsigned int
#define JS_FLAGS_RAW         uint32_t
#define JS_RC_RAW            uint32_t
#define JS_INVALID_PTR       ((uint64_t)-1)
#define JS_INVALID_STR_HASH  ((uint32_t)-1)

typedef enum : unsigned int {
    JS_UNDEFINED = 0u,
    JS_NUM       = 1u,
    JS_STR       = 2u,
    JS_BOOL      = 3u,
    JS_ARR       = 4u,
    JS_OBJ       = 5u,
    JS_FUN       = 6u
} JS_TYPE;

typedef enum : int {
    JS_ERR_TYPE_MIS        = -2,
    JS_ERR_BUF_NOT_ENOUGH  = -3,
    JS_ERR_NOT_IMPLEMENTED = -4,
} JS_ERR;

#define JS_VAL uint64_t

#define IS_SMI(x) (((JS_VAL)x & 1u) == 0)
#define JS_VAL_TO_PTR(x) ((JS_VAL)x & (~1u))
#define PTR_TO_JS_VAL(x) ((JS_VAL)x | 1u)
#define SMI_TO_I32(x) ((int32_t)((JS_VAL)x >> 1u))
#define I32_TO_SMI(x) ((JS_VAL)x << 1u)

static const int64_t JSRT_MaxSmiValue = 0x7FFFFFFF;
static const int64_t JSRT_MinSmiValue = 0xFFFFFFFF;

#define JS_OBJ_FLAGS_TYPE    0b00000000000000000000000011111111u

#define JS_STR_FLAGS_TYPE    0b00000000000000000000011100000000u
#define JS_FLAGS_STD_STR     0b00000000000000000000000000000000u
#define JS_FLAGS_CONCAT_STR  0b00000000000000000000000100000000u

#define JS_GC_FLAGS_TYPE     0b11110000000000000000000000000000u
#define JS_FLAGS_NO_GC       0b00010000000000000000000000000000u

struct JSRT_CTX;

enum JS_STR_TYPE {
    JS_STR_STD = 0,
    JS_STR_CONCATED = 1,
};

typedef struct {
    JS_FLAGS_RAW flags;
    JS_RC_RAW rc;
    uint32_t      str_size;
    uint32_t      simple_hash;
    JS_CHAR_RAW   str_val[0];
} JsStdStr;

#define JS_STR_TYPE(x) (((JsStdStr*)x)->flags & JS_STR_FLAGS_TYPE)

JsStdStr NewJsStrByCstr(struct JSRT_CTX* ctx, const char* bytes);

typedef struct {
    JS_VAL J_undefined;
    JS_VAL J_true;
    JS_VAL J_false;
} JsStaticVal;

JsStaticVal *NewJsStaticVal(struct JSRT_CTX* ctx);
void FreeJsStaticVal(struct JSRT_CTX* ctx, JsStaticVal* jsv);

typedef struct {
    JS_FLAGS_RAW flags;
    JS_RC_RAW rc;
    union {
        int32_t       bool_val;
        JS_DOUBLE_RAW num_val;
        JsStdStr*     std_str_val;
    };
} JsObject;

#define JS_TYPE(x) (((JsObject*)x)->flags & JS_OBJ_FLAGS_TYPE)

typedef struct JSRT_CTX {
    JsStaticVal *js_static;
    JsHeap* js_heap;
    uint32_t seed;
} JSRT_CTX;

JSRT_CTX* JSRT_NewCtx();
void JSRT_FreeCtx(JSRT_CTX* ctx);

JS_VAL JSRT_NewSmi(JSRT_CTX* ctx, int32_t value);
JS_VAL JSRT_NewStrUTF8(JSRT_CTX* ctx, const uint8_t* utf8);
int JSRT_StdStrToUTF8(JSRT_CTX* ctx, JS_VAL val, uint8_t* bytes, uint32_t size);

JS_VAL JSRT_ValAdd(JSRT_CTX* ctx, JS_VAL val1, JS_VAL val2);
JS_VAL JSRT_StrEq(JSRT_CTX* ctx, JS_VAL val1, JS_VAL val2);

uint32_t JSRT_SimpleHash(const JS_CHAR_RAW *str, size_t l, uint32_t seed);

static int JSRT_Errno;

#endif //JSRT
