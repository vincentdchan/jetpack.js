#include <node_api.h>
#include <cstdlib>
#include <cassert>

#include "./node_helper.hpp"
#include "parser/ParseErrorHandler.h"
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
  node_args<1> argv;
  status = argv.load(env, info);
  assert(status = napi_ok);

  napi_value entry_value;
  status = napi_get_named_property(env, argv[0], "entry", &entry_value);
  if (status != napi_ok) {
    napi_throw_type_error(env, nullptr, "entry is not proviede");
    return 0;
  }

  std::string path = node_cast<std::string>(env, entry_value);

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

  rt = jetpack::simple_api::HandleCommandLine(arr_len, cmd_argv);

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

  std::string result;
  try {
      std::string content = node_cast<std::string>(env, argv[0]);
      jetpack::parser::Config parser_config = jetpack::parser::Config::Default();
      jetpack::CodeGenConfig code_gen_config;
      parser_config.jsx = true;
      parser_config.constant_folding = true;
      code_gen_config.minify = true;
      result = jetpack::simple_api::ParseAndCodeGen(content, parser_config, code_gen_config);
  } catch (jetpack::parser::ParseError& err) {
      std::string errMsg = err.ErrorMessage();
      napi_throw_type_error(env, nullptr, errMsg.c_str());
      return 0;
  } catch (...) {
      napi_throw_type_error(env, nullptr, "unknown error");
      return 0;
  }

  return to_node_value(env, result);
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
