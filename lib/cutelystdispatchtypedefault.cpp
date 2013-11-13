/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "cutelystdispatchtypedefault.h"
#include "cutelystcontext.h"
#include "cutelystrequest.h"

CutelystDispatchTypeDefault::CutelystDispatchTypeDefault(QObject *parent) :
    CutelystDispatchType(parent)
{
}

bool CutelystDispatchTypeDefault::match(CutelystContext *c, const QString &path) const
{
    if (!path.isEmpty()) {
        return false;
    }

    QList<CutelystAction *> actions = c->getActions(QLatin1String("dispatchDefault"), c->req()->path());
    if (actions.size()) {
        CutelystAction *action = actions.last();
        setupMatchedAction(c, action, QString());

        return true;
    }
    return false;
}

bool CutelystDispatchTypeDefault::isLowPrecedence() const
{
    return true;
}
