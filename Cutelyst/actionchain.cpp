/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "actionchain_p.h"
#include "request_p.h"

#include "context.h"

using namespace Cutelyst;

ActionChain::ActionChain(const ActionList &chain, QObject *parent) : Action(parent)
  , d_ptr(new ActionChainPrivate)
{
    Q_D(ActionChain);
    d->chain = chain;

    Action *final = chain.last();
    setController(final->controller());
    QVariantHash args;
    setName(final->name());
    setReverse(final->reverse());
    args.insert(QStringLiteral("namespace"), final->ns());
    setupAction(args, 0);
}

ActionChain::~ActionChain()
{
    delete d_ptr;
}

bool ActionChain::dispatch(Context *c)
{
    Q_D(ActionChain);
    QStringList captures = c->req()->captures();
    ActionList chain = d->chain;
    Action *final = chain.takeLast();
    Q_FOREACH (Action *action, chain) {
        QStringList args;
        if (action->numberOfCaptures()) {
            args = captures.mid(0, action->numberOfCaptures());
        }

        Request *request =  c->request();
        const QStringList &currentArgs = request->args();
        request->setArguments(args);
        if (!action->dispatch(c)) {
            return false;
        }
        request->setArguments(currentArgs);
    }

    return final->dispatch(c);
}

