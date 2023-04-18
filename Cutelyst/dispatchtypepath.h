/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_DISPATCHTYPEPATH_H
#define CUTELYST_DISPATCHTYPEPATH_H

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/dispatchtype.h>

namespace Cutelyst {

class DispatchTypePathPrivate;
class CUTELYST_LIBRARY DispatchTypePath final : public DispatchType
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DispatchTypePath)
public:
    /**
     * Constructs a DispatchTypePath object with the given \p parent.
     */
    explicit DispatchTypePath(QObject *parent = nullptr);
    virtual ~DispatchTypePath() override;

    virtual QByteArray list() const override;

    virtual MatchType match(Context *c, const QString &path, const QStringList &args) const override;

    virtual bool registerAction(Action *action) override;

    virtual bool inUse() override;

    /**
     * Get a URI part for an action
     * Always returns NULL if captures is not empty since Path actions don't have captures
     */
    virtual QString uriForAction(Action *action, const QStringList &captures) const override;

protected:
    DispatchTypePathPrivate *d_ptr;
};

} // namespace Cutelyst

#endif // DispatchTypePath_H
