/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef BODYUWSGI_H
#define BODYUWSGI_H

#include <QIODevice>

struct wsgi_request;

class BodyUWSGI : public QIODevice
{
    Q_OBJECT
public:
    explicit BodyUWSGI(struct wsgi_request *request, bool sequential, QObject *parent = 0);

    virtual bool isSequential() const override;

    virtual qint64 pos() const override;
    virtual qint64 size() const override;
    virtual bool seek(qint64 off) override;

    virtual void close() override;

protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char * data, qint64 maxSize) override;

private:
    wsgi_request *m_request;
    bool m_sequential;
};

#endif // BODYUWSGI_H
