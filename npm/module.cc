#include <node_api.h>
#include <cstdlib>
#include <cassert>

#include "./node_helper.hpp"
#include "parser/ParseErrorHandler.h"
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
  node_args<1> argv;
  status = argv.load(env, info);
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

  JetpackFlags flags;
  flags |= JetpackFlag::Jsx;
  jetpack::simple_api::BundleModule(path, "bundle.js", flags);

  return 0;
}

static napi_value handle_command_line(napi_env env, napi_callback_info info) {
  int rt = 3;
  napi_status status;

  node_args<1> argv;
  status = argv.load(env, info);
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

static napi_value minify_code(napi_env env, napi_callback_info info) {
  napi_status status;

  node_args<1> argv;
  status = argv.load(env, info);
  assert(status = napi_ok);

  size_t str_size = 0;
  status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &str_size);
  if (status != 0) {
    napi_throw_type_error(env, nullptr, "the first arg should be a string");
    return 0;
  }

  char* buffer = (char*)malloc(str_size + 1);
  buffer[str_size] = 0;
  status = napi_get_value_string_utf8(env, argv[0], buffer, str_size + 1, &str_size);
  assert(status == napi_ok);

  std::string result;
  try {
      std::string content(buffer);
      jetpack::parser::Config parser_config = jetpack::parser::Config::Default();
      jetpack::CodeGenConfig code_gen_config;
      parser_config.jsx = true;
      parser_config.constant_folding = true;
      code_gen_config.minify = true;
      result = jetpack::simple_api::ParseAndCodeGen(std::move(content), parser_config, code_gen_config);
  } catch (jetpack::parser::ParseError& err) {
      std::string errMsg = err.ErrorMessage();
      free(buffer);
      napi_throw_type_error(env, nullptr, errMsg.c_str());
      return 0;
  } catch (...) {
      free(buffer);
      napi_throw_type_error(env, nullptr, "unknown error");
      return 0;
  }

  free(buffer);

  napi_value js_result;
  status = napi_create_string_utf8(env, result.c_str(), result.size(), &js_result);
  assert(status == napi_ok);
  return js_result;
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
