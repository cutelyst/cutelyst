/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
