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
#ifndef ACTIONCHAIN_H
#define ACTIONCHAIN_H

#include <QtCore/qobject.h>

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ActionChainPrivate;
/*! \class ActionChain actionchain.h Cutelyst/ActionChain
 * @brief Holds a chain of %Cutelyst %Actions
 *
 * This class represents a chain of Cutelyst Actions.
 * It behaves exactly like the action at the *end* of the chain
 * except on dispatch it will execute all the actions in the chain in order.
 */
class CUTELYST_LIBRARY ActionChain : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ActionChain)
public:
    /**
     * Constructs a ActionChain object with the folloing \p chain and the given \p parent.
     */
    explicit ActionChain(const ActionList &chain, QObject *parent = nullptr);

    /**
     * The action chain
     * @return a list of Cutelyst::Action objects encapsulated by this chain.
     */
    ActionList chain() const;

    /**
     * Reimplemented from Action::numberOfCaptures()
     */
    virtual qint8 numberOfCaptures() const override;

protected:
    virtual bool doExecute(Context *c) override;
};

}

#endif // ACTIONCHAIN_H
