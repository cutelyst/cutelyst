/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CONTROLLER_P_H
#define CONTROLLER_P_H

#include "controller.h"
#include "component.h"

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
    Action* actionClass(const QVariantHash &args);
    Action* createAction(const QVariantHash &args, const QMetaMethod &method, Controller *controller, Application *app);
    void registerActionMethods(const QMetaObject *meta, Controller *controller, Application *app);
    QMap<QString, QString> parseAttributes(const QMetaMethod &method, const QByteArray &str, const QByteArray &name);
    QStack<Component *> gatherActionRoles(const QVariantHash &args);
    QString parsePathAttr(const QString &value);
    QString parseChainedAttr(const QString &attr);

    QObject *instantiateClass(const QString &name, const QByteArray &super);
    bool superIsClassName(const QMetaObject *super, const QByteArray &className);

    QString pathPrefix;
    ActionList beginAutoList;
    Action *end = nullptr;
    Application *application = nullptr;
    Controller *q_ptr;
    Dispatcher *dispatcher = nullptr;
    QMap<QString, Action *> actions;
    ActionList actionList;
    bool parsedActions = false;
};

}

#endif // CONTROLLER_P_H
