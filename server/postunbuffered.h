/*
 * SPDX-FileCopyrightText: (C) 2016-2019 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef POSTUNBUFFERED_H
#define POSTUNBUFFERED_H

#include <QIODevice>

class PostUnbuffered : public QIODevice
{
    Q_OBJECT
public:
    explicit PostUnbuffered(QObject *parent = nullptr);

    qint64 m_contentLength = 0;
};

#endif // POSTUNBUFFERED_H
