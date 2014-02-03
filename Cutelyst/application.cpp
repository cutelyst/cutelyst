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

#include "application_p.h"

#include "enginehttp.h"
#include "context_p.h"
#include "request.h"
#include "controller.h"
#include "response.h"

#include <iostream>

#include <QStringList>
#include <QDebug>

using namespace std;
using namespace Cutelyst;

void cuteOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[debug] %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "[warn] %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[crit] %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[fatal] %s\n", localMsg.constData());
        abort();
    }
}

Application::Application(QObject *parent) :
    QObject(parent),
    d_ptr(new ApplicationPrivate)
{
    d_ptr->dispatcher = 0;
    qInstallMessageHandler(cuteOutput);
}

Application::~Application()
{
    delete d_ptr;
}

bool Application::parseArgs()
{
    QStringList args = QCoreApplication::arguments();
    if (args.contains(QLatin1String("--about")) ||
            args.contains(QLatin1String("--sobre")) ||
            args.contains(QLatin1String("/sobre"))) {
        cout << QCoreApplication::applicationName().toStdString() << endl
             << QCoreApplication::applicationVersion().toStdString() << endl
             << QCoreApplication::organizationName().toStdString() << endl
             << QCoreApplication::organizationDomain().toStdString() << endl
             << "Qt: " << QT_VERSION_STR << endl;
        return false;
    }

    return true;
}

int Application::printError()
{
    return 1;
}

bool Application::setup(Engine *engine)
{
    Q_D(Application);

    d->dispatcher = new Dispatcher(this);
    d->dispatcher->setupActions(d->controllers);

    if (engine) {
        d->engine = engine;
    } else {
        d->engine = new CutelystEngineHttp(this);
    }
    connect(d->engine, &Engine::handleRequest,
            this, &Application::handleRequest);

    return d->engine->init();
}

bool Application::registerPlugin(Plugin::AbstractPlugin *plugin)
{
    Q_D(Application);

    if (plugin->isApplicationPlugin()) {
        d->plugins << plugin;
        return true;
    }
    qWarning() << "The plugin:" << plugin->metaObject()->className() << "isn't an Application Plugin and cannot be registered";
    return false;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);

    if (d_ptr->dispatcher) {
        return false;
    }

    controller->setParent(this);
    d->controllers << controller;
    return true;
}

QString Application::applicationName() const
{
    Q_D(const Application);
    if (d->applicationName.isNull()) {
        return metaObject()->className();
    }
    return d->applicationName;
}

void Application::setApplicationName(const QString &applicationName)
{
    Q_D(Application);
    d->applicationName = applicationName;
}

QString Application::applicationVersion() const
{
    Q_D(const Application);
    return d->applicationVersion;
}

void Application::setApplicationVersion(const QString &applicationVersion)
{
    Q_D(Application);
    d->applicationVersion = applicationVersion;
}

void Application::handleRequest(Request *req, Response *resp)
{
    Q_D(Application);

    ContextPrivate *priv = new ContextPrivate;
    priv->engine = d->engine;
    priv->dispatcher = d->dispatcher;
    priv->request = req;
    priv->response = resp;
    Context *ctx = new Context(priv);

    // Register application plugins
    foreach (Plugin::AbstractPlugin *plugin, d->plugins) {
        ctx->registerPlugin(plugin, false);
    }

    // Register context plugins
    registerPlugins(ctx);

    ctx->handleRequest();

    delete ctx;
}
