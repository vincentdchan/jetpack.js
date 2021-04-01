#include <node_api.h>
#include <cassert>

#include "SimpleAPI.h"

#define ENTRY_PATH_MAX 512

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_enumerable, 0 }


/// bundleFile({
///   entry: '',
///
/// })
static napi_value bundle_file(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  assert(status = napi_ok);

  napi_value entry_value;
  status = napi_get_named_property(env, argv[0], "entry", &entry_value);
  if (status != napi_ok) {
    napi_throw_type_error(env, nullptr, "entry is not proviede");
    return 0;
  }

  char path[ENTRY_PATH_MAX];
  ::memset(path, 0, ENTRY_PATH_MAX);
  size_t s;
  status = napi_get_value_string_utf8(env, entry_value, path, ENTRY_PATH_MAX, &s);
  assert(status == napi_ok);

  jetpack::simple_api::Flags flags;
  flags.setJsx(true);
  jetpack::simple_api::BundleModule(path, "bundle.js", flags);

  return 0;
}

static napi_value handle_command_line(napi_env env, napi_callback_info info) {
  int rt = 3;
  napi_status status;

  size_t argc = 1;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  assert(status = napi_ok);

  bool is_arr = false;
  status = napi_is_array(env, argv[0], &is_arr);
  assert(status = napi_ok);
  if (!is_arr) {
    napi_throw_type_error(env, NULL, "first arguments should be an array");
    return 0;
  }

  uint32_t arr_len = 0;
  status = napi_get_array_length(env, argv[0], &arr_len);
  assert(status == napi_ok);

  char** cmd_argv = (char**)malloc(sizeof(char*) * arr_len);


#define STR_BUFFER_SIZE 1024
  for (uint32_t i = 0; i < arr_len; i++) {
    napi_value result;
    status = napi_get_element(env, argv[0], i, &result);
    assert(status == napi_ok);

    char* str_buffer = (char*)::malloc(STR_BUFFER_SIZE);
    ::memset(str_buffer, 0, STR_BUFFER_SIZE);
    size_t str_size;
    status = napi_get_value_string_utf8(env, result, str_buffer, 1024, &str_size);
    assert(status == napi_ok);

    cmd_argv[i] = str_buffer;
  }

  rt = jetpack::simple_api::HandleCommandLine(arr_len, cmd_argv);

failed:
  for (uint32_t i = 0; i < arr_len; i++) {
    ::free(cmd_argv[i]);
  }
  ::free(cmd_argv);

  napi_value ret_value;
  status = napi_create_int32(env, rt, &ret_value);
  assert(status == napi_ok);
  return ret_value;
}

static napi_status SetCallbackProp(napi_env env, napi_value exports, const char* key, napi_callback cb) {
  napi_status status;

  napi_property_descriptor desc = DECLARE_NAPI_METHOD(key, cb);
  status = napi_define_properties(env, exports, 1, &desc);

  return status;
}

static napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

#define REGISTER_CALLBACK(NAME, FUN) \
    status = SetCallbackProp(env, exports, NAME, FUN); \
    assert(status == napi_ok)

  REGISTER_CALLBACK("bundleFile", bundle_file);

  REGISTER_CALLBACK("handleCli", handle_command_line);

  return exports;
}

NAPI_MODULE(polodb, Init)
