/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "actionchain.h"
#include "common.h"
#include "context.h"
#include "dispatchtypechained_p.h"
#include "utils.h"

#include <QtCore/QUrl>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

DispatchTypeChained::DispatchTypeChained(QObject *parent)
    : DispatchType(parent)
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
    std::ranges::sort(endPoints, [](const Action *a, const Action *b) -> bool {
        return a->reverse() < b->reverse();
    });

    QVector<QStringList> paths;
    QVector<QStringList> unattachedTable;
    for (Action *endPoint : std::as_const(endPoints)) {
        QStringList parts;
        if (endPoint->numberOfArgs() == -1) {
            parts.append(u"..."_s);
        } else {
            for (int i = 0; i < endPoint->numberOfArgs(); ++i) {
                parts.append(u"*"_s);
            }
        }

        QString parent;
        QString extra    = DispatchTypeChainedPrivate::listExtraHttpMethods(endPoint);
        QString consumes = DispatchTypeChainedPrivate::listExtraConsumes(endPoint);
        ActionList parents;
        Action *current = endPoint;
        while (current) {
            for (int i = 0; i < current->numberOfCaptures(); ++i) {
                parts.prepend(u"*"_s);
            }

            const auto attributes       = current->attributes();
            const QStringList pathParts = attributes.values(u"PathPart"_s);
            for (const QString &part : pathParts) {
                if (!part.isEmpty()) {
                    parts.prepend(part);
                }
            }

            parent  = attributes.value(u"Chained"_s);
            current = d->actions.value(parent);
            if (current) {
                parents.prepend(current);
            }
        }

        if (parent.compare(u"/") != 0) {
            QStringList row;
            if (parents.isEmpty()) {
                row.append(u'/' + endPoint->reverse());
            } else {
                row.append(u'/' + parents.first()->reverse());
            }
            row.append(parent);
            unattachedTable.append(row);
            continue;
        }

        QVector<QStringList> rows;
        for (const Action *p : parents) {
            QString name = u'/' + p->reverse();

            QString extraHttpMethod = DispatchTypeChainedPrivate::listExtraHttpMethods(p);
            if (!extraHttpMethod.isEmpty()) {
                name.prepend(extraHttpMethod + u' ');
            }

            const auto attributes = p->attributes();
            auto it               = attributes.constFind(u"CaptureArgs"_s);
            if (it != attributes.constEnd()) {
                name.append(u" (" + it.value() + u')');
            } else {
                name.append(u" (0)");
            }

            QString ct = DispatchTypeChainedPrivate::listExtraConsumes(p);
            if (!ct.isEmpty()) {
                name.append(u" :" + ct);
            }

            if (p != parents[0]) {
                name = u"-> " + name;
            }

            rows.append({QString(), name});
        }

        QString line;
        if (!rows.isEmpty()) {
            line.append(u"=> ");
        }
        if (!extra.isEmpty()) {
            line.append(extra + u' ');
        }
        line.append(u'/' + endPoint->reverse());
        if (endPoint->numberOfArgs() == -1) {
            line.append(u" (...)");
        } else {
            line.append(u" (" + QString::number(endPoint->numberOfArgs()) + u')');
        }

        if (!consumes.isEmpty()) {
            line.append(u" :" + consumes);
        }
        rows.append({QString{}, line});

        rows[0][0] = u'/' + parts.join(u'/');
        paths.append(rows);
    }

    QTextStream out(&buffer, QTextStream::WriteOnly);

    if (!paths.isEmpty()) {
        out << Utils::buildTable(paths,
                                 {
                                     u"Path Spec"_s,
                                     u"Private"_s,
                                 },
                                 u"Loaded Chained actions:"_s);
    }

    if (!unattachedTable.isEmpty()) {
        out << Utils::buildTable(unattachedTable,
                                 {
                                     u"Private"_s,
                                     u"Missing parent"_s,
                                 },
                                 u"Unattached Chained actions:"_s);
    }

    return buffer;
}

DispatchType::MatchType
    DispatchTypeChained::match(Context *c, QStringView path, const QStringList &args) const
{
    if (!args.isEmpty()) {
        return NoMatch;
    }

    Q_D(const DispatchTypeChained);

    const BestActionMatch ret = d->recurseMatch(args.size(), u"/"_s, path.mid(1).split(u'/'));
    const ActionList chain    = ret.actions;
    if (ret.isNull || chain.isEmpty()) {
        return NoMatch;
    }

    QStringList decodedArgs;
    const auto parts = ret.parts;
    for (const auto &arg : parts) {
        QString aux = arg.toString();
        decodedArgs.append(Utils::decodePercentEncoding(&aux));
    }

    auto action      = new ActionChain(chain, c);
    Request *request = c->request();
    request->setArguments(decodedArgs);
    QStringList captures;
    for (const auto a : ret.captures) {
        captures.append(a.toString());
    }
    request->setCaptures(captures);
    request->setMatch(u'/' + action->reverse());
    setupMatchedAction(c, action);

    return ExactMatch;
}

