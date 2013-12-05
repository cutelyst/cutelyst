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

namespace CutelystPlugin {
class Plugin;
}
class Context;
class Request;
class Response;
class Engine;
class ApplicationPrivate;
class Application : public QCoreApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();

    void registerPlugin(CutelystPlugin::Plugin *plugin);

    bool parseArgs();
    int printError();
    bool setup(Engine *engine = 0);

Q_SIGNALS:
    void beforePrepareAction(Context *ctx, bool *skipMethod);
    void afterPrepareAction(Context *ctx);
    void beforeDispatch(Context *ctx);
    void afterDispatch(Context *ctx);

protected:
    ApplicationPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Application)

    void handleRequest(Request *req, Response *resp);
};

}

#endif // CUTELYST_APPLICATION_H
