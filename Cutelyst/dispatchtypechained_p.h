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
    QList<QStringView> captures;
    QList<QStringView> parts;
    int n_pathParts = 0;
    bool isNull     = true;
};

class DispatchTypeChainedPrivate
{
public:
    BestActionMatch recurseMatch(int reqArgsSize,
                                 const QString &parent,
                                 const QList<QStringView> &pathParts) const;
    bool checkArgsAttr(const Action *action, const QString &name) const;
    static QString listExtraHttpMethods(const Action *action);
    static QString listExtraConsumes(const Action *action);

    Actions endPoints;
    StringActionMap actions;
    StringStringActionsMap childrenOf;
};

} // namespace Cutelyst

#endif // DISPATCHTYPECHAINED_P_H
