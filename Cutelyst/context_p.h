/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_P_H
#define CUTELYST_P_H

#include "context.h"
#include "plugin.h"
#include "response.h"

#include <QVariantHash>
#include <QStack>

namespace Cutelyst {

class Stats;
class ContextPrivate
{
public:
    inline ContextPrivate(Context *c, Application *_app, Engine *_ngine, Dispatcher *_dispatcher, void *_reqPtr,
                          Request *_request, const QList<Plugin *> &_plugins, Stats *_stats, const Headers &_headers)
        : app(_app)
        , engine(_ngine)
        , dispatcher(_dispatcher)
        , requestPtr(_reqPtr)
        , request(_request)
        , response(new Response(c, _ngine, _headers))
        , plugins(_plugins)
        , stats(_stats)
    { }

    QString statsStartExecute(Component *code);
    void statsFinishExecute(const QString &statsInfo);

    Application *app;
    Engine *engine;
    Dispatcher *dispatcher;

    // Pointer to Engine data
    void *requestPtr;

    Request *request;
    Response *response;
    Action *action = nullptr;
    View *view = nullptr;
    QStack<Component *> stack;
    QList<Plugin *> plugins;
    QStringList error;
    QVariantHash stash;
    Stats *stats;
    bool detached = false;
    bool state = false;
};

}

#endif // CUTELYST_P_H
