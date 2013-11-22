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

#ifndef CUTELYSTENGINE_H
#define CUTELYSTENGINE_H

#include <QObject>
#include <QHostAddress>

class Cutelyst;
class CutelystRequest;
class CutelystResponse;
class CutelystEnginePrivate;
class CutelystEngine : public QObject
{
    Q_OBJECT
public:
    explicit CutelystEngine(QObject *parent = 0);
    ~CutelystEngine();

    CutelystRequest* request() const;
    virtual quint16 peerPort() const = 0;
    virtual QString peerName() const = 0;
    virtual QHostAddress peerAddress() const = 0;

    virtual bool init() = 0;

    void finalizeCookies(Cutelyst *c);
    virtual void finalizeHeaders(Cutelyst *c) = 0;
    virtual void finalizeBody(Cutelyst *c) = 0;
    void finalizeError(Cutelyst *c);

Q_SIGNALS:
    void handleRequest(CutelystRequest *request, CutelystResponse *response);

protected:
    virtual void parse(const QByteArray &data) = 0;
    CutelystRequest *createRequest(const QUrl &url,
                                   const QString &method,
                                   const QString &protocol,
                                   const QHash<QString, QByteArray> &headers,
                                   const QByteArray &body) const;

    CutelystEnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystEngine)
};

#endif // CUTELYSTENGINE_H
