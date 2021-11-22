#include <node_api.h>
#include <cstdlib>
#include <cassert>

#include "./node_helper.hpp"
#include "SimpleAPI.h"

#define ENTRY_PATH_MAX 512

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_enumerable, 0 }

#define NAPI_CALL(env, call)                                      \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((env), &error_info);               \
      bool is_pending;                                            \
      napi_is_exception_pending((env), &is_pending);              \
      if (!is_pending) {                                          \
        const char* message = (error_info->error_message == NULL) \
            ? "empty error message"                               \
            : error_info->error_message;                          \
        napi_throw_error((env), NULL, message);                   \
        return NULL;                                              \
      }                                                           \
    }                                                             \
  } while(0)

/// bundleFile({
///   entry: '',
///
/// })
static napi_value bundle_file(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  node_args<2> argv;
  assert(argv.load(env, info) == napi_ok);

  std::string path = get_object_prop<std::string>(env, argv[0], "entry");
  if (path.empty()) {
    napi_throw_type_error(env, nullptr, "entry is not proviede");
    return 0;
  }

  JetpackFlags flags;

  if (argv.size() >= 2) {
    bool jsx = get_object_prop_or<bool>(env, argv[1], "jsx", true);
    if (jsx) {
      flags |= JETPACK_JSX;
    }
    bool minify = get_object_prop_or<bool>(env, argv[1], "minify", false);
    if (minify) {
      flags |= JETPACK_MINIFY;
    }
    bool sourcemap = get_object_prop_or<bool>(env, argv[1], "sourcemap", false);
    if (sourcemap) {
      flags |= JETPACK_SOURCEMAP;
    }
    bool trace_file = get_object_prop_or<bool>(env, argv[1], "traceFile", true);
    if (trace_file) {
      flags |= JETPACK_TRACE_FILE;
    }
  } else {
    // default
    flags |= JETPACK_JSX;
    flags |= JETPACK_TRACE_FILE;
  }

  jetpack_bundle_module(path.c_str(), "bundle.js", flags, nullptr);

  return 0;
}

static napi_value handle_command_line(napi_env env, napi_callback_info info) {
  int rt = 3;
  napi_status status;

  node_args<1> argv;
  NAPI_CALL(env, argv.load(env, info));

  bool is_arr = node_is_array(env, argv[0]);
  if (!is_arr) {
    napi_throw_type_error(env, NULL, "first arguments should be an array");
    return 0;
  }

  uint32_t arr_len = 0;
  NAPI_CALL(env, napi_get_array_length(env, argv[0], &arr_len));

  char** cmd_argv = (char**)malloc(sizeof(char*) * arr_len);

#define STR_BUFFER_SIZE 1024
  for (uint32_t i = 0; i < arr_len; i++) {
    napi_value result;
    NAPI_CALL(env, napi_get_element(env, argv[0], i, &result));

    char* str_buffer = (char*)::malloc(STR_BUFFER_SIZE);
    ::memset(str_buffer, 0, STR_BUFFER_SIZE);
    size_t str_size;
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str_buffer, 1024, &str_size));

    cmd_argv[i] = str_buffer;
  }

  rt = jetpack_handle_command_line(arr_len, cmd_argv);

failed:
  for (uint32_t i = 0; i < arr_len; i++) {
    ::free(cmd_argv[i]);
  }
  ::free(cmd_argv);

  return to_node_value(env, rt);
}

static napi_status SetCallbackProp(napi_env env, napi_value exports, const char* key, napi_callback cb) {
  napi_status status;

  napi_property_descriptor desc = DECLARE_NAPI_METHOD(key, cb);
  status = napi_define_properties(env, exports, 1, &desc);

  return status;
}

static napi_value minify_code(napi_env env, napi_callback_info info) {
  napi_status status;

  node_args<1> argv;
  status = argv.load(env, info);
  assert(status = napi_ok);

  std::string content = node_cast<std::string>(env, argv[0]);
  const char* result = jetpack_parse_and_codegen(content.c_str(), JETPACK_MINIFY | JETPACK_JSX);
  if (result == nullptr) {
    const char* error_msg = jetpack_error_message();
    napi_throw_type_error(env, nullptr, error_msg);
    return 0;
  }

  napi_value value;
  status = napi_create_string_utf8(env, result, NAPI_AUTO_LENGTH, &value);

  assert(status == napi_ok);
                                
  return value;
}

static napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

#define REGISTER_CALLBACK(NAME, FUN) \
    status = SetCallbackProp(env, exports, NAME, FUN); \
    assert(status == napi_ok)

  REGISTER_CALLBACK("bundleFile", bundle_file);
  REGISTER_CALLBACK("handleCli", handle_command_line);
  REGISTER_CALLBACK("minify", minify_code);

  return exports;
}

NAPI_MODULE(jetpack, Init)
