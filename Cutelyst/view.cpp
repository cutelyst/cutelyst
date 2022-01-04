/*
 * Copyright (C) 2013-2019 Daniel Nicoletti <dantti12@gmail.com>
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
#include "view_p.h"

#include "common.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

View::View(QObject *parent, const QString &name) : Component(new ViewPrivate, parent)
{
    setName(name);
}

View::View(ViewPrivate *d, QObject *parent, const QString &name) : Component(d, parent)
{
    setName(name);
}

Component::Modifiers View::modifiers() const
{
    return Component::OnlyExecute;
}

bool View::doExecute(Context *c)
{
    Q_D(const View);
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
    const QString acceptEncoding = c->req()->header(QStringLiteral("ACCEPT_ENCODING"));
    if (d->minimalSizeToDeflate >= 0 && output.count() > d->minimalSizeToDeflate &&
        acceptEncoding.contains(u"deflate", Qt::CaseInsensitive)) {
        QByteArray compressedData = qCompress(output); // Use  zlib's default compression
        compressedData.remove(0, 6); // Remove qCompress and zlib headers
        compressedData.chop(4); // Remove zlib tailer
        response->headers().setContentEncoding(QStringLiteral("deflate"));
        response->setBody(compressedData);
    } else {
        response->setBody(output);
    }
    return !c->error();
}

void View::setMinimalSizeToDeflate(qint32 minSize) {
    Q_D(View);
    d->minimalSizeToDeflate = minSize;
}
#include "moc_view.cpp"
