/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "plugin.h"

#include "application.h"
#include "context.h"

using namespace Cutelyst;

Plugin::Plugin(Application *parent)
    : QObject(parent)
{
}

bool Plugin::setup(Application *app)
{
    Q_UNUSED(app)
    return true;
}

#include "moc_plugin.cpp"
