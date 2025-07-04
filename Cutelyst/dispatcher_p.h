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
    explicit DispatcherPrivate(Dispatcher *q)
        : q_ptr(q)
    {
    }

    inline void prepareAction(Context *c, QStringView path) const;

    void printActions() const;
    inline ActionList getContainers(QStringView ns) const;
    inline Action *
        command2Action(const Context *c, QStringView command, const QStringList &args) const;
    inline Action *
        invokeAsPath(const Context *c, QStringView relativePath, const QStringList &args) const;

    static inline QString actionRel2Abs(const Context *c, QStringView path);

    struct ActionReplacement {
        QString name;
        Action *action = nullptr;
    };
    QMap<QStringView, ActionReplacement> actions;
    struct ActionListReplacement {
        QString name;
        ActionList actions;
    };
    QMap<QStringView, ActionListReplacement> actionContainer;
    ActionList rootActions;

    struct NameController {
        QString name;
        Controller *controller = nullptr;
    };
    QMap<QStringView, NameController> controllers;
    QVector<DispatchType *> dispatchers;
    Dispatcher *q_ptr;
};

} // namespace Cutelyst

#endif // CUTELYST_DISPATCHER_P_H
