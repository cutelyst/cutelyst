/*
 * SPDX-FileCopyrightText: (C) 2020-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_export.h>
#include <functional>
#include <memory>

namespace Cutelyst {

class Context;
class ASyncPrivate;
class CUTELYST_EXPORT ASync
{
public:
    ASync() noexcept;
    ASync(Context *c);
    /**
     * ASync class should be used in a scoped manner.
     *
     * This constructor will call \link Context::detachAsync() c->detachAsync()\endlink on context
     * \a c and once it goes out of scope if Context pointer is still valid it will call the
     * callback function \a cb and then \link Context::attachAsync() c->attachAsync()\endlink.
     *
     * Make sure it is captured by lambdas to avoid it leaving scope.
     */
    ASync(Context *c, std::move_only_function<void(Context *c)> cb);
    ASync(const ASync &other);
    ASync(ASync &&other) noexcept;

    ~ASync();

    ASync &operator=(const ASync &copy);

    ASync &operator=(ASync &&other) noexcept
    {
        std::swap(d, other.d);
        return *this;
    }

private:
    std::shared_ptr<ASyncPrivate> d;
};

} // namespace Cutelyst
