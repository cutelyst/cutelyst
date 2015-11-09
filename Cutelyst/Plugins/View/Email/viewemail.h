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

#ifndef VIEWEMAIL_H
#define VIEWEMAIL_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewEmailPrivate;
/**
 * ViewEmail class is a Cutelyst View handler that returns stash
 * data in Email format.
 */
class CUTELYST_LIBRARY ViewEmail : public Cutelyst::View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewEmail)
public:
    explicit ViewEmail(QObject *parent, const QString &name = QString());
    virtual ~ViewEmail();

    virtual bool render(Cutelyst::Context *c) const Q_DECL_FINAL;

protected:
    ViewEmailPrivate *d_ptr;
};

}

#endif // VIEWEMAIL_H
