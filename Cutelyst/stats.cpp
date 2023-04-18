/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "common.h"
#include "dispatchtype.h"
#include "enginerequest.h"
#include "stats_p.h"
#include "utils.h"

#include <QtCore/QStringList>

using namespace Cutelyst;

Stats::Stats(EngineRequest *request)
    : d_ptr(new StatsPrivate)
{
    Q_D(Stats);
    d->engineRequest = request;
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
    stat.begin  = d->engineRequest->elapsed.nsecsElapsed();
    d->actions.push_back(stat);
}

void Stats::profileEnd(const QString &action)
{
    Q_D(Stats);
    for (auto &stat : d->actions) {
        if (stat.action == action) {
            stat.end = d->engineRequest->elapsed.nsecsElapsed();
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

    QVector<QStringList> table;
    for (const auto &stat : d->actions) {
        table.append({stat.action,
                      QString::number((stat.end - stat.begin) / 1000000000.0, 'f') + QLatin1Char('s')});
    }

    ret = Utils::buildTable(table, {QStringLiteral("Action"), QStringLiteral("Time")});
    return ret;
}
