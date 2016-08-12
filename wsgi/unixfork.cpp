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
#include "unixfork.h"

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <QDebug>

UnixFork::UnixFork(QObject *parent) : QObject(parent)
{

}

bool UnixFork::createProcess(int process)
{
    for (int i = 0; i < process; ++i) {
        if (!createChild()) {
            return false;
        }
    }
    qDebug() << "Created workers" << process;
    return true;
}

bool UnixFork::createChild()
{
    qint64 childPID;

    childPID = fork();

    if(childPID >= 0) {
        if(childPID == 0) {
            Q_EMIT forked();
        } else {
            m_childs.push_back(childPID);
            return true;
        }
    } else {
        qFatal("Fork failed, quitting!!!!!!");
    }

    return false;
}
