/**
 * A template library for helping manipulating Node.js's N-API
 * 
 * September 2021 @copyright
 * Vincent Chan(okcdz@diverse.space)
 */
#pragma once

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <string>
#include <node_api.h>

inline bool node_is_array(napi_env env, napi_value value)
{
  napi_status status;
  bool result = false;
  status = napi_is_array(env, value, &result);
  if (status != napi_ok) {
    return false;
  }
  return result;
}

template <typename T>
T node_cast(napi_env env, napi_value value) {
  return T();
}

template <>
std::string node_cast<std::string>(napi_env env, napi_value value) {
  napi_status status;

  size_t need_size = 0;
  status = napi_get_value_string_utf8(env, value, nullptr, 0, &need_size);
  if (status != napi_ok) {
    return "";
  }

  if (need_size == 0) {
    return "";
  }

  std::string result;
  result.resize(need_size, '\0');

  status = napi_get_value_string_utf8(env, value, result.data(), need_size + 1, &need_size);
  if (status != napi_ok) {
    return "";
  }

  return result;
}

template<typename T>
struct assert_false : std::false_type
{ };

template <typename T>
napi_value to_node_value(napi_env env, const T& v) {
  static_assert(assert_false<T>::value , "not impl");
  return nullptr;
}

template <>
napi_value to_node_value<std::string>(napi_env env, const std::string &v) {
  napi_value result = nullptr;
  napi_status status = napi_create_string_utf8(env, v.c_str(), v.size(), &result);
  assert(status == napi_ok);
  return result;
}

template <>
napi_value to_node_value<int32_t>(napi_env env, const int32_t& v) {
  napi_value ret_value;
  napi_status status = napi_create_int32(env, v, &ret_value);
  assert(status == napi_ok);
  return ret_value;
}

template <typename T>
T get_object_prop_or(napi_env env, napi_value obj, const char *name, const T& default_v) {
  napi_status status;

  napi_value value;
  status = napi_get_named_property(env, obj, name, &value);
  if (status != napi_ok) {
    return default_v;
  }

  return node_cast<T>(env, value);
}

template <typename T>
T get_object_prop(napi_env env, napi_value obj, const char *name) {
  return get_object_prop_or(env, obj, name, T());
}

template <size_t N>
class node_args
{
public:
  node_args() {
    std::memset(data_, 0, sizeof(napi_value) * N);
  }

  node_args(const node_args<N> &that) = delete;
  node_args(node_args<N> &&that) = delete;

  node_args<N> &operator=(const node_args<N> &that) = delete;
  node_args<N> &operator=(node_args<N> &&that) = delete;

  inline napi_status load(napi_env env, napi_callback_info info) {
    argc_ = N;
    return napi_get_cb_info(env, info, &argc_, data_, nullptr, nullptr);
  }

  inline napi_value &operator[](size_t index) {
    assert(index < argc_);
    return data_[index];
  }

  inline const napi_value &operator[](size_t index) const {
    assert(index < argc_);
    return data_[index];
  }

  inline size_t size() const {
    return argc_;
  }

private:
  size_t argc_ = 0;
  napi_value data_[N];
};
