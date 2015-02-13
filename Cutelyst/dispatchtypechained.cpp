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

#include "dispatchtypechained_p.h"
#include "common.h"

#include <QStringBuilder>

using namespace Cutelyst;

DispatchTypeChained::DispatchTypeChained(QObject *parent) : DispatchType(parent)
  , d_ptr(new DispatchTypeChainedPrivate)
{

}

DispatchTypeChained::~DispatchTypeChained()
{

}

DispatchType::MatchType DispatchTypeChained::match(Context *ctx, const QString &path, const QStringList &args) const
{
    if (!args.isEmpty()) {
        return NoMatch;
    }

    Q_D(const DispatchTypeChained);

    QStringList parts = path.split(QChar('/'));

}

bool DispatchTypeChained::registerAction(Action *action)
{
    Q_D(DispatchTypeChained);

    const QMap<QString, QString> &attributes = action->attributes();
    const QStringList &chainedList = attributes.values(QLatin1String("Chained"));
    if (chainedList.isEmpty()) {
        return false;
    }

    if (chainedList.size() > 0) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple Chained attributes not supported registering"
                << action->reverse();
        exit(1);
    }

    const QString &chainedTo = chainedList.first();
    if (chainedTo == QLatin1String("/")) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Actions cannot chain to themselves registering /"
                << action->reverse();
        exit(1);
    }

    const QStringList &pathPart = attributes.values(QLatin1String("PathPart"));

    QString part = action->name();

    if (pathPart.size() == 1 && !pathPart[0].isEmpty()) {
        part = pathPart[0];
    } else if (pathPart.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple PathPart attributes not supported registering"
                << action->reverse();
        exit(1);
    }

    if (part.startsWith(QChar('/'))) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Absolute parameters to PathPart not allowed registering"
                << action->reverse();
        exit(1);
    }

    // TODO
//    action->attributes()["PathPart"] = part;

    d->childrenOf[chainedTo][part].prepend(action);

    d->actions[QLatin1Char('/') % action->reverse()] = action;

    d->checkArgsAttr(action, "Args");
    d->checkArgsAttr(action, "CaptureArgs");

    if (attributes.contains("Args") && attributes.contains("CaptureArgs")) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Combining Args and CaptureArgs attributes not supported registering"
                << action->reverse();
        exit(1);
    }

    if (!attributes.contains("CaptureArgs")) {
        d->endPoints.prepend(action);
    }

    return true;
}

void DispatchTypeChainedPrivate::checkArgsAttr(Action *action, const QString &name)
{
    const QMap<QString, QString> &attributes = action->attributes();
    if (!attributes.contains(name)) {
        return;
    }

    const QStringList &values = attributes.values(name);
    if (values.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple"
                << name
                << "attributes not supported registering"
                << action->reverse();
        exit(1);
    }

    QString args = values[0];
    bool ok;
    if (!args.isEmpty() && args.toInt(&ok) < 0 && !ok) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Invalid"
                << name << "(" << args << ") for action"
                << action->reverse()
                << "(use '" << name << "' or '" << name << "(<number>)')";
        exit(1);
    }
}
