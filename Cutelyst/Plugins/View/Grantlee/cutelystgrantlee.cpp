/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "cutelystgrantlee.h"

#include "csrf.h"
#include "urifor.h"

CutelystGrantlee::CutelystGrantlee(QObject *parent)
    : QObject(parent)
{
}

QHash<QString, Grantlee::AbstractNodeFactory *> CutelystGrantlee::nodeFactories(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::AbstractNodeFactory *> ret{
        {QStringLiteral("c_uri_for"), new UriForTag()},
        {QStringLiteral("c_csrf_token"), new CSRFTag()},
    };

    return ret;
}

QHash<QString, Grantlee::Filter *> CutelystGrantlee::filters(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Grantlee::Filter *> ret;

    return ret;
}

#include "moc_cutelystgrantlee.cpp"
