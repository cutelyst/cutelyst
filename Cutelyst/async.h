/*
 * SPDX-FileCopyrightText: (C) 2020-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ASYNC_H
#define ASYNC_H

#include <Cutelyst/cutelyst_global.h>

#include <memory>
#include <functional>

namespace Cutelyst {

class Context;
class ASyncPrivate;
class CUTELYST_LIBRARY ASync
{
public:
    ASync();
    ASync(Context *c);
    ASync(Context *c, std::function<void(Context *c)> cb);
    ASync(const ASync &other);

    ~ASync();

    ASync &operator =(const ASync &copy) noexcept;

private:
    std::shared_ptr<ASyncPrivate> d;
};

}

#endif // ASYNC_H
