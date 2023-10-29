/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATS_P_H
#define STATS_P_H

#include "stats.h"

#include <vector>

namespace Cutelyst {

struct StatsAction {
    QString action;
    std::chrono::time_point<std::chrono::steady_clock> begin;
    std::chrono::time_point<std::chrono::steady_clock> end;
};

class EngineRequest;
class StatsPrivate
{
public:
    std::vector<StatsAction> actions;
    EngineRequest *engineRequest;
};

} // namespace Cutelyst

#endif // STATS_P_H
