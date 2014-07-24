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

#include <Cutelyst/Action>

namespace Cutelyst {

class Context;
class Controller;
class DispatchType;
class DispatcherPrivate;
class Dispatcher : public QObject
{
    Q_OBJECT
public:
    explicit Dispatcher(QObject *parent = 0);
    ~Dispatcher();

protected:
    void setupActions(const QList<Controller *> &controllers);

    bool dispatch(Context *ctx);
    bool forward(Context *ctx, const QByteArray &opname, const QStringList &arguments = QStringList());
    void prepareAction(Context *ctx);
    Action* getAction(const QByteArray &name, const QByteArray &ns = QByteArray()) const;
    ActionList getActions(const QByteArray &name, const QByteArray &ns) const;
    QHash<QByteArray, Controller *> controllers() const;
    QByteArray uriForAction(Action *action, const QStringList &captures);
    void registerDispatchType(DispatchType *dispatchType);

private:
    QByteArray printActions();
    Action *command2Action(Context *ctx, const QByteArray &command, const QStringList &extraParams = QStringList());
    QByteArray actionRel2Abs(Context *ctx, const QByteArray &path);
    Action *invokeAsPath(Context *ctx, const QByteArray &relativePath, const QStringList &args);
    inline QByteArray cleanNamespace(const QByteArray &ns) const;

protected:
    friend class Application;
    friend class Context;
    friend class Controller;
    DispatcherPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Dispatcher)
};

}

#endif // CUTELYST_DISPATCHER_H
