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
#include "request_p.h"

#include <QVariantHash>
#include <QStack>

namespace Cutelyst {

class Stats;
class ContextPrivate
{
public:
    inline ContextPrivate(Application *_app, Engine *_ngine, Dispatcher *_dispatcher, const QVector<Plugin *> &_plugins)
        : app(_app)
        , engine(_ngine)
        , dispatcher(_dispatcher)
        , plugins(_plugins)
    { }

    QString statsStartExecute(Component *code);
    void statsFinishExecute(const QString &statsInfo);

    QStringList error;
    QVariantHash stash;
    QStack<Component *> stack;
    QVector<Plugin *> plugins;

    Application *app;
    Engine *engine;
    Dispatcher *dispatcher;

    // Pointer to Engine data
    void *requestPtr;

    Request *request;
    Response *response;
    Action *action = nullptr;
    View *view = nullptr;
    Stats *stats = nullptr;
    bool detached = false;
    bool state = false;
};

}

#endif // CUTELYST_P_H
