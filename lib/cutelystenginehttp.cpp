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
#include "cutelystrequest_p.h"

#include <QStringList>

CutelystEngineHttp::CutelystEngineHttp(int socket, QObject *parent) :
    CutelystEngine(socket, parent),
    m_bufLastIndex(0)
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

            QRegExp methodProtocolRE("(\\w+)\\s+(.*)(?:\\s+(HTTP.*))$");
            QRegExp methodRE("(\\w+)\\s+(.*)");
            bool badRequest = false;
            if (methodProtocolRE.indexIn(section) != -1) {
                qDebug() << 1 <<  methodProtocolRE.capturedTexts() << methodProtocolRE.captureCount();
                if (methodProtocolRE.captureCount() == 3) {
                    m_method = methodProtocolRE.cap(1);
                    m_args = methodProtocolRE.cap(2).split(QLatin1String("/"));
                    m_protocol = methodProtocolRE.cap(3);
                } else {
                    badRequest = true;
                }
            } else if (methodRE.indexIn(section) != -1) {
                qDebug() << 2 << methodRE.capturedTexts() << methodRE.captureCount();
                if (methodRE.captureCount() == 2) {
                    m_method = methodRE.cap(1);
                    m_args = methodProtocolRE.cap(2).split(QLatin1String("/"));
                } else {
                    badRequest = true;
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
            qDebug() << section << section.length();
            qDebug() << section.section(QLatin1Char(':'), 0, 0) << section.section(QLatin1Char(':'), 1).trimmed() << endl;
        }
    }

    CutelystRequestPrivate *requestPriv = new CutelystRequestPrivate;
    requestPriv->engine = this;
    requestPriv->method = m_method;
    requestPriv->args = m_args;
    requestPriv->protocol = m_protocol;
    requestPriv->headers = m_headers;

    qDebug() << request;

    //    while (request.end())
    QByteArray data;
    data = "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "    <head>"
            "        <meta charset=\"utf-8\">"
            "        <title>Hello World</title>"
            "    </head>"
            "    <body>"
            "        <h1>Hello World</h1>"
            "        <p>"
            "            Jamie was here."
            "        </p>"
            "    </body>"
            "</html>";
    write(data);
    deleteLater();
}
