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

#ifndef CUTELYST_DISPATCHER_H
#define CUTELYST_DISPATCHER_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include "cutelystaction.h"

namespace Cutelyst {

class Context;
class Controller;
class CutelystDispatchType;
class DispatcherPrivate;
class Dispatcher : public QObject
{
    Q_OBJECT
public:
    explicit Dispatcher(QObject *parent = 0);
    ~Dispatcher();
    void setupActions();

    bool dispatch(Context *ctx);
    bool forward(Context *ctx, const QString &opname, const QStringList &arguments);
    void prepareAction(Context *ctx);
    Action* getAction(const QString &name, const QString &ns = QString()) const;
    ActionList getActions(const QString &name, const QString &ns) const;
    QHash<QString, Controller*> controllers() const;
    QString uriForAction(Action *action, const QStringList &captures);

private:
    void printActions();
    Action *command2Action(Context *ctx, const QString &command, const QStringList &extraParams = QStringList());
    QStringList unexcapedArgs(const QStringList &args);
    QString actionRel2Abs(Context *ctx, const QString &path);
    Action *invokeAsPath(Context *ctx, const QString &relativePath, const QStringList &args);
    QString cleanNamespace(const QString &ns) const;

protected:
    DispatcherPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Dispatcher)
};

}

#endif // CUTELYST_DISPATCHER_H
