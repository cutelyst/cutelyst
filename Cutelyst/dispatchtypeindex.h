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

#ifndef CUTELYST_DISPATCHTYPEINDEX_H
#define CUTELYST_DISPATCHTYPEINDEX_H

#include "dispatchtype.h"

namespace Cutelyst {

class DispatchTypeIndex : public CutelystDispatchType
{
    Q_OBJECT
public:
    explicit DispatchTypeIndex(QObject *parent = 0);

    /**
     * Check if there's an index action for a given path,
     * and set it up to use it if there is; only matches a
     * full URI - if c->req->args is already set this
     * DispatchType is guaranteed not to match.
     */
    virtual bool match(Context *ctx, const QString &path) const;

    virtual QString uriForAction(Action *action, const QStringList &captures) const;

    virtual bool isLowPrecedence() const;
};

}

#endif // CUTELYST_DISPATCHTYPEINDEX_H
