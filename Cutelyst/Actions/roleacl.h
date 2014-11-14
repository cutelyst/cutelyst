/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef ROLEACL_H
#define ROLEACL_H

#include <QVariantHash>

#include <Cutelyst/does.h>
#include <Cutelyst/Context>

namespace Cutelyst {

class RoleACLPrivate;
class RoleACL : public Does
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RoleACL)
public:
    Q_INVOKABLE RoleACL();
    virtual ~RoleACL();

    virtual Modifiers modifiers() const;

    virtual bool init(Application *application, const QVariantHash &args);

    virtual bool aroundExecute(Context *ctx, DoesCode code);

    bool canVisit(Context *ctx) const;

protected:
    virtual bool dispatcherReady(const Dispatcher *dispatcher, Controller *controller);

    RoleACLPrivate *d_ptr;
};

}

#endif // ROLEACL_H
