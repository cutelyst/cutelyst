/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "bodyuwsgi.h"

#include "engineuwsgi.h"

BodyUWSGI::BodyUWSGI(wsgi_request *request, QObject *parent) :
    QIODevice(parent),
    m_request(request)
{
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

qint64 BodyUWSGI::pos() const
{
    return m_request->post_pos;
}

qint64 BodyUWSGI::size() const
{
    // Content-Length
    return m_request->post_cl;
}

bool BodyUWSGI::seek(qint64 pos)
{
    uwsgi_request_body_seek(m_request, pos);
    return true;
}

qint64 BodyUWSGI::readData(char *data, qint64 maxlen)
{
    ssize_t body_len = 0;
    char *body = uwsgi_request_body_read(m_request, maxlen, &body_len);
    qstrncpy(data, body, body_len);
    return body_len;
}

qint64 BodyUWSGI::readLineData(char *data, qint64 maxlen)
{
    ssize_t body_len = 0;
    char *body = uwsgi_request_body_readline(m_request, maxlen, &body_len);
    qstrncpy(data, body, body_len);
    return body_len;
}

qint64 BodyUWSGI::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}
