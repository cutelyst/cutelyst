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

#ifndef CUTELYST_DISPATCHER_P_H
#define CUTELYST_DISPATCHER_P_H

#include "dispatcher.h"

namespace Cutelyst {

class DispatcherPrivate
{
    Q_DECLARE_PUBLIC(Dispatcher)
public:
    DispatcherPrivate(Dispatcher *q) : q_ptr(q) {}

    inline void prepareAction(Context *c, const QString &requestPath) const;

    void printActions() const;
    inline ActionList getContainers(const QString &ns) const;
    inline Action *command2Action(Context *c, const QString &command, const QStringList &args) const;
    inline Action *invokeAsPath(Context *c, const QString &relativePath, const QStringList &args) const;

    static inline QString actionRel2Abs(Context *c, const QString &path);
    static inline QString cleanNamespace(const QString &ns);
    static inline QString normalizePath(const QString &path);

    QMap<QString, Action*> actions;
    QMap<QString, ActionList> actionContainer;
    ActionList rootActions;
    QMap<QString, Controller *> controllers;
    QVector<DispatchType*> dispatchers;
    Dispatcher *q_ptr;
};

}

#endif // CUTELYST_DISPATCHER_P_H
