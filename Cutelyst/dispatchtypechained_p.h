/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DISPATCHTYPECHAINED_P_H
#define DISPATCHTYPECHAINED_P_H

#include "dispatchtypechained.h"

#include <vector>

namespace Cutelyst {

typedef QHash<QString, Action *> StringActionMap;
typedef std::vector<Action *> Actions;
typedef QHash<QString, Actions> StringActionsMap;
typedef QHash<QString, StringActionsMap> StringStringActionsMap;

struct BestActionMatch {
    ActionList actions;
    QStringList captures;
    QStringList parts;
    int n_pathParts = 0;
    bool isNull     = true;
};

class DispatchTypeChainedPrivate
{
public:
    BestActionMatch recurseMatch(int reqArgsSize, const QString &parent, const QStringList &pathParts) const;
    bool checkArgsAttr(Action *action, const QString &name) const;
    static QString listExtraHttpMethods(Action *action);
    static QString listExtraConsumes(Action *action);

    Actions endPoints;
    StringActionMap actions;
    StringStringActionsMap childrenOf;
};

} // namespace Cutelyst

#endif // DISPATCHTYPECHAINED_P_H
