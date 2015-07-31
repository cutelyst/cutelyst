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

#ifndef CUTELYST_P_H
#define CUTELYST_P_H

#include "context.h"
#include "plugin.h"

#include <QVariantHash>
#include <QStack>

namespace Cutelyst {

class Stats;
class ContextPrivate
{
public:
    QString statsStartExecute(Component *code);
    void statsFinishExecute(const QString &statsInfo);

    // Pointer to Engine data
    void *requestPtr = nullptr;

    Application *app;
    Engine *engine;
    Request *request;
    Response *response = nullptr;
    Action *action = nullptr;
    View *view = nullptr;
    QStack<Component *> stack;
    Dispatcher *dispatcher;
    QList<Plugin *> plugins;
    QStringList error;
    QVariantHash stash;
    Stats *stats = nullptr;
    bool detached = false;
    bool state = false;
    bool chunked = false;
    bool chunked_done = false;
};

}

#endif // CUTELYST_P_H
