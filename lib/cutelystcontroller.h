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

#ifndef CUTELYSTCONTROLLER_H
#define CUTELYSTCONTROLLER_H

#include <QObject>

#include "cutelystcontext.h"

class CutelystController : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("Controller", "")
public:
    Q_INVOKABLE explicit CutelystController(QObject *parent = 0);
    ~CutelystController();

};

Q_DECLARE_METATYPE(CutelystController*)

#endif // CUTELYSTCONTROLLER_H
