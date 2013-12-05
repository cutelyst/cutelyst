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

#ifndef CUTELYSTCHILDPROCESS_P_H
#define CUTELYSTCHILDPROCESS_P_H

#include "cutelystchildprocess.h"
#include "cutelystdispatcher.h"

#include <QSocketNotifier>

namespace Cutelyst {

class CutelystChildProcessPrivate
{
    Q_DECLARE_PUBLIC(CutelystChildProcess)
public:
    CutelystChildProcessPrivate(CutelystChildProcess *parent);
    ~CutelystChildProcessPrivate();

    void gotFD(int socket);
    ssize_t sendFD(int sock, void *buf, ssize_t buflen, int fd);
    ssize_t readFD(int sock, void *buf, ssize_t bufsize, int *fd);

    CutelystChildProcess *q_ptr;
    QSocketNotifier *notifier;
    Dispatcher *dispatcher;
    QString error;
    int childFD;
    int parentFD;
    int childPID;
};

}

#endif // CUTELYSTCHILDPROCESS_P_H
