/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "root.h"

#include <Cutelyst/Context>

Root::Root(QObject *app)
    : Controller(app)
{
}

Root::~Root()
{
}

void Root::index(Context *c)
{
    // In order to easy the logic on the gui class we will
    // emit the Context, this would not work if the signal
    // connection was Qt::QueueConnection, in such case
    // Cutelyst::Async would need to be used
    Q_EMIT indexCalled(c);
}
