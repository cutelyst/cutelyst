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
#include "Plugin/plugin.h"

#include <QVariantHash>

namespace Cutelyst {

class ContextPrivate
{
public:
    ContextPrivate();
    ~ContextPrivate();

    Application *app;
    Engine *engine;
    Request *request;
    Response *response;
    Action *action;
    Dispatcher *dispatcher;
    QHash<Plugin::AbstractPlugin *, QVariantHash> plugins;
    bool detached;
    QByteArray match;
    QStringList error;
    QVariantHash stash;
    int status;
    bool state;
    QTime *stats;
};

}

#endif // CUTELYST_P_H
