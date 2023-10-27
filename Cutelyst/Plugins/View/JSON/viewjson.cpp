/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "viewjson_p.h"

#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

using namespace Cutelyst;

/*!
 * \class Cutelyst::ViewJson viewjson.h Cutelyst/Plugins/JSON/viewjson.h
 * \brief JSON view for your data
 *
 * Cutelyst::ViewJSON is a Cutelyst View handler that returns stash data in JSON format.
 */
ViewJson::ViewJson(QObject *parent, const QString &name)
    : View(new ViewJsonPrivate, parent, name)
{
}

ViewJson::JsonFormat ViewJson::outputFormat() const
{
    Q_D(const ViewJson);
    switch (d->format) {
    case QJsonDocument::Indented:
        return Indented;
        break;
    case QJsonDocument::Compact:
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

void ViewJson::setExposeStash(const QString &key)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::String;
    d->exposeKey  = key;
}

void ViewJson::setExposeStash(const QStringList &keys)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::StringList;
    d->exposeKeys = keys;
}

void ViewJson::setExposeStash(const QRegularExpression &re)
{
    Q_D(ViewJson);
    d->exposeMode = ViewJson::RegularExpression;
    d->exposeRE   = re;
}

void ViewJson::setXJsonHeader(bool enable)
{
    Q_D(ViewJson);
    d->xJsonHeader = enable;
}

bool ViewJson::xJsonHeader() const
{
    Q_D(const ViewJson);
    return d->xJsonHeader;
}

QByteArray ViewJson::render(Context *c) const
{
    Q_D(const ViewJson);

    QJsonObject obj;

    const QVariantHash stash = c->stash();

    switch (d->exposeMode) {
    case All:
        obj = QJsonObject::fromVariantHash(stash);
        break;
    case String:
    {
        auto it = stash.constFind(d->exposeKey);
        if (it != stash.constEnd()) {
            obj.insert(d->exposeKey, QJsonValue::fromVariant(it.value()));
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
                exposedStash.insert(key, it.value());
            }
            ++it;
        }
        obj = QJsonObject::fromVariantHash(exposedStash);
        break;
    }
    case RegularExpression:
    {
        QVariantHash exposedStash;
        QRegularExpression re = d->exposeRE; // thread safety

        auto it = stash.constBegin();
        while (it != stash.constEnd()) {
            const QString &key = it.key();
            if (re.match(key).hasMatch()) {
                exposedStash.insert(key, it.value());
            }
            ++it;
        }
        obj = QJsonObject::fromVariantHash(exposedStash);
        break;
    }
    }

    Response *res = c->response();
    if (d->xJsonHeader && c->request()->headers().contains("X-Prototype-Version")) {
        res->setHeader("X-Json"_qba, "eval(\"(\"+this.transport.responseText+\")\")"_qba);
    }

    res->setContentType("application/json"_qba);

    return QJsonDocument(obj).toJson(d->format);
}

#include "moc_viewjson.cpp"
