/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DISPATCHTYPEPATH_P_H
#define DISPATCHTYPEPATH_P_H

#include "dispatchtypepath.h"

#include <vector>

namespace Cutelyst {

typedef std::vector<Action *> Actions;
typedef QHash<QString, Actions> StringActionsMap;

class DispatchTypePathPrivate
{
public:
    bool registerPath(const QString &path, Action *action);

    StringActionsMap paths;
};

} // namespace Cutelyst

#endif // DISPATCHTYPEPATH_P_H
