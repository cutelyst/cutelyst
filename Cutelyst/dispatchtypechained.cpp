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


bool actionReverseLessThan(Action *action1, Action *action2)
{
    return action1->reverse() < action2->reverse();
}


QByteArray DispatchTypeChained::list() const
{
    Q_D(const DispatchTypeChained);

    ActionList endPoints = d->endPoints;
    qSort(endPoints.begin(), endPoints.end(), actionReverseLessThan);

    QList<QStringList> paths;
    QList<QStringList> unattachedTable;
    Q_FOREACH (Action *endPoint, endPoints) {
        QStringList parts;
        if (!endPoint->attributes().contains("Args")) {
            parts.append(QLatin1String("..."));
        } else {
            for (int i = 0; i < endPoint->numberOfArgs(); ++i) {
                parts.append(QLatin1String("*"));
            }
        }

        QString parent;
        QString extra = DispatchTypeChainedPrivate::listExtraHttpMethods(endPoint);
        QString consumes = DispatchTypeChainedPrivate::listExtraConsumes(endPoint);
        ActionList parents;
        Action *current = endPoint;
        while (current) {
            if (current->attributes().contains("CaptureArgs")) {
                for (int i = 0; i < endPoint->numberOfCaptures(); ++i) {
                    parts.prepend(QLatin1String("*"));
                }
            }

            Q_FOREACH (const QString &part, current->attributes().values("PathPart")) {
                if (!part.isEmpty()) {
                    parts.prepend(part);
                }
            }

            parent = current->attributes().value("Chained");
            current = d->actions.value(parent);
            if (current) {
                parents.prepend(current);
            }
        }

        if (parent != QLatin1String("/")) {
            QStringList row;
            if (parents.isEmpty()) {
                row.append(QLatin1Char('/') % endPoint->name());
            } else {
                row.append(QLatin1Char('/') % parents.first()->name());
            }
            row.append(parent);
            unattachedTable.append(row);
            continue;
        }

        QList<QStringList> rows;
        Q_FOREACH (Action *p, parents) {
            QString name = QLatin1Char('/') % p->name();

            QString extra = DispatchTypeChainedPrivate::listExtraHttpMethods(p);
            if (!extra.isEmpty()) {
                name.prepend(extra % QLatin1Char(' '));
            }

            if (p->attributes().contains("CaptureArgs")) {
                name.append(QLatin1String(" (") % p->attributes().value("CaptureArgs") % QLatin1Char(')'));
            }

            QString ct = DispatchTypeChainedPrivate::listExtraConsumes(p);
            if (!ct.isEmpty()) {
                name.append(QLatin1String(" :") % ct);
            }

            if (p != parents[0]) {
                name = QLatin1String("-> ") % name;
            }

            rows.append({QString(), name});
        }

        QString line;
        if (!rows.isEmpty()) {
            line.append(QLatin1String("=> "));
        }
        if (!extra.isEmpty()) {
            line.append(extra % QLatin1Char(' '));
        }
        line.append(QLatin1Char('/') % endPoint->name());
        if (!consumes.isEmpty()) {
            line.append(QLatin1String(" :") % consumes);
        }
        rows.append({QString(), line});

        rows[0][0] = QLatin1Char('/') % parts.join(QChar('/'));
        paths.append(rows);
    }

    QByteArray buffer;
    QTextStream out(&buffer, QIODevice::WriteOnly);

    if (!paths.isEmpty()) {
        QStringList chainedHeaders;
        chainedHeaders.append("Path Spec");
        chainedHeaders.append("Private");
        out << buildTable("Loaded Chained actions:", chainedHeaders, paths);
    }

    if (!unattachedTable.isEmpty()) {
        QStringList unattachedHeaders;
        unattachedHeaders.append("Private");
        unattachedHeaders.append("Missing parent");

        out << buildTable("Unattached Chained actions:", unattachedHeaders, unattachedTable);
    }

    return buffer;
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

    if (chainedList.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple Chained attributes not supported registering" % action->reverse();
        exit(1);
    }

    const QString &chainedTo = chainedList.first();
    if (chainedTo == QLatin1Char('/') % action->name()) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Actions cannot chain to themselves registering /" % action->name();
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

    d->actions[QLatin1Char('/') % action->name()] = action;

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

QString DispatchTypeChained::uriForAction(Action *action, const QStringList &captures) const
{
    return QString();
}

bool DispatchTypeChained::inUse() const
{
    Q_D(const DispatchTypeChained);
    return !d->actions.isEmpty();
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

QString DispatchTypeChainedPrivate::listExtraHttpMethods(Action *action)
{
    if (action->attributes().contains("HTTP_METHODS")) {
        QStringList extra = action->attributes().values("HTTP_METHODS");
        return extra.join(QLatin1String(", "));
    }
    return QString();
}

QString DispatchTypeChainedPrivate::listExtraConsumes(Action *action)
{
    if (action->attributes().contains("CONSUMES")) {
        QStringList extra = action->attributes().values("CONSUMES");
        return extra.join(QLatin1String(", "));
    }
    return QString();
}
