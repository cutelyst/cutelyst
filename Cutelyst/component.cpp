/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "component_p.h"
#include "common.h"
#include "context.h"

using namespace Cutelyst;

Component::Component(QObject *parent) : QObject(parent)
  , d_ptr(new ComponentPrivate)
{
}

Component::Component(ComponentPrivate* d, QObject *parent) : QObject(parent)
  , d_ptr(d)
{
}

Component::~Component()
{
    delete d_ptr;
}

Component::Modifiers Component::modifiers() const
{
    return Component::None;
}

QString Component::name() const
{
    Q_D(const Component);
    return d->name;
}

void Component::setName(const QString &name)
{
    Q_D(Component);
    d->name = name;
}

QString Component::reverse() const
{
    Q_D(const Component);
    return d->reverse;
}

void Component::setReverse(const QString &reverse)
{
    Q_D(Component);
    d->reverse = reverse;
}

bool Component::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_UNUSED(application)
    Q_UNUSED(args)
    return true;
}

bool Component::execute(Context *c)
{
    Q_D(Component);

    if (d->proccessRoles) {
        const auto beforeRoles = d->beforeRoles;
        for (Component *code : beforeRoles) {
            if (!code->beforeExecute(c)) {
                return false;
            }
        }

        QStack<Component *> stack = d->aroundRoles;
        // first item on the stack is always the execution code
        stack.push_front(this);
        if (!aroundExecute(c, stack)) {
            return false;
        }

        const auto afterRoles = d->afterRoles;
        for (Component *code : afterRoles) {
            if (!code->afterExecute(c)) {
                return false;
            }
        }

        // Do not call doExecute twice
        return true;
    }

    return doExecute(c);
}

bool Component::beforeExecute(Context *c)
{
    Q_UNUSED(c)
    return true;
}

bool Component::aroundExecute(Context *c, QStack<Cutelyst::Component *> stack)
{
    Q_UNUSED(c)

    int stackSize = stack.size();
    if (stackSize == 1) {
        Component *code = stack.pop();
        return code->doExecute(c);
    } else if (stackSize > 1) {
        Component *code = stack.pop();
        return code->aroundExecute(c, stack);
    }

    // Should NEVER happen
    qCCritical(CUTELYST_COMPONENT) << "Reached end of the stack!" << c->req()->uri();
    return false;
}

bool Component::afterExecute(Context *c)
{
    Q_UNUSED(c)
    return true;
}

bool Component::doExecute(Context *c)
{
    Q_UNUSED(c)
    return true;
}

void Component::applyRoles(const QStack<Cutelyst::Component *> &roles)
{
    Q_D(Component);

    for (Component *code : roles) {
        if (code->modifiers() & BeforeExecute) {
            d->beforeRoles.push(code);
        }

        if (code->modifiers() & AroundExecute) {
            d->aroundRoles.push(code);
        }

        if (code->modifiers() & AfterExecute) {
            d->afterRoles.push(code);
        }
    }
    d->roles = roles;
    d->proccessRoles = true;
}

bool Component::dispatcherReady(const Dispatcher *dispatch, Controller *controller)
{
    Q_D(Component);

    const auto roles = d->roles;
    for (Component *code : roles) {
        code->dispatcherReady(dispatch, controller);
    }
    return true;
}

#include "moc_component.cpp"
