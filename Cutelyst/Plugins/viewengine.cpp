/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "viewengine.h"

#include "application.h"
#include "context.h"
#include "response.h"
#include "request.h"
#include "Plugins/View/ViewInterface.h"

#include "common.h"

#include <QDir>
#include <QPluginLoader>
#include <QJsonArray>
#include <QCoreApplication>
#include <QStringBuilder>
#include <QLoggingCategory>
#include <QThread>

using namespace Cutelyst;

ViewEngine::ViewEngine(const QString &engine, Application *app) : View(app)
  , m_interface(0)
{
    QDir pluginsDir("/usr/lib/cutelyst-plugins");
    Q_FOREACH (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName), app);
        QJsonObject json = pluginLoader.metaData()["MetaData"].toObject();
        if (json["name"].toString() == engine) {
            QObject *plugin = pluginLoader.instance();
            if (plugin) {
                m_interface = qobject_cast<ViewInterface *>(plugin);
                if (!m_interface) {
                    qCCritical(CUTELYST_VIEW) << "Could not create an instance of the view engine:" << engine;
                } else {
                    m_interface = qobject_cast<ViewInterface *>(m_interface->metaObject()->newInstance(Q_ARG(QObject*, app)));

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

    // Set code name
    setObjectName(metaObject()->className() % QLatin1String("::") % engine % QLatin1String("->execute"));
}

ViewEngine::~ViewEngine()
{
}

QStringList ViewEngine::includePaths() const
{
    Q_ASSERT(m_interface);
    return m_interface->includePaths();
}

void ViewEngine::setIncludePaths(const QStringList &paths)
{
    Q_ASSERT(m_interface);
    m_interface->setIncludePaths(paths);
}

QString ViewEngine::templateExtension() const
{
    Q_ASSERT(m_interface);
    return m_interface->templateExtension();
}

void ViewEngine::setTemplateExtension(const QString &extension)
{
    Q_ASSERT(m_interface);
    m_interface->setTemplateExtension(extension);
}

QString ViewEngine::wrapper() const
{
    Q_ASSERT(m_interface);
    return m_interface->wrapper();
}

void ViewEngine::setWrapper(const QString &name)
{
    Q_ASSERT(m_interface);
    m_interface->setWrapper(name);
}

bool ViewEngine::isCaching() const
{
    Q_ASSERT(m_interface);
    return m_interface->isCaching();
}

void ViewEngine::setCache(bool enable)
{
    Q_ASSERT(m_interface);
    m_interface->setCache(enable);
}

bool ViewEngine::render(Context *ctx) const
{
    Q_ASSERT(m_interface);
    if (!m_interface->render(ctx)) {
        ctx->response()->setStatus(Response::InternalServerError);
        return false;
    }
    return true;
}
