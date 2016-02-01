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

BodyBufferedUWSGI::BodyBufferedUWSGI(wsgi_request *request, QObject *parent) :
    QIODevice(parent),
    m_request(request),
    m_buffer(new QBuffer(this))
{
}

qint64 BodyBufferedUWSGI::pos() const
{
    if (!m_filled) {
        return 0;
    }
    return m_buffer->pos();
}

qint64 BodyBufferedUWSGI::size() const
{
    return m_request->post_cl;
}

bool BodyBufferedUWSGI::seek(qint64 off)
{
    if (!m_filled) {
        fillBuffer();
    }

    if (m_buffer->seek(off)) {
        QIODevice::seek(off);
        return true;
    }
    return false;
}

void BodyBufferedUWSGI::close()
{
    m_filled = false;
    m_buffer->close();
    QIODevice::close();
}

qint64 BodyBufferedUWSGI::readData(char *data, qint64 maxlen)
{
    if (!m_filled) {
        fillBuffer();
    }
    return m_buffer->read(data, maxlen);
}

qint64 BodyBufferedUWSGI::readLineData(char *data, qint64 maxlen)
{
    if (!m_filled) {
        fillBuffer();
    }
    return m_buffer->readLine(data, maxlen);
}

qint64 BodyBufferedUWSGI::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    if (!m_filled) {
        fillBuffer();
    }
    return -1;
}

void BodyBufferedUWSGI::fillBuffer()
{
//    qCDebug(CUTELYST_UWSGI) << "Filling body buffer, size:" << m_request->post_cl;

    QByteArray buff;
    buff.reserve(m_request->post_cl);

    size_t remains = m_request->post_cl;
    while (remains > 0) {
        ssize_t body_len;
        char *body_data =  uwsgi_request_body_read(m_request, UMIN(remains, 4096) , &body_len);
        if (!body_data || body_data == uwsgi.empty) {
            break;
        }
        buff.append(body_data, body_len);
        remains -= body_len;
    }
    m_buffer->setData(buff);
    m_buffer->open(QIODevice::ReadOnly);

    m_filled = true;
}
