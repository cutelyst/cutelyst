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

#ifndef CUTELYST_APPLICATION_P_H
#define CUTELYST_APPLICATION_P_H

#include "application.h"
#include "dispatcher.h"
#include "engine.h"
#include "plugin.h"
#include "context.h"
#include "componentfactory.h"

namespace Cutelyst {

class ApplicationPrivate
{
    Q_DECLARE_PUBLIC(Application)
public:
    void setupHome();
    void setupChildren(const QObjectList &children);

    void logRequest(Request *req);
    void logRequestParameters(const ParamsMultiMap &params, const QString &title);
    void logRequestUploads(const QMap<QString, Upload *> &uploads);

    Application *q_ptr;
    Dispatcher *dispatcher;
    QList<Plugin *> plugins;
    QMap<QString, Controller *> controllers;
    QMap<QString, View *> views;
    QList<DispatchType *> dispatchers;
    QMap<QString, ComponentFactory *> factories;
    Headers headers;
    QVariantMap config;
    Engine *engine;
    bool useStats;
    bool init = false;
};

}

#endif // CUTELYST_APPLICATION_P_H
