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

#ifndef CUTELYST_H
#define CUTELYST_H

#include <QObject>
#include <QStringList>

class CutelystAction;
class CutelystRequest;
class CutelystResponse;
class CutelystDispatcher;
class CutelystContextPrivate;
class CutelystContext : public QObject
{
    Q_OBJECT
public:
    explicit CutelystContext(QObject *parent = 0);
    ~CutelystContext();

    bool error() const;
    bool state() const;
    QStringList args() const;
    QString uriPrefix() const;
    CutelystRequest *request() const;
    CutelystRequest *req() const;
    CutelystResponse *response() const;
    CutelystAction *action() const;
    CutelystDispatcher *dispatcher() const;
    QString ns() const;
    QString match() const;

    QVariantHash* stash();

    void dispatch();
    bool forward(const QString &action, const QStringList &arguments = QStringList());
    CutelystAction *getAction(const QString &action, const QString &ns = QString());
    QList<CutelystAction*> getActions(const QString &action, const QString &ns = QString());

protected:
    friend class CutelystEngine;
    friend class CutelystDispatchType;
    CutelystContextPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystContext)
};

#endif // CUTELYST_H
