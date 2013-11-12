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

#include "cutelystenginehttp.h"

#include "cutelystcontext.h"
#include "cutelystresponse.h"
#include "cutelystrequest_p.h"

#include <QStringList>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QDateTime>

CutelystEngineHttp::CutelystEngineHttp(int socket, CutelystDispatcher *dispatcher, QObject *parent) :
    CutelystEngine(socket, dispatcher, parent),
    m_bufLastIndex(0)
{
}

void CutelystEngineHttp::finalizeCookies(CutelystContext *c)
{

}

void CutelystEngineHttp::finalizeHeaders(CutelystContext *c)
{
    QByteArray header;
    header.append(QString::fromLatin1("HTTP/1.1 %1 %2\r\n").arg(QString::number(c->response()->status()),
                                                                c->response()->statusString()));

    QMap<QString, QString> headers = c->response()->headers();
    headers.insert(QLatin1String("Date"), QDateTime::currentDateTime().toString(Qt::ISODate));
    headers.insert(QLatin1String("Server"), QLatin1String("CutelystEngineHttp"));

    QMap<QString, QString>::ConstIterator it = headers.constBegin();
    while (it != headers.constEnd()) {
        header.append(it.key() % QLatin1String(": ") % it.value() % QLatin1String("\r\n"));
        ++it;
    }
    header.append(QLatin1String("\r\n"));

    write(header);
}

void CutelystEngineHttp::finalizeBody(CutelystContext *c)
{
    write(c->response()->body());
    deleteLater();
}

void CutelystEngineHttp::finalizeError(CutelystContext *c)
{

}

void CutelystEngineHttp::parse(const QByteArray &request)
{
    m_buffer.append(request);

    int newLine;
    if (m_method.isEmpty()) {
        if ((newLine = request.indexOf('\n', m_bufLastIndex)) != -1) {
            QByteArray section = request.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
            m_bufLastIndex = newLine + 1;

            QRegularExpression methodProtocolRE("(\\w+)\\s+(.*)(?:\\s+(HTTP.*))$");
            QRegularExpression methodRE("(\\w+)\\s+(.*)");
            bool badRequest = false;
            QRegularExpressionMatch match = methodProtocolRE.match(section);
            if (match.hasMatch()) {
                m_method = match.captured(1);
                m_path = match.captured(2);
                m_protocol = match.captured(3);
            } else {
                match = methodRE.match(section);
                if (match.hasMatch()) {
                    m_method = match.captured(1);
                    m_path = match.captured(2);
                }
            }

            if (badRequest) {
                qDebug() << "BAD REQUEST" << request;
                return;
            }
        }
    }

    while ((newLine = request.indexOf('\n', m_bufLastIndex)) != -1) {
        QString section = request.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
        m_bufLastIndex = newLine + 1;

        if (!section.isEmpty()) {
            m_headers[section.section(QLatin1Char(':'), 0, 0)] = section.section(QLatin1Char(':'), 1).trimmed();
        }
    }

    CutelystRequestPrivate *requestPriv = new CutelystRequestPrivate;
    requestPriv->engine = this;
    requestPriv->method = m_method;
    requestPriv->path = m_path;
    requestPriv->protocol = m_protocol;
    requestPriv->headers = m_headers;

    CutelystRequest *req = new CutelystRequest(requestPriv);

    qDebug() << request;

    handleRequest(req);
}

QString CutelystEngineHttp::statusText(quint16 status)
{
    switch (status) {
    case 200:
        return QLatin1String("OK");
    case 400:
        return QLatin1String("Bad Request");
    case 404:
        return QLatin1String("Not Found");
    default:
        return QLatin1String("Unknown");
    }

}
