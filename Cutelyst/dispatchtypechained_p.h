/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
    bool isNull = true;
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

}

#endif // DISPATCHTYPECHAINED_P_H
