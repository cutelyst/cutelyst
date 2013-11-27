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

#ifndef CUTELYSTAPPLICATION_H
#define CUTELYSTAPPLICATION_H

#include <QCoreApplication>

namespace CutelystPlugin {
class Plugin;
}
class Cutelyst;
class CutelystRequest;
class CutelystResponse;
class CutelystEngine;
class CutelystApplicationPrivate;
class CutelystApplication : public QCoreApplication
{
    Q_OBJECT
public:
    CutelystApplication(int &argc, char **argv);
    ~CutelystApplication();

    void registerPlugin(CutelystPlugin::Plugin *plugin, const QString &name = QString());

    bool parseArgs();
    int printError();
    bool setup(CutelystEngine *engine = 0);

Q_SIGNALS:
    void beforePrepareAction(Cutelyst *c, bool *skipMethod);
    void afterPrepareAction(Cutelyst *c);
    void beforeDispatch(Cutelyst *c);
    void afterDispatch(Cutelyst *c);

protected:
    CutelystApplicationPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystApplication)

    void handleRequest(CutelystRequest *req, CutelystResponse *resp);
};

#endif // CUTELYSTAPPLICATION_H