bool DispatchTypeChained::registerAction(Action *action)
{
    Q_D(DispatchTypeChained);

    auto attributes               = action->attributes();
    const QStringList chainedList = attributes.values(u"Chained"_s);
    if (chainedList.isEmpty()) {
        return false;
    }

    if (chainedList.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Multiple Chained attributes not supported registering" << action->reverse();
        return false;
    }

    const QString &chainedTo = chainedList.first();
    if (chainedTo == u'/' + action->name()) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Actions cannot chain to themselves registering /" << action->name();
        return false;
    }

    const QStringList pathPart = attributes.values(u"PathPart"_s);

    QString part = action->name();

    if (pathPart.size() == 1 && !pathPart[0].isEmpty()) {
        part = pathPart[0];
    } else if (pathPart.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Multiple PathPart attributes not supported registering" << action->reverse();
        return false;
    }

    if (part.startsWith(u'/')) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Absolute parameters to PathPart not allowed registering" << action->reverse();
        return false;
    }

    attributes.replace(u"PathPart"_s, part);
    action->setAttributes(attributes);

    auto &childrenOf = d->childrenOf[chainedTo][part];
    childrenOf.insert(childrenOf.begin(), action);

    d->actions[u'/' + action->reverse()] = action;

    if (!d->checkArgsAttr(action, u"Args"_s) || !d->checkArgsAttr(action, u"CaptureArgs"_s)) {
        return false;
    }

    if (attributes.contains(u"Args"_s) && attributes.contains(u"CaptureArgs"_s)) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Combining Args and CaptureArgs attributes not supported registering"
            << action->reverse();
        return false;
    }

    if (!attributes.contains(u"CaptureArgs"_s)) {
        d->endPoints.push_back(action);
    }

    return true;
}

QString DispatchTypeChained::uriForAction(Action *action, const QStringList &captures) const
{
    Q_D(const DispatchTypeChained);

    QString ret;
    const ParamsMultiMap attributes = action->attributes();
    if (!(attributes.contains(u"Chained"_s) && !attributes.contains(u"CaptureArgs"_s))) {
        qCWarning(CUTELYST_DISPATCHER_CHAINED)
            << "uriForAction: action is not an end point" << action;
        return ret;
    }

    QString parent;
    QStringList localCaptures = captures;
    QStringList parts;
    const Action *curr = action;
    while (curr) {
        const ParamsMultiMap curr_attributes = curr->attributes();
        if (curr_attributes.contains(u"CaptureArgs"_s)) {
            if (localCaptures.size() < curr->numberOfCaptures()) {
                // Not enough captures
                qCWarning(CUTELYST_DISPATCHER_CHAINED)
                    << "uriForAction: not enough captures" << curr->numberOfCaptures()
                    << captures.size();
                return ret;
            }

            parts = localCaptures.mid(localCaptures.size() - curr->numberOfCaptures()) + parts;
            localCaptures = localCaptures.mid(0, localCaptures.size() - curr->numberOfCaptures());
        }

        const QString pp = curr_attributes.value(u"PathPart"_s);
        if (!pp.isEmpty()) {
            parts.prepend(pp);
        }

        parent = curr_attributes.value(u"Chained"_s);
        curr   = d->actions.value(parent);
    }

    if (parent.compare(u"/") != 0) {
        // fail for dangling action
        qCWarning(CUTELYST_DISPATCHER_CHAINED) << "uriForAction: dangling action" << parent;
        return ret;
    }

    if (!localCaptures.isEmpty()) {
        // fail for too many captures
        qCWarning(CUTELYST_DISPATCHER_CHAINED)
            << "uriForAction: too many captures" << localCaptures;
        return ret;
    }

    ret = u'/' + parts.join(u'/');
    return ret;
}

