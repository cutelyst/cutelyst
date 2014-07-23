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

#include "common.h"

#include <QDir>
#include <QPluginLoader>
#include <QJsonArray>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QThread>

using namespace Cutelyst;

View::View(const QString &engine, QObject *parent) :
    QObject(parent)
{
    QDir pluginsDir("/usr/lib/cutelyst-plugins");
    Q_FOREACH (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject json = pluginLoader.metaData()["MetaData"].toObject();
        if (json["name"].toString() == engine) {
            QObject *plugin = pluginLoader.instance();
            if (plugin) {
                m_interface = qobject_cast<ViewInterface *>(plugin);
                if (!m_interface) {
                    qCCritical(CUTELYST_VIEW) << "Could not create an instance of the view engine:" << engine;
                } else if (m_interface->thread() != QThread::currentThread()) {
                    m_interface = qobject_cast<ViewInterface *>(m_interface->metaObject()->newInstance());

                    if (!m_interface) {
                        qCCritical(CUTELYST_VIEW) << "Could not create a NEW instance of the view engine:" << engine;
                    }
                }
                break;
            }
        }
    }

//    qDebug() << interface;

    if (!m_interface) {
        qCCritical(CUTELYST_VIEW) << "View Engine not loaded:" << engine;
    } else {
        m_interface->setParent(this);
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
    Q_ASSERT(m_interface);
    return m_interface->includePath();
}

void View::setIncludePath(const QString &path)
{
    Q_ASSERT(m_interface);
    m_interface->setIncludePath(path);
}

QString View::templateExtension() const
{
    Q_ASSERT(m_interface);
    return m_interface->templateExtension();
}

void View::setTemplateExtension(const QString &extension)
{
    Q_ASSERT(m_interface);
    m_interface->setTemplateExtension(extension);
}

QString View::wrapper() const
{
    Q_ASSERT(m_interface);
    return m_interface->wrapper();
}

void View::setWrapper(const QString &name)
{
    Q_ASSERT(m_interface);
    m_interface->setWrapper(name);
}

bool View::render(Context *ctx)
{
    Q_ASSERT(m_interface);
    return m_interface->render(ctx);
}
