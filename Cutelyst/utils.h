/*
 * SPDX-FileCopyrightText: (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/cutelyst_global.h>
#include <chrono>

#include <QtCore/QStringList>

namespace Cutelyst {

/**
 * \ingroup core
 * \brief Helper functions used all over %Cutelyst.
 *
 * Helper functions used all over %Cutelyst.
 *
 * \par Include statement
 * #include <Cutelyst/utils.h>
 */
namespace Utils {
CUTELYST_LIBRARY QByteArray buildTable(const QVector<QStringList> &table,
                                       const QStringList &headers = {},
                                       const QString &title       = {});

CUTELYST_LIBRARY QString decodePercentEncoding(QString *s);

CUTELYST_LIBRARY ParamsMultiMap decodePercentEncoding(char *data, int len);

CUTELYST_LIBRARY QString decodePercentEncoding(QByteArray *ba);

/**
 * Reads a time span from @a str and parses it into a duration value.
 *
 * The following time units are understood:
 * @li usec, us
 * @li msec, ms
 * @li seconds, second, sec, s
 * @li minutes, minute, min, m
 * @li hours, hour, hr, h
 * @li days, day, d
 * @li weeks, week, w
 * @li months, month, M
 * @li years, year, y
 *
 * If no time unit is specified, seconds are assumed.
 *
 * Examples for valid time span specifications:
 * @li 2 h
 * @li 2hours
 * @li 48hr
 * @li 1y 12month
 * @li 55s500ms
 * @li 300ms20s 5day
 *
 * If @a ok is not @c nullptr, failure is reported by setting @a *ok to @c false,
 * and success by setting @a *ok to @c true.
 */
CUTELYST_LIBRARY std::chrono::microseconds durationFromString(QStringView str, bool *ok = nullptr);
} // namespace Utils

} // namespace Cutelyst
