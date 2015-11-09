/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "viewemail_p.h"

#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

using namespace Cutelyst;

ViewEmail::ViewEmail(QObject *parent, const QString &name) : View(parent, name)
  , d_ptr(new ViewEmailPrivate)
{

}

ViewEmail::~ViewEmail()
{
    delete d_ptr;
}

bool ViewEmail::render(Context *c) const
{
    Q_D(const ViewEmail);

    QVariantHash email = c->stash(QStringLiteral("email")).toHash();



    return true;
}
