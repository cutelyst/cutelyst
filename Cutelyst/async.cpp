/*
 * Copyright (C) 2021 Daniel Nicoletti <dantti12@gmail.com>
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
#include "async.h"
#include "context.h"

#include <QPointer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_ASYNC, "cutelyst.async", QtInfoMsg)

using namespace Cutelyst;

namespace Cutelyst {

class ASyncPrivate {
public:
    ASyncPrivate(Context *_c) : c(_c) {
//        qDebug(CUTELYST_ASYNC, "Detaching async %s", qPrintable(c->objectName()));
        c->detachAsync();
    }
    ASyncPrivate(Context *_c, std::function<void(Context *c)> _cb) : c(_c), cb(_cb) {
//        qDebug(CUTELYST_ASYNC, "Detaching async %s", qPrintable(c->objectName()));
        c->detachAsync();
    }
    ~ASyncPrivate() {
        if (!c.isNull()) {
            if (cb) {
                cb(c);
            }
//            qDebug(CUTELYST_ASYNC, "Attaching async %s", qPrintable(c->objectName()));
            c->attachAsync();
        }
    }

    QPointer<Context> c;
    std::function<void(Context *c)> cb;
};

}

ASync::ASync() = default;

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
ASync::ASync(Context *c) : d(std::make_shared<ASyncPrivate>(c))
{
}

/*!
 * \brief ASync class should be used in a scoped manner
 *
 * This constructor will call c->detachAsync() and once it goes
 * out of scope if Context pointer is still valid it will call the callback
 * function and then c->attachAsync().
 *
 * Make sure it is captured by lambdas to avoid it leaving scope.
 *
 * \param c
 * \param cb callback to be called when async tasks are finished
 */
ASync::ASync(Context *c, std::function<void (Context *)> cb) : d(std::make_shared<ASyncPrivate>(c, cb))
{
}

/*!
 * Copy constructor
 */
ASync::ASync(const ASync &other) : d(other.d)
{
}

ASync::~ASync() = default;

ASync &ASync::operator =(const ASync &copy) noexcept
{
    d = copy.d;
    return *this;
}
