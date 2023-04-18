/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATS_H
#define STATS_H

#include <Cutelyst/cutelyst_global.h>

#include <QObject>

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
    StatsPrivate *d_ptr;
};

} // namespace Cutelyst

#endif // STATS_H
