//
// Created by Duzhong Chen on 2021/3/28.
//

#ifndef ROCKET_BUNDLE_RESOLVERESULT_H
#define ROCKET_BUNDLE_RESOLVERESULT_H

#include <optional>
#include "WorkerError.h"

namespace jetpack {

    template <typename T, typename C = T>
    struct ResolveResult {
    public:
        ResolveResult() = default;
        explicit ResolveResult(const C& v): value(v) {}
        ResolveResult(C&& v): value(std::move(v)) {}
        ResolveResult(const ResolveResult<T, C>& that) = delete;
        ResolveResult(ResolveResult<T, C>&& that) = default;

        [[nodiscard]]
        inline bool HasError() const {
            return error.has_value();
        }

        T value;
        std::optional<WorkerError> error;

    };

}

#endif //ROCKET_BUNDLE_RESOLVERESULT_H
