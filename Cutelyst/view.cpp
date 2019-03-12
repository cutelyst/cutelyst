/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "view.h"

#include "common.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

View::View(QObject *parent, const QString &name) : Component(parent)
{
    setName(name);
}

View::View(ComponentPrivate *d, QObject *parent, const QString &name) : Component(d, parent)
{
    setName(name);
}

Component::Modifiers View::modifiers() const
{
    return Component::OnlyExecute;
}

bool View::doExecute(Context *c)
{
    Response *response = c->response();
    if (response->hasBody()) {
        // Ignore if we already have a body
        return true;
    }

    const QByteArray output = render(c);
    if (Q_UNLIKELY(c->error())) {
        const auto errors = c->errors();
        for (const QString &error : errors) {
            qCCritical(CUTELYST_VIEW) << error;
        }
    }
    response->setBody(output);

    return !c->error();
}

#include "moc_view.cpp"
