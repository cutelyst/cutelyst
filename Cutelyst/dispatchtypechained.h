/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef DISPATCHTYPECHAINED_H
#define DISPATCHTYPECHAINED_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/action.h>
#include <Cutelyst/dispatchtype.h>

namespace Cutelyst {

class DispatchTypeChainedPrivate;
class CUTELYST_LIBRARY DispatchTypeChained : public DispatchType
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DispatchTypeChained)
public:
    /**
     * Constructs a DispatchTypeChained object with the given \p parent.
     */
    explicit DispatchTypeChained(QObject *parent = nullptr);
    ~DispatchTypeChained();

    virtual QByteArray list() const override;

    virtual MatchType match(Context *c, const QString &path, const QStringList &args) const override;

    virtual bool registerAction(Action *action) override;

    virtual QString uriForAction(Action *action, const QStringList &captures) const override;

    Action *expandAction(const Context *c, Action *action) const final;

    virtual bool inUse() override;

private:
    DispatchTypeChainedPrivate *d_ptr;
};

}

#endif // DISPATCHTYPECHAINED_H
