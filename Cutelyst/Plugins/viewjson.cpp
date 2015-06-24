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

ViewJson::ViewJson(Application *app) : Cutelyst::View(app)
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

bool ViewJson::render(Context *c) const
{
    Q_D(const ViewJson);

    QJsonDocument document;

    const QVariantHash &stash = c->stash();

    // TODO once 5.5 is out and we depend on it use QJsonObject::fromVariantHash
    QVariantMap workaround;
    switch (d->exposeMode) {
    case All:
    {
        QVariantHash::ConstIterator it = stash.constBegin();
        while (it != stash.constEnd()) {
            workaround.insert(it.key(), it.value());
            ++it;
        }
        document.setObject(QJsonObject::fromVariantMap(workaround));
        break;
    }
    case String:
    {
        QVariantHash::ConstIterator it = stash.constFind(d->exposeKey);
        if (it != stash.constEnd()) {
            QJsonObject obj;
            obj.insert(d->exposeKey, QJsonValue::fromVariant(it.value()));
            document.setObject(obj);
        }
        break;
    }
    case StringList:
    {
        QVariantHash::ConstIterator it = stash.constBegin();
        while (it != stash.constEnd()) {
            const QString &key = it.key();
            if (d->exposeKeys.contains(key)) {
                workaround.insert(it.key(), it.value());
            }
            ++it;
        }
        document.setObject(QJsonObject::fromVariantMap(workaround));
        break;
    }
    case RegularExpression:
    {
        QVariantHash::ConstIterator it = stash.constBegin();
        while (it != stash.constEnd()) {
            const QString &key = it.key();
            if (d->exposeRE.match(key).hasMatch()) {
                workaround.insert(key, it.value());
            }
            ++it;
        }
        document.setObject(QJsonObject::fromVariantMap(workaround));
        break;
    }
    }

    Response *res = c->response();
    res->setContentType(QStringLiteral("application/json; charset=utf-8"));

    res->body() = document.toJson(d->format);

    return true;
}
