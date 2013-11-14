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

#ifndef CUTELYSTCONTEXT_P_H
#define CUTELYSTCONTEXT_P_H

#include "cutelystcontext.h"

class CutelystContextPrivate
{
//    Q_DECLARE_PUBLIC(CutelystContext)
public:
    CutelystContextPrivate(CutelystContext *parent);

    CutelystEngine *engine;
    CutelystRequest *request;
    CutelystResponse *response;
    CutelystAction *action;
    CutelystDispatcher *dispatcher;
    bool detached;
    QString match;
    int status;
    bool state;
};

#endif // CUTELYSTCONTEXT_P_H
