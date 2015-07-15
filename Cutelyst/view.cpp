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

#include "view.h"

#include <Application>

using namespace Cutelyst;

View::View(Application *app, const QString &name) : Component(app)
{
    setProperty("__name", name);
}

View::~View()
{
}

QString View::name() const
{
    return property("__name").toString();
}

Component::Modifiers View::modifiers() const
{
    return Component::OnlyExecute;
}

bool View::render(Context *c) const
{
    Q_UNUSED(c)
    return false;
}
