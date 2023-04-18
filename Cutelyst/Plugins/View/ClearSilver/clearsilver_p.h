/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CLEARSILVER_P_H
#define CLEARSILVER_P_H

#include "clearsilver.h"
#include "view_p.h"

#include <ClearSilver/ClearSilver.h>

namespace Cutelyst {

class ClearSilverPrivate : public ViewPrivate
{
public:
    virtual ~ClearSilverPrivate() override = default;

    HDF *hdfForStash(Context *c, const QVariantHash &stash) const;
    void serializeHash(HDF *hdf, const QVariantHash &hash, const QString &prefix = QString()) const;
    void serializeMap(HDF *hdf, const QVariantMap &map, const QString &prefix = QString()) const;
    void serializeVariant(HDF *hdf, const QVariant &value, const QString &key) const;
    bool render(Context *c, const QString &filename, const QVariantHash &stash, QByteArray &output) const;
    void renderError(Context *c, const QString &error) const;

    QStringList includePaths;
    QString extension = QStringLiteral(".html");
    QString wrapper;
};

} // namespace Cutelyst

#endif // CLEARSILVER_P_H
