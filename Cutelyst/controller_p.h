/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CONTROLLER_P_H
#define CONTROLLER_P_H

#include "component.h"
#include "controller.h"

namespace Cutelyst {

class ControllerPrivate
{
    Q_DECLARE_PUBLIC(Controller)
public:
    ControllerPrivate(Controller *parent);
    void init(Application *app, Dispatcher *_dispatcher);
    // Called when the Dispatcher has finished
    // setting up all controllers
    void setupFinished();
    Action *actionClass(const QVariantHash &args);
    Action *createAction(const QVariantHash &args, const QMetaMethod &method, Controller *controller, Application *app);
    void registerActionMethods(const QMetaObject *meta, Controller *controller, Application *app);
    ParamsMultiMap parseAttributes(const QMetaMethod &method, const QByteArray &str, const QByteArray &name);
    QStack<Component *> gatherActionRoles(const QVariantHash &args);
    QString parsePathAttr(const QString &value);
    QString parseChainedAttr(const QString &attr);

    QObject *instantiateClass(const QString &name, const QByteArray &super);
    bool superIsClassName(const QMetaObject *super, const QByteArray &className);

    QString pathPrefix;
    ActionList beginAutoList;
    Action *end              = nullptr;
    Application *application = nullptr;
    Controller *q_ptr;
    Dispatcher *dispatcher = nullptr;
    struct Replacement {
        QString name;
        Action *action = nullptr;
    };
    QMap<QStringView, Replacement> actions;
    ActionList actionList;
    bool parsedActions = false;
};

} // namespace Cutelyst

#endif // CONTROLLER_P_H
