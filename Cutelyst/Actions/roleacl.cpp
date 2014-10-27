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

#include "roleacl_p.h"

#include "common.h"

#include <Cutelyst/Plugin/authentication.h>
#include <Cutelyst/Controller>
#include <QMap>

using namespace Cutelyst;
using namespace Plugin;

RoleACL::RoleACL(const QVariantHash &args) :
    d_ptr(new RoleACLPrivate)
{
    Q_D(RoleACL);

    QMap<QByteArray, QByteArray> attributes;
    attributes = args.value("attributes").value<QMap<QByteArray, QByteArray> >();

    if (!attributes.contains("RequiresRole") || !attributes.contains("AllowedRole")) {
        qCritical() << "Action"
                    << args.value("reverse")
                    << "requires at least one RequiresRole or AllowedRole attribute";
    } else {
        QList<QByteArray> required = attributes.values("RequiresRole");
        Q_FOREACH (const QByteArray &role, required) {
            d->requiresRole.append(role);
        }

        QList<QByteArray> allowed = attributes.values("AllowedRole");
        Q_FOREACH (const QByteArray &role, allowed) {
            d->allowedRole.append(role);
        }
    }

    if (!attributes.contains("ACLDetachTo") && !attributes.value("ACLDetachTo").isEmpty()) {
        qCritical() << "Action"
                    << args.value("reverse")
                    << "requires the ACLDetachTo(<action>) attribute";
    } else {
        d->detachTo = args.value("ACLDetachTo").toByteArray();
    }
}

RoleACL::~RoleACL()
{
    delete d_ptr;
}

bool RoleACL::aroundExecute(Context *ctx, Action *orig)
{
    Q_D(RoleACL);

    if (canVisit(ctx)) {
        return orig->execute(ctx);
    }

    ctx->detach(ctx->controller()->actionFor(d->detachTo));

    return false;
}

bool RoleACL::canVisit(Context *ctx)
{
    Q_D(RoleACL);

    Plugin::Authentication *auth = ctx->plugin<Plugin::Authentication*>();
    if (auth) {
        QStringList user_has = auth->user().values(QStringLiteral("roles"));

        QStringList required = d->requiresRole;
        QStringList allowed = d->allowedRole;

        if (!required.isEmpty() && !allowed.isEmpty()) {
            Q_FOREACH (const QString &role, required) {
                if (!user_has.contains(role)) {
                    return false;
                }
            }
            Q_FOREACH (const QString &role, allowed) {
                if (user_has.contains(role)) {
                    return true;
                }
            }
            return false;
        }  else if (!required.isEmpty()) {
            Q_FOREACH (const QString &role, required) {
                if (!user_has.contains(role)) {
                    return false;
                }
            }
            return true;
        } else if (!allowed.isEmpty()) {
            Q_FOREACH (const QString &role, allowed) {
                if (user_has.contains(role)) {
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}
