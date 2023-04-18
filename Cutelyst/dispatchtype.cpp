/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "dispatchtype.h"

#include "context_p.h"

using namespace Cutelyst;

DispatchType::DispatchType(QObject *parent)
    : QObject(parent)
{
}

DispatchType::~DispatchType()
{
}

Action *DispatchType::expandAction(const Context *c, Action *action) const
{
    Q_UNUSED(c)
    Q_UNUSED(action)
    return nullptr;
}

QString DispatchType::uriForAction(Action *action, const QStringList &captures) const
{
    Q_UNUSED(action)
    Q_UNUSED(captures)
    return QString();
}

bool DispatchType::registerAction(Action *action)
{
    Q_UNUSED(action)
    return true;
}

bool DispatchType::isLowPrecedence() const
{
    return false;
}

void DispatchType::setupMatchedAction(Context *c, Action *action) const
{
    c->d_ptr->action = action;
}

#include "moc_dispatchtype.cpp"
