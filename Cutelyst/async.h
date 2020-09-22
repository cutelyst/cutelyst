/*
 * Copyright (C) 2020 Daniel Nicoletti <dantti12@gmail.com>
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
#include <QSharedPointer>

namespace Cutelyst {

class Context;
class ASyncPrivate;
class CUTELYST_LIBRARY ASync
{
public:
    ASync();

    /*!
     * \brief ASync class should be used in a scoped manner
     *
     * This constructor will call c->detachAsync() and once it goes
     * out of scope it will call c->attachAsync() if Context pointer is still valid.
     *
     * Make sure it is captured by lambdas to avoid it leaving scope.
     *
     * \param c
     */
    ASync(Context *c);
    ASync(const ASync &other);

    ~ASync();

    ASync &operator =(const ASync &copy);

private:
    QSharedPointer<ASyncPrivate> d;
};

}

#endif // ASYNC_H
