#pragma once

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <node_api.h>

template<size_t N>
class node_args{
public:
    node_args() {
        std::memset(data_, 0, sizeof(napi_value) * N);
    }

    node_args(const node_args<N>& that) = delete;
    node_args(node_args<N>&& that) = delete;

    node_args<N>& operator=(const node_args<N>& that) = delete;
    node_args<N>& operator=(node_args<N>&& that) = delete;

    inline napi_status load(napi_env env, napi_callback_info info) {
        size_t argc = N;
        return napi_get_cb_info(env, info, &argc, data_, nullptr, nullptr);
    }

    inline napi_value& operator[](size_t index) {
        return data_[index];
    }

    inline const napi_value& operator[](size_t index) const {
        return data_[index];
    }

private:
    napi_value data_[N];    

};
