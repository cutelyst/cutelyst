/*
 * SPDX-FileCopyrightText: (C) 2020-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_global.h>
#include <functional>
#include <memory>

namespace Cutelyst {

class Context;
class ASyncPrivate;
class CUTELYST_LIBRARY ASync
{
public:
    ASync() noexcept;
    ASync(Context *c);
    ASync(Context *c, std::function<void(Context *c)> cb);
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
