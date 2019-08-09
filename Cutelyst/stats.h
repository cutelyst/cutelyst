/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef STATS_H
#define STATS_H

#include <QObject>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class EngineRequest;
class StatsPrivate;
class Stats
{
    Q_DECLARE_PRIVATE(Stats)
public:
    /**
     * Constructs a new stats object with the given parent.
     */
    explicit Stats(EngineRequest *request);
    virtual ~Stats();

    /**
     * Called before an action is executed to start counting it's time
     */
    virtual void profileStart(const QString &action);

    /**
     * Called after an action is executed to stop counting it's time
     */
    virtual void profileEnd(const QString &action);

    /**
     * Returns a text report of collected timmings
     */
    virtual QByteArray report();

protected:
    friend class Prometheus;
    StatsPrivate *d_ptr;
};

}

#endif // STATS_H
