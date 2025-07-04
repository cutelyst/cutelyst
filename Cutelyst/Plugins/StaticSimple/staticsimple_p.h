/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICSIMPLE_P_H
#define STATICSIMPLE_P_H

#include "staticsimple.h"

#include <QDir>
#include <QRegularExpression>

using namespace Qt::StringLiterals;

namespace Cutelyst {

class StaticSimplePrivate
{
public:
    QVector<QDir> includePaths;
    QStringList dirs;
    QRegularExpression re = QRegularExpression(u"\\.[^/]+$"_s);
    bool serveDirsOnly{false};
};

} // namespace Cutelyst

#endif // STATICSIMPLE_P_
