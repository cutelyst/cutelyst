/*
 * SPDX-FileCopyrightText: (C) 2015-2025 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "stats.h"

#include "application.h"
#include "common.h"
#include "dispatchtype.h"
#include "enginerequest.h"
#include "stats_p.h"
#include "utils.h"

#include <QtCore/QStringList>

using namespace Cutelyst;
using namespace Qt::StringLiterals;

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
    stat.begin  = std::chrono::steady_clock::now();
    d->actions.push_back(stat);
}

void Stats::profileEnd(const QString &action)
{
    Q_D(Stats);
    auto it = std::ranges::find_if(d->actions,
                                   [action](const auto &stat) { return stat.action == action; });

    if (it != d->actions.end()) {
        it->end = std::chrono::steady_clock::now();
    }
}

QByteArray Stats::report()
{
    Q_D(const Stats);

    QByteArray ret;
    if (d->actions.empty()) {
        return ret;
    }

    QVector<QStringList> table;
    for (const auto &stat : d->actions) {
        const std::chrono::duration<double> duration = stat.end - stat.begin;
        table.append({stat.action, QString::number(duration.count(), 'f') + QLatin1Char('s')});
    }

    ret = Utils::buildTable(table, {u"Action"_s, u"Time"_s});
    return ret;
}
