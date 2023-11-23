/*
 * SPDX-FileCopyrightText: (C) 2020-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "async.h"

#include "context.h"

#include <QLoggingCategory>
#include <QPointer>

Q_LOGGING_CATEGORY(CUTELYST_ASYNC, "cutelyst.async", QtInfoMsg)

using namespace Cutelyst;

namespace Cutelyst {

class ASyncPrivate
{
public:
    ASyncPrivate(Context *_c)
        : c(_c)
    {
        //        qDebug(CUTELYST_ASYNC, "Detaching async %s", qPrintable(c->objectName()));
        c->detachAsync();
    }
    ASyncPrivate(Context *_c, std::function<void(Context *c)> _cb)
        : c(_c)
        , cb(_cb)
    {
        //        qDebug(CUTELYST_ASYNC, "Detaching async %s", qPrintable(c->objectName()));
        c->detachAsync();
    }
    ~ASyncPrivate()
    {
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

} // namespace Cutelyst

/**
 * \ingroup core
 * \class Cutelyst::ASync async.h Cutelyst/async.h
 * \brief Helper class for asynchronous processing.
 *
 * %ASync helps with asynchronous processing. It automatically calls Context::detachAsync()
 * on the given context on construction and will call Context::attachAsync() on it’s own
 * destruction if the context pointer given in the constructor is still valid.
 */

/**
 * Constructs a new default %ASync object containing a \c nullptr.
 */
ASync::ASync() noexcept = default;

/**
 * ASync class should be used in a scoped manner.
 *
 * This constructor will call \link Context::detachAsync() c->detachAsync()\endlink on context
 * \a c and once it goes out of scope it will call
 * \link Context::attachAsync() c->attachAsync()\endlink if Context pointer is still valid.
 *
 * Make sure it is captured by lambdas to avoid it leaving scope.
 */
ASync::ASync(Context *c)
    : d(std::make_shared<ASyncPrivate>(c))
{
}

ASync::ASync(Context *c, std::function<void(Context *)> cb)
    : d(std::make_shared<ASyncPrivate>(c, cb))
{
}

/**
 * Constructs a copy of \a other.
 */
ASync::ASync(const ASync &other)
    : d(other.d)
{
}

/**
 * Move-constructs an %ASync from \a other.
 */
ASync::ASync(ASync &&other) noexcept
    : d(std::move(other.d))
{
}

/**
 * Destroys the %ASync object.
 */
ASync::~ASync() = default;

/**
 * Assigns \a other to this object.
 */
ASync &ASync::operator=(const ASync &copy)
{
    d = copy.d;
    return *this;
}
