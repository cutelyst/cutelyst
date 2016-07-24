/*
 * Copyright (C) 2015-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "stats_p.h"

#include "dispatchtype.h"
#include "application.h"
#include "engine.h"
#include "utils.h"

#include "common.h"

#include <QtCore/QStringList>

using namespace Cutelyst;

Stats::Stats(Application *app) : d_ptr(new StatsPrivate)
{
    Q_D(Stats);
    d->engine = app->engine();
}

Stats::~Stats()
{
    delete d_ptr;
}

void Stats::profileStart(const QString &action)
{
    Q_D(Stats);
    StatsAction stat;
    stat.action = action;
    stat.begin = d->engine->time();
    d->actions.push_back(stat);
}

void Stats::profileEnd(const QString &action)
{
    Q_D(Stats);
    for (auto &stat : d->actions) {
        if (stat.action == action) {
            stat.end = d->engine->time();
            break;
        }
    }
}

QByteArray Stats::report()
{
    Q_D(const Stats);

    QByteArray ret;
    if (d->actions.size() == 0) {
        return ret;
    }

    QList<QStringList> table;
    for (const auto &stat : d->actions) {
        table.append({ stat.action,
                       QString::number((stat.end - stat.begin)/1000000.0, 'f') + QLatin1Char('s') });
    }

    ret = Utils::buildTable(table, {
                                QStringLiteral("Action"), QStringLiteral("Time")
                            });
    return ret;
}
