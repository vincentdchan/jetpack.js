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

  jetpack::simple_api::BundleModule(true, false, true, false, path, "bundle.js");

  return 0;
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

  return exports;
}

NAPI_MODULE(polodb, Init)
