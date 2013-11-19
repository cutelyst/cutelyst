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
class CutelystEngine;
class CutelystRequest;
class CutelystResponse;
class CutelystDispatcher;
class CutelystController;
class CutelystPrivate;
class Cutelyst : public QObject
{
    Q_OBJECT
public:
    Cutelyst(CutelystEngine *engine, CutelystDispatcher *dispatcher);
    ~Cutelyst();

    bool error() const;

    /**
     * Contains the return value of the last executed action.
     */
    bool state() const;
    void setState(bool state);
    QStringList args() const;
    QString uriPrefix() const;
    CutelystEngine *engine() const;
    CutelystRequest *request() const;
    CutelystRequest *req() const;
    CutelystResponse *response() const;
    CutelystAction *action() const;
    CutelystDispatcher *dispatcher() const;
    CutelystController *controller(const QString &name = QString()) const;
    QString ns() const;
    QString match() const;

    QVariantHash* stash();

    bool dispatch();
    bool detached() const;
    void detach();
    bool forward(const QString &action, const QStringList &arguments = QStringList());
    CutelystAction *getAction(const QString &action, const QString &ns = QString());
    QList<CutelystAction*> getActions(const QString &action, const QString &ns = QString());

protected:
    void handleRequest(CutelystRequest *req, CutelystResponse *resp);
    void finalizeHeaders();
    void finalizeCookies();
    void finalizeBody();
    void finalizeError();
    int finalize();

    friend class CutelystEngine;
    friend class CutelystDispatchType;
    CutelystPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Cutelyst)
};

#endif // CUTELYST_H
