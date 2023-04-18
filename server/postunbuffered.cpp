/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "postunbuffered.h"

PostUnbuffered::PostUnbuffered(QObject *parent)
    : QIODevice(parent)
{
}

#include "moc_postunbuffered.cpp"
