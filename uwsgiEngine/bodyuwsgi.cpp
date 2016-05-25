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

bool BodyUWSGI::seek(qint64 off)
{
    QIODevice::seek(off);
    uwsgi_request_body_seek(m_request, off);
    return true;
}

void BodyUWSGI::close()
{
    // Don't do anything
}

qint64 BodyUWSGI::readData(char *data, qint64 maxlen)
{
    ssize_t rlen = 0;
    char *buf = uwsgi_request_body_read(m_request, maxlen, &rlen);

    if (buf == uwsgi.empty) {
        return 0;
    } else if (buf == NULL) {
        return -1;
    }

    memcpy(data, buf, rlen);

    return rlen;
}

qint64 BodyUWSGI::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}

#include "moc_bodyuwsgi.cpp"
