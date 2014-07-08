/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "view.h"
#include "context.h"
#include "response.h"
#include "request.h"
#include "Plugin/View/ViewInterface.h"

#include <QDir>
#include <QPluginLoader>
#include <QJsonArray>

using namespace Cutelyst;

View::View(const QString &engine, QObject *parent) :
    QObject(parent),
    interface(0)
{
    QDir pluginsDir("/usr/lib/cutelyst-plugins");
    Q_FOREACH (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject json = pluginLoader.metaData()["MetaData"].toObject();
        if (json["name"].toString() == engine) {
            QObject *plugin = pluginLoader.instance();
            if (plugin) {
                interface = qobject_cast<ViewInterface *>(plugin);
                if (!interface) {
                    qCritical() << "Could not create an instance of the engine:" << engine;
                } else {
                    break;
                }
            }
        }
    }

//    qDebug() << interface;

    if (!interface) {
        qCritical() << "Engine not loaded:" << engine;
    }
}

bool View::process(Context *ctx)
{
    Response *res = ctx->res();
    if (res->contentType().isEmpty()) {
        res->setContentType("text/html; charset=utf-8");
    }

    if (ctx->req()->method() == "HEAD") {
        return true;
    }

    if (!res->body().isNull()) {
        return true;
    }

    quint16 status = res->status();
    if (status == 204 || (status >= 300 && status < 400)) {
        return true;
    }

    return render(ctx);
}

QString View::includePath() const
{
    return interface->includePath();
}

void View::setIncludePath(const QString &path)
{
    interface->setIncludePath(path);
}

QString View::templateExtension() const
{
    return interface->templateExtension();
}

void View::setTemplateExtension(const QString &extension)
{
    interface->setTemplateExtension(extension);
}

QString View::wrapper() const
{
    return interface->wrapper();
}

void View::setWrapper(const QString &name)
{
    interface->setWrapper(name);
}

bool View::render(Context *ctx)
{
    return interface->render(ctx);
}
