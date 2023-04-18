/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "cutelystcutelee.h"

#include "csrf.h"
#include "urifor.h"

CutelystCutelee::CutelystCutelee(QObject *parent)
    : QObject(parent)
{
}

QHash<QString, Cutelee::AbstractNodeFactory *> CutelystCutelee::nodeFactories(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Cutelee::AbstractNodeFactory *> ret{
        {QStringLiteral("c_uri_for"), new UriForTag()},
        {QStringLiteral("c_csrf_token"), new CSRFTag()},
    };

    return ret;
}

QHash<QString, Cutelee::Filter *> CutelystCutelee::filters(const QString &name)
{
    Q_UNUSED(name)

    QHash<QString, Cutelee::Filter *> ret;

    return ret;
}

#include "moc_cutelystcutelee.cpp"
