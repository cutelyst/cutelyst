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

#include "cutelystapplication_p.h"

#include "cutelystenginehttp.h"
#include "cutelystengineuwsgi.h"
#include "cutelyst.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"

#include "cpstaticsimple.h"

#include <iostream>

#include <QStringList>
#include <QDebug>

using namespace std;

CutelystApplication::CutelystApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv),
    d_ptr(new CutelystApplicationPrivate)
{
}

CutelystApplication::~CutelystApplication()
{
    delete d_ptr;
}

void CutelystApplication::registerPlugin(CutelystPlugin *plugin, const QString &name)
{
    Q_D(CutelystApplication);

    QString pluginName = name;
    if (pluginName.isEmpty()) {
        pluginName = plugin->objectName();
    }

    if (!d->plugins.contains(pluginName)) {
        d->plugins.insert(pluginName, plugin);
    }
}

bool CutelystApplication::parseArgs()
{
    Q_D(CutelystApplication);

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

int CutelystApplication::printError()
{
    return 1;
}

bool CutelystApplication::setup(CutelystEngine *engine)
{
    Q_D(CutelystApplication);

    d->dispatcher = new CutelystDispatcher(this);
    d->dispatcher->setupActions();

    if (engine) {
        d->engine = engine;
    } else {
        d->engine = new CutelystEngineHttp(this);
    }
    connect(d->engine, &CutelystEngine::handleRequest,
            this, &CutelystApplication::handleRequest);

    QHash<QString, CutelystPlugin *>::Iterator it = d->plugins.begin();
    while (it != d->plugins.end()) {
        it.value()->setup(this);
        ++it;
    }

    return d->engine->init();
}

void CutelystApplication::handleRequest(CutelystRequest *req, CutelystResponse *resp)
{
    Q_D(CutelystApplication);

    Cutelyst *c = new Cutelyst(d->engine, d->dispatcher);
    connect(c, &Cutelyst::beforePrepareAction, this, &CutelystApplication::beforePrepareAction);
    connect(c, &Cutelyst::afterPrepareAction, this, &CutelystApplication::afterPrepareAction);
    connect(c, &Cutelyst::beforeDispatch, this, &CutelystApplication::beforeDispatch);
    connect(c, &Cutelyst::afterDispatch, this, &CutelystApplication::afterDispatch);
    c->handleRequest(req, resp);
    delete c;
}
