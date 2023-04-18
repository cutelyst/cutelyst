/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTIONCHAIN_H
#define ACTIONCHAIN_H

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/qobject.h>

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
    virtual ~ActionChain() override = default;

    /**
     * The action chain
     * @return a list of Cutelyst::Action objects encapsulated by this chain.
     */
    ActionList chain() const noexcept;

    /**
     * Reimplemented from Action::numberOfCaptures()
     */
    virtual qint8 numberOfCaptures() const noexcept override;

protected:
    virtual bool doExecute(Context *c) override;
};

} // namespace Cutelyst

#endif // ACTIONCHAIN_H
