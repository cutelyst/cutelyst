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

#ifndef CUTELYSTENGINEHTTP_H
#define CUTELYSTENGINEHTTP_H

#include "cutelystengine.h"

#include <QTcpSocket>
#include <QStringList>

class CutelystEngineHttp : public CutelystEngine
{
    Q_OBJECT
public:
    explicit CutelystEngineHttp(int socket, CutelystDispatcher *dispatcher, QObject *parent = 0);

    virtual void finalizeCookies(CutelystContext *c);
    virtual void finalizeHeaders(CutelystContext *c);
    virtual void finalizeBody(CutelystContext *c);
    virtual void finalizeError(CutelystContext *c);

protected:
    virtual void parse(const QByteArray &request);

private:
    QString statusString(quint16 status) const;

    QVariantHash m_data;
    QByteArray m_buffer;
    quint64 m_bufLastIndex;
    QString m_method;
    QString m_path;
    QString m_protocol;
    QHash<QString, QString> m_headers;
};

#endif // CUTELYSTENGINEHTTP_H
