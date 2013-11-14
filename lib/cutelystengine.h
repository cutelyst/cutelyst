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

class CutelystContext;
class CutelystDispatcher;
class CutelystRequest;
class CutelystEnginePrivate;
class CutelystEngine : public QObject
{
    Q_OBJECT
public:
    explicit CutelystEngine(int socket, CutelystDispatcher *dispatcher, QObject *parent = 0);
    ~CutelystEngine();

    bool isValid() const;
    CutelystRequest* request() const;
    quint16 peerPort() const;
    QString peerName() const;
    QHostAddress peerAddress() const;

    void finalizeCookies(CutelystContext *c);
    virtual void finalizeHeaders(CutelystContext *c) = 0;
    virtual void finalizeBody(CutelystContext *c) = 0;
    virtual void finalizeError(CutelystContext *c) = 0;

protected:
    virtual void parse(const QByteArray &data) = 0;
    qint64 write(const QByteArray &data);
    void handleRequest(CutelystRequest *request);
    CutelystRequest *createRequest(const QUrl &url,
                                   const QString &method,
                                   const QString &protocol,
                                   const QHash<QString, QByteArray> &headers) const;

    CutelystEnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystEngine)

    void readyRead();
};

#endif // CUTELYSTENGINE_H
