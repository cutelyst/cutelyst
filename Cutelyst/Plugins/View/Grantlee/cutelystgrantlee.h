/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTGRANTLEE_H
#define CUTELYSTGRANTLEE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <grantlee/taglibraryinterface.h>

#    if defined(cutelyst_grantlee_urifor_EXPORTS)
#        define CUTELYST_GRANTLEE_URIFOR_EXPORT Q_DECL_EXPORT
#    else
#        define CUTELYST_GRANTLEE_URIFOR_EXPORT Q_DECL_IMPORT
#    endif

class CutelystGrantlee final : public QObject
    , public Grantlee::TagLibraryInterface
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
