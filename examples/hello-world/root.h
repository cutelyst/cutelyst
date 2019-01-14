/***************************************************************************
 *   Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    Root(QObject *app);
    ~Root();

public:
    C_ATTR(hello, :Path :AutoArgs)
    void hello(Context *c);

    C_ATTR(json, :Local :AutoArgs)
    void json(Context *c);

    C_ATTR(echo, :Local :AutoArgs)
    void echo(Context *c);

    C_ATTR(ws, :Local :AutoArgs)
    void ws(Context *c);

    C_ATTR(session, :Local :AutoArgs)
    void session(Context *c);

    C_ATTR(read_session, :Local :AutoArgs)
    void read_session(Context *c);

    C_ATTR(async, :Local :AutoArgs)
    void async(Context *c, const QString &timeout);

private Q_SLOTS:
    bool Auto(Context *c);

private:
    C_ATTR(End,)
    void End(Context *c);
};

#endif // ROOT_H
