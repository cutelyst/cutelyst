/*
 * SPDX-FileCopyrightText: (C) 2014-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once
#include "dispatchtypepath.h"

#include <vector>

namespace Cutelyst {

typedef std::vector<Action *> Actions;
struct DispatchTypePathReplacement {
    QString name;
    Actions actions;
};
typedef QHash<QStringView, DispatchTypePathReplacement> StringActionsMap;

class DispatchTypePathPrivate
{
public:
    bool registerPath(const QString &path, Action *action);

    StringActionsMap paths;
};

} // namespace Cutelyst
