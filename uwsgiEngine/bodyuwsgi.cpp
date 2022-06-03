/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "bodyuwsgi.h"

#include <uwsgi.h>

#include "engineuwsgi.h"

BodyUWSGI::BodyUWSGI(wsgi_request *request, bool sequential, QObject *parent) : QIODevice(parent)
  , m_request(request)
  , m_sequential(sequential)
{
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

bool BodyUWSGI::isSequential() const
{
    return m_sequential;
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
