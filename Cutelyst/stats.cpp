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

#include "stats_p.h"

#include "dispatchtype.h"
#include "application.h"
#include "engine.h"
#include "utils.h"

#include "common.h"

#include <QtCore/QStringList>
#include <QtCore/QStringBuilder>

using namespace Cutelyst;

Stats::Stats(Application *app) : QObject(app)
  , d_ptr(new StatsPrivate)
{
    Q_D(Stats);
    d->engine = app->engine();
}

Stats::~Stats()
{

}

void Stats::profileStart(const QString &action)
{
    Q_D(Stats);
    StatsAction stat;
    stat.action = action;
    stat.begin = d->engine->time();
    d->actions.append(stat);
}

void Stats::profileEnd(const QString &action)
{
    Q_D(Stats);
    for (int i = 0; i < d->actions.size(); ++i) {
        StatsAction &stat = d->actions[i];
        if (stat.action == action) {
            stat.end = d->engine->time();
            break;
        }
    }
}

QString Stats::report()
{
    Q_D(Stats);

    if (d->actions.isEmpty()) {
        return QString();
    }

    QList<QStringList> table;
    Q_FOREACH (StatsAction stat, d->actions) {
        table.append({ stat.action,
                       QString::number((stat.end - stat.begin)/1000000.0, 'f') % QLatin1Char('s') });
    }

    return QString::fromLatin1(Utils::buildTable(table, {
                                                     QLatin1String("Action"), QLatin1String("Time")
                                                 }));
}
