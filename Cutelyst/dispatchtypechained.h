/*
 * SPDX-FileCopyrightText: (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_export.h>
#include <Cutelyst/dispatchtype.h>

namespace Cutelyst {

class DispatchTypeChainedPrivate;
/**
 * @ingroup core
 * @class Cutelyst::DispatchTypeChained
 * @brief Describes a chained dispatch type.
 *
 * Describes a chained dispatch type.
 */
class CUTELYST_EXPORT DispatchTypeChained final : public DispatchType
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DispatchTypeChained)
public:
    /**
     * Constructs a new %DispatchTypeChained object with the given \a parent.
     */
    explicit DispatchTypeChained(QObject *parent = nullptr);

    /**
     * Destryos the %DispatchTypeChained object.
     */
    ~DispatchTypeChained() override;

    QByteArray list() const override;

    MatchType match(Context *c, QStringView path, const QStringList &args) const override;

    bool registerAction(Action *action) override;

    QString uriForAction(Action *action, const QStringList &captures) const override;

    Action *expandAction(const Context *c, Action *action) const final;

    bool inUse() override;

private:
    DispatchTypeChainedPrivate *d_ptr;
};

} // namespace Cutelyst
