/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTCUTELEE_H
#define CUTELYSTCUTELEE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <cutelee/taglibraryinterface.h>

#    if defined(cutelyst_cutelee_urifor_EXPORTS)
#        define CUTELYST_CUTELEE_URIFOR_EXPORT Q_DECL_EXPORT
#    else
#        define CUTELYST_CUTELEE_URIFOR_EXPORT Q_DECL_IMPORT
#    endif

class CutelystCutelee final : public QObject
    , public Cutelee::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Cutelee::TagLibraryInterface)
public:
    explicit CutelystCutelee(QObject *parent = nullptr);

    virtual QHash<QString, Cutelee::AbstractNodeFactory *> nodeFactories(const QString &name = QString()) override;

    virtual QHash<QString, Cutelee::Filter *> filters(const QString &name = QString()) override;
};

#endif

#endif // CUTELYSTCUTELEE_H
