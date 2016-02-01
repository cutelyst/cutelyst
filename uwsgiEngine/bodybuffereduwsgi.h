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

#ifndef BODYBUFFEREDUWSGI_H
#define BODYBUFFEREDUWSGI_H

#include <QIODevice>
#include <QBuffer>

struct wsgi_request;

class BodyBufferedUWSGI : public QIODevice
{
    Q_OBJECT
public:
    explicit BodyBufferedUWSGI(struct wsgi_request *request, QObject *parent = 0);

    virtual qint64 pos() const;
    virtual qint64 size() const;
    virtual bool seek(qint64 off);

    virtual void close();

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 readLineData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char * data, qint64 maxSize);

private:
    void fillBuffer();

    wsgi_request *m_request;
    mutable QBuffer *m_buffer;
    mutable bool m_filled = false;
};

#endif // BODYBUFFEREDUWSGI_H
