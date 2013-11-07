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

#ifndef CUTELYSTDISPATCHER_H
#define CUTELYSTDISPATCHER_H

#include <QObject>
#include <QHash>

class CutelystContext;
class CutelystAction;
class CutelystDispatchType;
class CutelystDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit CutelystDispatcher(QObject *parent = 0);
    void setupActions();

    void dispatch(CutelystContext *c);
    void prepareAction(CutelystContext *c);
    CutelystAction* getAction(const QString &action, const QString &ns);
    QList<CutelystAction *> getActions(const QString &action, const QString &ns);

private:
    void printActions();
    QStringList unexcapedArgs(const QStringList &args);
    QHash<QString, CutelystAction*> m_actions;
    QList<CutelystDispatchType*> m_dispatchers;
};

#endif // CUTELYSTDISPATCHER_H
