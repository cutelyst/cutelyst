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
/**
 * \ingroup core
 * \class ActionChain actionchain.h Cutelyst/ActionChain
 * \brief Holds a chain of %Cutelyst actions
 *
 * This class represents a chain of Cutelyst actions.
 * It behaves exactly like the action at the *end* of the chain
 * except on dispatch it will execute all the actions in the chain in order.
 */
class CUTELYST_LIBRARY ActionChain : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ActionChain)
public:
    /**
     * Constructs a new %ActionChain object with the given \a chain and \a parent.
     */
    explicit ActionChain(const ActionList &chain, QObject *parent = nullptr);

    /**
     * Destroys the %ActionChain object.
     */
    virtual ~ActionChain() override = default;

    /**
     * Returns a list of Action objects encapsulated by this chain.
     */
    [[nodiscard]] ActionList chain() const noexcept;

    qint8 numberOfCaptures() const noexcept override;

protected:
    bool doExecute(Context *c) override;
};

} // namespace Cutelyst

#endif // ACTIONCHAIN_H