Action *DispatchTypeChained::expandAction(const Context *c, Action *action) const
{
    Q_D(const DispatchTypeChained);

    // Do not expand action if action already is an ActionChain
    if (qobject_cast<ActionChain *>(action)) {
        return action;
    }

    // The action must be chained to something
    if (!action->attributes().contains(u"Chained"_s)) {
        return nullptr;
    }

    ActionList chain;
    Action *curr = action;

    while (curr) {
        chain.prepend(curr);
        const QString parent = curr->attribute(u"Chained"_s);
        curr                 = d->actions.value(parent);
    }

    return new ActionChain(chain, const_cast<Context *>(c));
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

BestActionMatch DispatchTypeChainedPrivate::recurseMatch(int reqArgsSize,
                                                         const QString &parent,
                                                         const QList<QStringView> &pathParts) const
{
    BestActionMatch bestAction;
    const auto it = childrenOf.constFind(parent);
    if (it == childrenOf.constEnd()) {
        return bestAction;
    }

    const StringActionsMap &children = it.value();
    QStringList keys                 = children.keys();
    std::ranges::sort(keys, [](const QString &a, const QString &b) -> bool {
        // action2 then action1 to try the longest part first
        return b.size() < a.size();
    });

    for (const QString &tryPart : std::as_const(keys)) {
        auto parts = pathParts;
        if (!tryPart.isEmpty()) {
            // We want to count the number of parts a split would give
            // and remove the number of parts from tryPart
            int tryPartCount         = tryPart.count(u'/') + 1;
            const auto possibleParts = parts.mid(0, tryPartCount);

            QString possiblePartsString;
            bool first = true;
            for (const auto part : possibleParts) {
                if (first) {
                    possiblePartsString = part.toString();
                    first               = false;
                } else {
                    possiblePartsString.append(u'/' + part);
                }
            }

            if (tryPart != possiblePartsString) {
                continue;
            }
            parts = parts.mid(tryPartCount);
        }

        const Actions tryActions = children.value(tryPart);
        for (Action *action : tryActions) {
            const ParamsMultiMap attributes = action->attributes();
            if (attributes.contains(u"CaptureArgs"_s)) {
                const auto captureCount = action->numberOfCaptures();
                // Short-circuit if not enough remaining parts
                if (parts.size() < captureCount) {
                    continue;
                }

                // strip CaptureArgs into list
                const auto captures = parts.mid(0, captureCount);

                // check if the action may fit, depending on a given test by the app
                if (!action->matchCaptures(captures.size())) {
                    continue;
                }

                const auto localParts = parts.mid(captureCount);

                // try the remaining parts against children of this action
                const BestActionMatch ret =
                    recurseMatch(reqArgsSize, u'/' + action->reverse(), localParts);

                //    No best action currently
                // OR The action has less parts
                // OR The action has equal parts but less captured data (ergo more defined)
                ActionList bestActions    = ret.actions;
                const auto actionCaptures = ret.captures;
                const auto actionParts    = ret.parts;
                int bestActionParts       = bestAction.parts.size();

                if (!bestActions.isEmpty() &&
                    (bestAction.isNull || actionParts.size() < bestActionParts ||
                     (actionParts.size() == bestActionParts &&
                      actionCaptures.size() < bestAction.captures.size() &&
                      ret.n_pathParts > bestAction.n_pathParts))) {
                    bestActions.prepend(action);
                    int pathparts          = attributes.value(u"PathPart"_s).count(u'/') + 1;
                    bestAction.actions     = bestActions;
                    bestAction.captures    = captures + actionCaptures;
                    bestAction.parts       = actionParts;
                    bestAction.n_pathParts = pathparts + ret.n_pathParts;
                    bestAction.isNull      = false;
                }
            } else {
                if (!action->match(reqArgsSize + parts.size())) {
                    continue;
                }

                const QString argsAttr = attributes.value(u"Args"_s);
                const int pathparts    = attributes.value(u"PathPart"_s).count(u'/') + 1;
                //    No best action currently
                // OR This one matches with fewer parts left than the current best action,
                //    And therefore is a better match
                // OR No parts and this expects 0
                //    The current best action might also be Args(0),
                //    but we couldn't chose between then anyway so we'll take the last seen

                if (bestAction.isNull || parts.size() < bestAction.parts.size() ||
                    (parts.isEmpty() && !argsAttr.isEmpty() && action->numberOfArgs() == 0)) {
                    bestAction.actions     = {action};
                    bestAction.captures    = {};
                    bestAction.parts       = parts;
                    bestAction.n_pathParts = pathparts;
                    bestAction.isNull      = false;
                }
            }
        }
    }

    return bestAction;
}

bool DispatchTypeChainedPrivate::checkArgsAttr(const Action *action, const QString &name) const
{
    const auto attributes = action->attributes();
    if (!attributes.contains(name)) {
        return true;
    }

    const QStringList values = attributes.values(name);
    if (values.size() > 1) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Multiple" << name << "attributes not supported registering" << action->reverse();
        return false;
    }

    QString args = values[0];
    bool ok;
    if (!args.isEmpty() && args.toInt(&ok) < 0 && !ok) {
        qCCritical(CUTELYST_DISPATCHER_CHAINED)
            << "Invalid" << name << "(" << args << ") for action" << action->reverse() << "(use '"
            << name << "' or '" << name << "(<number>)')";
        return false;
    }

    return true;
}

QString DispatchTypeChainedPrivate::listExtraHttpMethods(const Action *action)
{
    QString ret;
    const auto attributes = action->attributes();
    if (attributes.contains(u"HTTP_METHODS"_s)) {
        const QStringList extra = attributes.values(u"HTTP_METHODS"_s);
        ret                     = extra.join(u", ");
    }
    return ret;
}

QString DispatchTypeChainedPrivate::listExtraConsumes(const Action *action)
{
    QString ret;
    const auto attributes = action->attributes();
    if (attributes.contains(u"CONSUMES"_s)) {
        const QStringList extra = attributes.values(u"CONSUMES"_s);
        ret                     = extra.join(u", ");
    }
    return ret;
}

#include "moc_dispatchtypechained.cpp"
