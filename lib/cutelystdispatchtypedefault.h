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

#ifndef CUTELYSTDISPATCHTYPEDEFAULT_H
#define CUTELYSTDISPATCHTYPEDEFAULT_H

#include "cutelystdispatchtype.h"

class CutelystDispatchTypeDefault : public CutelystDispatchType
{
    Q_OBJECT
public:
    explicit CutelystDispatchTypeDefault(QObject *parent = 0);

    /**
     * If path is empty (i.e. all path parts have been converted
     * into args), attempts to find a default for the namespace
     * constructed from the args, or the last inherited default
     * otherwise and will match that.
     *
     * If path is not empty, never matches since Default will
     * only match if all other possibilities have been exhausted.
     */
    virtual bool match(CutelystContext *c, const QString &path) const;

    virtual bool isLowPrecedence() const;
};

#endif // CUTELYSTDISPATCHTYPEDEFAULT_H
