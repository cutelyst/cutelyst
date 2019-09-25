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

#ifndef CLEARSILVER_P_H
#define CLEARSILVER_P_H

#include "clearsilver.h"
#include "component_p.h"

#include <ClearSilver/ClearSilver.h>

namespace Cutelyst {

class ClearSilverPrivate : public ComponentPrivate
{
public:
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

}

#endif // CLEARSILVER_P_H
