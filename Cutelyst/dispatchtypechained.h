/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DISPATCHTYPECHAINED_H
#define DISPATCHTYPECHAINED_H

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
    virtual ~DispatchTypeChained() override;

    virtual QByteArray list() const override;

    virtual MatchType match(Context *c, const QString &path, const QStringList &args) const override;

    virtual bool registerAction(Action *action) override;

    virtual QString uriForAction(Action *action, const QStringList &captures) const override;

    Action *expandAction(const Context *c, Action *action) const final;

    virtual bool inUse() override;

private:
    DispatchTypeChainedPrivate *d_ptr;
};

} // namespace Cutelyst

#endif // DISPATCHTYPECHAINED_H
