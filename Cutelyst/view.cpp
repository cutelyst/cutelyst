/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "view.h"

#include "common.h"
#include "view_p.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>

#include <QtCore/QLoggingCategory>
#include <QtCore/QVariant>

using namespace Cutelyst;

View::View(QObject *parent, const QString &name)
    : Component(new ViewPrivate, parent)
{
    setName(name);
}

View::View(ViewPrivate *d, QObject *parent, const QString &name)
    : Component(d, parent)
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
        acceptEncoding.contains(QLatin1String("deflate"), Qt::CaseInsensitive)) {
        QByteArray compressedData = qCompress(output); // Use  zlib's default compression
        compressedData.remove(0, 6);                   // Remove qCompress and zlib headers
        compressedData.chop(4);                        // Remove zlib tailer
        response->headers().setContentEncoding(QStringLiteral("deflate"));
        response->setBody(compressedData);
    } else {
        response->setBody(output);
    }
    return !c->error();
}

void View::setMinimalSizeToDeflate(qint32 minSize)
{
    Q_D(View);
    d->minimalSizeToDeflate = minSize;
}
#include "moc_view.cpp"
