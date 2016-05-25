/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "bodybuffereduwsgi.h"
#include "engineuwsgi.h"

BodyBufferedUWSGI::BodyBufferedUWSGI(wsgi_request *request, QObject *parent) : QIODevice(parent)
  , m_request(request)
{
}

qint64 BodyBufferedUWSGI::pos() const
{
    return QIODevice::pos();
}

qint64 BodyBufferedUWSGI::size() const
{
    return qint64(m_request->post_cl);
}

bool BodyBufferedUWSGI::seek(qint64 pos)
{
    if (!m_filled) {
        fillBuffer();
    }

    if (pos > m_buf.size() || pos < 0) {
        qWarning("BodyBufferedUWSGI::seek: Invalid pos: %d", int(pos));
        return false;
    }
    return QIODevice::seek(pos);
}

void BodyBufferedUWSGI::close()
{
    m_filled = false;
    QIODevice::close();
}

qint64 BodyBufferedUWSGI::readData(char *data, qint64 len)
{
    if (!m_filled) {
        fillBuffer();
    }

    if ((len = qMin(len, qint64(m_buf.size()) - pos())) <= 0) {
        return qint64(0);
    }
    memcpy(data, m_buf.constData() + pos(), len);
    return len;
}

qint64 BodyBufferedUWSGI::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}

void BodyBufferedUWSGI::fillBuffer()
{
//    qCDebug(CUTELYST_UWSGI) << "Filling body buffer, size:" << m_request->post_cl;

    QByteArray buf;
    buf.reserve(m_request->post_cl);

    size_t remains = m_request->post_cl;
    while (remains > 0) {
        ssize_t body_len = 0; // it MUST be initted, for some reason...
        char *body_data =  uwsgi_request_body_read(m_request, remains, &body_len);
        if (!body_data || body_data == uwsgi.empty) {
            break;
        }
        buf.append(body_data, body_len);
        remains -= body_len;
    }

    m_buf = buf;
    m_filled = true;
}

#include "moc_bodybuffereduwsgi.cpp"
