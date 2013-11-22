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

#include "cutelystpluginsession.h"
#include "cutelystapplication.h"
#include "cutelystrequest.h"
#include "cutelyst.h"

#include <QDebug>

CutelystPluginSession::CutelystPluginSession(QObject *parent) :
    CutelystPlugin(parent)
{
}

bool CutelystPluginSession::setup(CutelystApplication *app)
{
    connect(app, &CutelystApplication::beforeDispatch,
            this, &CutelystPluginSession::restoreSession);
    connect(app, &CutelystApplication::afterDispatch,
            this, &CutelystPluginSession::saveSession);
}

void CutelystPluginSession::restoreSession(Cutelyst *c)
{
    qDebug() << Q_FUNC_INFO << c->req()->cookies();
}

void CutelystPluginSession::saveSession(Cutelyst *c)
{
    qDebug() << Q_FUNC_INFO << c->stash();
}
