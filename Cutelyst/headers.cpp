/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "headers.h"

#include <QStringBuilder>
#include <QStringList>

using namespace Cutelyst;

Headers::Headers()
{
}

qint64 Headers::contentLength() const
{
    return value("Content-Length").toLongLong();
}

void Headers::setContentLength(qint64 value)
{
    setHeader("Content-Length", QByteArray::number(value));
}

QDateTime Headers::date() const
{
    return QDateTime::fromString(value("Date"));
}

void Headers::setDate(const QDateTime &date)
{
    QString dateString;
    // TODO check if there is some RFC format
    dateString = date.toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    setHeader("Date", dateString.toLocal8Bit());
}

QByteArray Headers::server() const
{
    return value("Server");
}

void Headers::setServer(const QByteArray &value)
{
    setHeader("Server", value);
}

QByteArray Headers::header(const QByteArray &field) const
{
    return value(field);
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    setHeader(field.toLocal8Bit(), values.join(QLatin1String(", ")).toLocal8Bit());
}

void Headers::setHeader(const QByteArray &field, const QByteArray &value)
{
    if (value.isNull()) {
        remove(field);
    } else {
        insert(field, value);
    }
}
