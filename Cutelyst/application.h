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

#ifndef CUTELYST_APPLICATION_H
#define CUTELYST_APPLICATION_H

#include <QCoreApplication>

namespace Cutelyst {

namespace Plugin {
class AbstractPlugin;
}
class Context;
class Request;
class Response;
class Engine;
class ApplicationPrivate;
class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = 0);
    virtual ~Application();

    bool parseArgs();
    int printError();
    bool setup(Engine *engine = 0);

    /**
     * Registers a global plugin ie one that doesn't need
     * to be created explicity for a single request and returns
     * true on plugin->isApplicationPlugin();
     *
     * @return true if the plugin could be registered
     */
    bool registerPlugin(Plugin::AbstractPlugin  *plugin);

    QString applicationName() const;
    void setApplicationName(const QString &applicationName);

    QString applicationVersion() const;
    void setApplicationVersion(const QString &applicationVersion);

Q_SIGNALS:
    /**
     * Emited so that you register all plugins that are specifically
     * to each request
     */
    void registerPlugins(Context *ctx);

protected:
    ApplicationPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Application)

    void handleRequest(Request *req, Response *resp);
};

}

#define CutelystApplicationInterface_iid "org.cutelyst.CutelystApplicationInterface"

Q_DECLARE_INTERFACE(Cutelyst::Application, CutelystApplicationInterface_iid)

#endif // CUTELYST_APPLICATION_H
