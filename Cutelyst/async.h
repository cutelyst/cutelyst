/*
 * Copyright (C) 2020-2022 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
