/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYSTPLUGIN_H
#define CUTELYSTPLUGIN_H

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

typedef QHash<QString, QString> CStringHash;

class Application;
class CUTELYST_LIBRARY Plugin : public QObject
{
    Q_OBJECT
public:
    Plugin(Application *parent);

    /**
     * Reimplement this if you need to connect to
     * the signals emitted from Cutelyst::Application
     */
    virtual bool setup(Application *app);
};

}

#endif // CUTELYSTPLUGIN_H
