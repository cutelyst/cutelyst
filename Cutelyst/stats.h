/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef STATS_H
#define STATS_H

#include <QObject>

namespace Cutelyst {

class StatsPrivate;
class Stats : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Stats)
public:
    explicit Stats(QObject *parent = 0);
    virtual ~Stats();

    void profileStart(const QString &action, const QString &parent);
    void profileEnd(const QString &action);
    QByteArray report();

protected:
    StatsPrivate *d_ptr;
};

}

#endif // STATS_H
