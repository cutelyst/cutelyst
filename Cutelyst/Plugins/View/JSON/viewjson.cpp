/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "viewjson_p.h"

#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using namespace Cutelyst;

ViewJson::ViewJson(QObject *parent, const QString &name) : View(parent, name)
  , d_ptr(new ViewJsonPrivate)
{

}

ViewJson::~ViewJson()
{
    delete d_ptr;
}

ViewJson::JsonFormat ViewJson::outputFormat() const
{
    Q_D(const ViewJson);
    switch (d->format) {
    case QJsonDocument::Indented:
        return Indented;
        break;
    case Compact:
        return Compact;
    }
    return Compact;
}

void ViewJson::setOutputFormat(JsonFormat format)
{
    Q_D(ViewJson);
    switch (format) {
    case Indented:
        d->format = QJsonDocument::Indented;
        break;
    case Compact:
        d->format = QJsonDocument::Compact;
    }
}

ViewJson::ExposeMode ViewJson::exposeStashMode() const
{
    Q_D(const ViewJson);
    return d->exposeMode;
}

void ViewJson::setExposeStashString(const QString &key)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::String;
    d->exposeKey = key;
}

void ViewJson::setExposeStashStringList(const QStringList &keys)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::StringList;
    d->exposeKeys = keys;
}

void ViewJson::setExposeStashRegularExpression(const QRegularExpression &re)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::RegularExpression;
    d->exposeRE = re;
}

QByteArray ViewJson::render(Context *c) const
{
    Q_D(const ViewJson);

    QJsonDocument document;

    const QVariantHash stash = c->stash();

    switch (d->exposeMode) {
    case All:
        document.setObject(QJsonObject::fromVariantHash(stash));
        break;
    case String:
    {
        auto it = stash.constFind(d->exposeKey);
        if (it != stash.constEnd()) {
            QJsonObject obj;
            obj.insert(d->exposeKey, QJsonValue::fromVariant(it.value()));
            document.setObject(obj);
        }
        break;
    }
    case StringList:
    {
        QVariantHash exposedStash;

        auto it = stash.constBegin();
        while (it != stash.constEnd()) {
            const QString &key = it.key();
            if (d->exposeKeys.contains(key)) {
                exposedStash.insertMulti(it.key(), it.value());
            }
            ++it;
        }
        document.setObject(QJsonObject::fromVariantHash(exposedStash));
        break;
    }
    case RegularExpression:
    {
        QVariantHash exposedStash;

        auto it = stash.constBegin();
        while (it != stash.constEnd()) {
            const QString &key = it.key();
            if (d->exposeRE.match(key).hasMatch()) {
                exposedStash.insertMulti(key, it.value());
            }
            ++it;
        }
        document.setObject(QJsonObject::fromVariantHash(exposedStash));
        break;
    }
    }

    c->response()->setContentType(QStringLiteral("application/json; charset=utf-8"));

    return document.toJson(d->format);
}
