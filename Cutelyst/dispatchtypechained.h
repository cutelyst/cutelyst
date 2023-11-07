/*
 * SPDX-FileCopyrightText: (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/dispatchtype.h>

namespace Cutelyst {

class DispatchTypeChainedPrivate;
class CUTELYST_LIBRARY DispatchTypeChained final : public DispatchType
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DispatchTypeChained)
public:
    /**
     * Constructs a DispatchTypeChained object with the given \p parent.
     */
    explicit DispatchTypeChained(QObject *parent = nullptr);
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
