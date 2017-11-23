/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYSTGRANTLEE_H
#define CUTELYSTGRANTLEE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <grantlee/taglibraryinterface.h>

#if defined(cutelyst_grantlee_urifor_EXPORTS)
#  define CUTELYST_GRANTLEE_URIFOR_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_GRANTLEE_URIFOR_EXPORT Q_DECL_IMPORT
#endif

class CutelystGrantlee : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface/1.0")
    Q_INTERFACES(Grantlee::TagLibraryInterface)
public:
    explicit CutelystGrantlee(QObject *parent = nullptr);

    virtual QHash<QString, Grantlee::AbstractNodeFactory *> nodeFactories(const QString &name = QString()) override;

    virtual QHash<QString, Grantlee::Filter *> filters(const QString &name = QString()) override;
};

#endif

#endif // CUTELYSTGRANTLEE_H
