/*
 * Copyright (C) 2015-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "dispatchtypechained_p.h"
#include "common.h"
#include "actionchain.h"
#include "utils.h"
#include "context.h"

#include <QtCore/QUrl>

using namespace Cutelyst;

DispatchTypeChained::DispatchTypeChained(QObject *parent) : DispatchType(parent)
  , d_ptr(new DispatchTypeChainedPrivate)
{

}

DispatchTypeChained::~DispatchTypeChained()
{
    delete d_ptr;
}

QByteArray DispatchTypeChained::list() const
{
    Q_D(const DispatchTypeChained);

    QByteArray buffer;
    Actions endPoints = d->endPoints;
    std::sort(endPoints.begin(), endPoints.end(), [](Action *a, Action *b) -> bool {
        return a->reverse() < b->reverse();
    });

    QVector<QStringList> paths;
    QVector<QStringList> unattachedTable;
    for (Action *endPoint : endPoints) {
        QStringList parts;
        if (endPoint->numberOfArgs() == -1) {
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
            for (int i = 0; i < current->numberOfCaptures(); ++i) {
                parts.prepend(QLatin1String("*"));
            }

            const auto attributes = current->attributes();
            const QStringList pathParts = attributes.values(QLatin1String("PathPart"));
            for (const QString &part : pathParts) {
                if (!part.isEmpty()) {
                    parts.prepend(part);
                }
            }

            parent = attributes.value(QLatin1String("Chained"));
            current = d->actions.value(parent);
            if (current) {
                parents.prepend(current);
            }
        }

        if (parent != QLatin1String("/")) {
            QStringList row;
            if (parents.isEmpty()) {
                row.append(QLatin1Char('/') + endPoint->reverse());
            } else {
                row.append(QLatin1Char('/') + parents.first()->reverse());
            }
            row.append(parent);
            unattachedTable.append(row);
            continue;
        }

        QVector<QStringList> rows;
        for (Action *p : parents) {
            QString name = QLatin1Char('/') + p->reverse();

            QString extraHttpMethod = DispatchTypeChainedPrivate::listExtraHttpMethods(p);
            if (!extraHttpMethod.isEmpty()) {
                name.prepend(extraHttpMethod + QLatin1Char(' '));
            }

            const auto attributes = p->attributes();
            auto it = attributes.constFind(QLatin1String("CaptureArgs"));
            if (it != attributes.constEnd()) {
                name.append(QLatin1String(" (") + it.value() + QLatin1Char(')'));
            } else {
                name.append(QLatin1String(" (0)"));
            }

            QString ct = DispatchTypeChainedPrivate::listExtraConsumes(p);
            if (!ct.isEmpty()) {
                name.append(QLatin1String(" :") + ct);
            }

            if (p != parents[0]) {
                name = QLatin1String("-> ") + name;
            }

            rows.append({QString(), name});
        }

        QString line;
        if (!rows.isEmpty()) {
            line.append(QLatin1String("=> "));
        }
        if (!extra.isEmpty()) {
            line.append(extra + QLatin1Char(' '));
        }
        line.append(QLatin1Char('/') + endPoint->reverse());
        if (endPoint->numberOfArgs() == -1) {
            line.append(QLatin1String(" (...)"));
        } else {
            line.append(QLatin1String(" (") + QString::number(endPoint->numberOfArgs()) + QLatin1Char(')'));
        }

        if (!consumes.isEmpty()) {
            line.append(QLatin1String(" :") + consumes);
        }
        rows.append({QString(), line});

        rows[0][0] = QLatin1Char('/') + parts.join(QLatin1Char('/'));
        paths.append(rows);
    }

    QTextStream out(&buffer, QIODevice::WriteOnly);

    if (!paths.isEmpty()) {
        out << Utils::buildTable(paths, { QLatin1String("Path Spec"), QLatin1String("Private") },
                                 QLatin1String("Loaded Chained actions:"));
    }

    if (!unattachedTable.isEmpty()) {
        out << Utils::buildTable(unattachedTable, { QLatin1String("Private"), QLatin1String("Missing parent") },
                                 QLatin1String("Unattached Chained actions:"));
    }

    return buffer;
}

DispatchType::MatchType DispatchTypeChained::match(Context *c, const QString &path, const QStringList &args) const
{
    if (!args.isEmpty()) {
        return NoMatch;
    }

    Q_D(const DispatchTypeChained);

    const BestActionMatch ret = d->recurseMatch(args.size(), QStringLiteral("/"), path.split(QLatin1Char('/')));
    const ActionList chain = ret.actions;
    if (ret.isNull || chain.isEmpty()) {
        return NoMatch;
    }

    QStringList decodedArgs;
    const QStringList parts = ret.parts;
    for (const QString &arg : parts) {
        QString aux = arg;
        decodedArgs.append(Utils::decodePercentEncoding(&aux));
    }

    ActionChain *action = new ActionChain(chain, c);
    Request *request = c->request();
    request->setArguments(decodedArgs);
    request->setCaptures(ret.captures);
    request->setMatch(QLatin1Char('/') + action->reverse());
    setupMatchedAction(c, action);

    return ExactMatch;
}

bool DispatchTypeChained::registerAction(Action *action)
{
    Q_D(DispatchTypeChained);

    auto attributes = action->attributes();
    const QStringList chainedList = attributes.values(QLatin1String("Chained"));
    if (chainedList.isEmpty()) {
        return false;
    }

    if (chainedList.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple Chained attributes not supported registering" << action->reverse();
        return false;
    }

    const QString chainedTo = chainedList.first();
    if (chainedTo == QLatin1Char('/') + action->name()) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Actions cannot chain to themselves registering /" << action->name();
        return false;
    }

    const QStringList pathPart = attributes.values(QLatin1String("PathPart"));

    QString part = action->name();

    if (pathPart.size() == 1 && !pathPart[0].isEmpty()) {
        part = pathPart[0];
    } else if (pathPart.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple PathPart attributes not supported registering"
                << action->reverse();
        return false;
    }

    if (part.startsWith(QLatin1Char('/'))) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Absolute parameters to PathPart not allowed registering"
                << action->reverse();
        return false;
    }

    attributes.insert(QStringLiteral("PathPart"), part);
    action->setAttributes(attributes);

    auto &childrenOf = d->childrenOf[chainedTo][part];
    childrenOf.insert(childrenOf.begin(), action);

    d->actions[QLatin1Char('/') + action->reverse()] = action;

    if (!d->checkArgsAttr(action, QLatin1String("Args")) ||
            !d->checkArgsAttr(action, QLatin1String("CaptureArgs"))) {
        return false;
    }

    if (attributes.contains(QLatin1String("Args")) && attributes.contains(QLatin1String("CaptureArgs"))) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Combining Args and CaptureArgs attributes not supported registering"
                << action->reverse();
        return false;
    }

    if (!attributes.contains(QLatin1String("CaptureArgs"))) {
        d->endPoints.push_back(action);
    }

    return true;
}

QString DispatchTypeChained::uriForAction(Action *action, const QStringList &captures) const
{
    Q_D(const DispatchTypeChained);

    QString ret;
    const QMap<QString, QString> attributes = action->attributes();
    if (!(attributes.contains(QStringLiteral("Chained")) &&
            !attributes.contains(QStringLiteral("CaptureArgs")))) {
        qCWarning(CUTELYST_DISPATCHER_CHAINED) << "uriForAction: action is not an end point" << action;
        return ret;
    }

    QString parent;
    QStringList localCaptures = captures;
    QStringList parts;
    Action *curr = action;
    while (curr) {
        const QMap<QString, QString> curr_attributes = curr->attributes();
        if (curr_attributes.contains(QStringLiteral("CaptureArgs"))) {
            if (localCaptures.size() < curr->numberOfCaptures()) {
                // Not enough captures
                qCWarning(CUTELYST_DISPATCHER_CHAINED) << "uriForAction: not enough captures" << curr->numberOfCaptures() << captures.size();
                return ret;
            }

            parts = localCaptures.mid(localCaptures.size() - curr->numberOfCaptures()) + parts;
            localCaptures = localCaptures.mid(0, localCaptures.size() - curr->numberOfCaptures());
        }

        const QString pp = curr_attributes.value(QStringLiteral("PathPart"));
        if (!pp.isEmpty()) {
            parts.prepend(pp);
        }

        parent = curr_attributes.value(QStringLiteral("Chained"));
        curr = d->actions.value(parent);
    }

    if (parent != QLatin1String("/")) {
        // fail for dangling action
        qCWarning(CUTELYST_DISPATCHER_CHAINED) << "uriForAction: dangling action" << parent;
        return ret;
    }

    if (!localCaptures.isEmpty()) {
        // fail for too many captures
        qCWarning(CUTELYST_DISPATCHER_CHAINED) << "uriForAction: too many captures" << localCaptures;
        return ret;
    }

    ret = QLatin1Char('/') + parts.join(QLatin1Char('/'));
    return ret;
}

Action *DispatchTypeChained::expandAction(const Context *c, Action *action) const
{
    Q_D(const DispatchTypeChained);

    // Do not expand action if action already is an ActionChain
    if (qobject_cast<ActionChain*>(action)) {
        return action;
    }

    // The action must be chained to something
    if (!action->attributes().contains(QStringLiteral("Chained"))) {
        return nullptr;
    }

    ActionList chain;
    Action *curr = action;

    while (curr) {
        chain.prepend(curr);
        const QString parent = curr->attribute(QStringLiteral("Chained"));
        curr = d->actions.value(parent);
    }

    return new ActionChain(chain, const_cast<Context*>(c));
}

bool DispatchTypeChained::inUse()
{
    Q_D(const DispatchTypeChained);

    if (d->actions.isEmpty()) {
        return false;
    }

    // Optimize end points

    return true;
}

BestActionMatch DispatchTypeChainedPrivate::recurseMatch(int reqArgsSize, const QString &parent, const QStringList &pathParts) const
{
    BestActionMatch bestAction;
    auto it = childrenOf.constFind(parent);
    if (it == childrenOf.constEnd()) {
        return bestAction;
    }

    const StringActionsMap &children = it.value();
    QStringList keys = children.keys();
    std::sort(keys.begin(), keys.end(), [](const QString &a, const QString &b) -> bool {
        // action2 then action1 to try the longest part first
        return b.size() < a.size();
    });

    for (const QString &tryPart : keys) {
        QStringList parts = pathParts;
        if (!tryPart.isEmpty()) {
            // We want to count the number of parts a split would give
            // and remove the number of parts from tryPart
            int tryPartCount = tryPart.count(QLatin1Char('/')) + 1;
            const QStringList possiblePart = parts.mid(0, tryPartCount);
            if (tryPart != possiblePart.join(QLatin1Char('/'))) {
                continue;
            }
            parts = parts.mid(tryPartCount);
        }

        const Actions tryActions = children.value(tryPart);
        for (Action *action : tryActions) {
            const QMap<QString, QString> attributes = action->attributes();
            if (attributes.contains(QStringLiteral("CaptureArgs"))) {
                const int captureCount = action->numberOfCaptures();
                // Short-circuit if not enough remaining parts
                if (parts.size() < captureCount) {
                    continue;
                }

                // strip CaptureArgs into list
                const QStringList captures = parts.mid(0, captureCount);

                // check if the action may fit, depending on a given test by the app
                if (!action->matchCaptures(captures.size())) {
                    continue;
                }

                const QStringList localParts = parts.mid(captureCount);

                // try the remaining parts against children of this action
                const BestActionMatch ret = recurseMatch(reqArgsSize, QLatin1Char('/') + action->reverse(), localParts);

                //    No best action currently
                // OR The action has less parts
                // OR The action has equal parts but less captured data (ergo more defined)
                ActionList actions = ret.actions;
                const QStringList actionCaptures = ret.captures;
                const QStringList actionParts = ret.parts;
                int bestActionParts = bestAction.parts.size();

                if (!actions.isEmpty() &&
                        (bestAction.isNull ||
                         actionParts.size() < bestActionParts ||
                         (actionParts.size() == bestActionParts &&
                          actionCaptures.size() < bestAction.captures.size() &&
                          ret.n_pathParts > bestAction.n_pathParts))) {
                    actions.prepend(action);
                    int pathparts = attributes.value(QStringLiteral("PathPart")).count(QLatin1Char('/')) + 1;
                    bestAction.actions = actions;
                    bestAction.captures = captures + actionCaptures;
                    bestAction.parts = actionParts;
                    bestAction.n_pathParts = pathparts + ret.n_pathParts;
                    bestAction.isNull = false;
                }
            } else {
                if (!action->match(reqArgsSize + parts.size())) {
                    continue;
                }

                const QString argsAttr = attributes.value(QStringLiteral("Args"));
                const int pathparts = attributes.value(QStringLiteral("PathPart")).count(QLatin1Char('/')) + 1;
                //    No best action currently
                // OR This one matches with fewer parts left than the current best action,
                //    And therefore is a better match
                // OR No parts and this expects 0
                //    The current best action might also be Args(0),
                //    but we couldn't chose between then anyway so we'll take the last seen

                if (bestAction.isNull ||
                        parts.size() < bestAction.parts.size() ||
                        (parts.isEmpty() && !argsAttr.isEmpty() && action->numberOfArgs() == 0)) {
                    bestAction.actions = { action };
                    bestAction.captures = QStringList();
                    bestAction.parts = parts;
                    bestAction.n_pathParts = pathparts;
                    bestAction.isNull = false;
                }
            }
        }
    }

    return bestAction;
}

bool DispatchTypeChainedPrivate::checkArgsAttr(Action *action, const QString &name) const
{
    const QMap<QString, QString> attributes = action->attributes();
    if (!attributes.contains(name)) {
        return true;
    }

    const QStringList values = attributes.values(name);
    if (values.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Multiple"
                << name
                << "attributes not supported registering"
                << action->reverse();
        return false;
    }

    QString args = values[0];
    bool ok;
    if (!args.isEmpty() && args.toInt(&ok) < 0 && !ok) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
                << "Invalid"
                << name << "(" << args << ") for action"
                << action->reverse()
                << "(use '" << name << "' or '" << name << "(<number>)')";
        return false;
    }

    return true;
}

QString DispatchTypeChainedPrivate::listExtraHttpMethods(Action *action)
{
    QString ret;
    const auto attributes = action->attributes();
    if (attributes.contains(QLatin1String("HTTP_METHODS"))) {
        const QStringList extra = attributes.values(QLatin1String("HTTP_METHODS"));
        ret = extra.join(QLatin1String(", "));
    }
    return ret;
}

QString DispatchTypeChainedPrivate::listExtraConsumes(Action *action)
{
    QString ret;
    const auto attributes = action->attributes();
    if (attributes.contains(QLatin1String("CONSUMES"))) {
        const QStringList extra = attributes.values(QLatin1String("CONSUMES"));
        ret = extra.join(QLatin1String(", "));
    }
    return ret;
}

#include "moc_dispatchtypechained.cpp"
