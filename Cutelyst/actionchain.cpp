/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "actionchain_p.h"
#include "context_p.h"
#include "request_p.h"

using namespace Cutelyst;

ActionChain::ActionChain(const ActionList &chain, QObject *parent)
    : Action(new ActionChainPrivate, parent)
{
    Q_D(ActionChain);
    d->chain = chain;

    const Action *final = d->chain.last();

    QVariantHash args;
    args.insert(QStringLiteral("namespace"), final->ns());
    setupAction(args, nullptr);

    setName(u'_' + final->name());
    setReverse(final->reverse());
    setAttributes(final->attributes());
    setController(final->controller());

    for (Action *action : chain) {
        // FINAL should not have captures?
        if (/*action != final && */ action->numberOfCaptures() > 0) {
            d->captures += action->numberOfCaptures();
        }
    }
}

ActionList ActionChain::chain() const noexcept
{
    Q_D(const ActionChain);
    return d->chain;
}

qint8 ActionChain::numberOfCaptures() const noexcept
{
    Q_D(const ActionChain);
    return d->captures;
}

bool ActionChain::doExecute(Context *c)
{
    Q_D(const ActionChain);

    Request *request              = c->request();
    const QStringList captures    = request->captures();
    const QStringList currentArgs = request->args();
    const ActionList chain        = d->chain;

    int &actionRefCount = c->d_ptr->actionRefCount;
    int &captured       = c->d_ptr->chainedCaptured;
    int &chainedIx      = c->d_ptr->chainedIx;

    for (; chainedIx < chain.size(); ++chainedIx) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.prepend(this);
            request->setArguments(currentArgs);
            break;
        }

        Action *action = chain.at(chainedIx);

        QStringList args;
        while (args.size() < action->numberOfCaptures() && captured < captures.size()) {
            args.append(captures.at(captured++));
        }

        // Final action gets args instead of captures
        request->setArguments(action != chain.last() ? args : currentArgs);
        ++actionRefCount;
        const bool ret = action->dispatch(c);
        --actionRefCount;
        if (!ret) {
            return false;
        }
    }

    return true;
}

#include "moc_actionchain.cpp"
