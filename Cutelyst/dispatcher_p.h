/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_DISPATCHER_P_H
#define CUTELYST_DISPATCHER_P_H

#include "dispatcher.h"

namespace Cutelyst {

class DispatcherPrivate
{
    Q_DECLARE_PUBLIC(Dispatcher)
public:
    DispatcherPrivate(Dispatcher *q)
        : q_ptr(q)
    {
    }

    inline void prepareAction(Context *c, const QString &requestPath) const;

    void printActions() const;
    inline ActionList getContainers(const QString &ns) const;
    inline Action *command2Action(Context *c, const QString &command, const QStringList &args) const;
    inline Action *invokeAsPath(Context *c, const QString &relativePath, const QStringList &args) const;

    static inline QString actionRel2Abs(Context *c, const QString &path);
    static inline QString cleanNamespace(const QString &ns);
    static inline QString normalizePath(const QString &path);

    struct Replacement {
        QString name;
        Action *action = nullptr;
    };
    QMap<QStringView, Replacement> actions;
    QMap<QString, ActionList> actionContainer;
    ActionList rootActions;
    QMap<QString, Controller *> controllers;
    QVector<DispatchType *> dispatchers;
    Dispatcher *q_ptr;
};

} // namespace Cutelyst

#endif // CUTELYST_DISPATCHER_P_H
