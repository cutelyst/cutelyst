/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "common.h"
#include "controller.h"
#include "dispatchtypepath_p.h"
#include "utils.h"

#include <QBuffer>
#include <QDebug>
#include <QRegularExpression>

using namespace Cutelyst;

DispatchTypePath::DispatchTypePath(QObject *parent)
    : DispatchType(parent)
    , d_ptr(new DispatchTypePathPrivate)
{
}

DispatchTypePath::~DispatchTypePath()
{
    delete d_ptr;
}

QByteArray DispatchTypePath::list() const
{
    Q_D(const DispatchTypePath);

    QRegularExpression multipleSlashes(QLatin1String("/{1,}"));

    QVector<QStringList> table;

    auto keys = d->paths.keys();

    std::sort(keys.begin(), keys.end(), [](QStringView a, QStringView b) {
        return a.compare(b, Qt::CaseInsensitive);
    });
    for (const auto &path : keys) {
        const auto paths = d->paths.value(path);
        for (Action *action : paths.actions) {
            QString _path = u'/' + path;
            if (action->attribute(QLatin1String("Args")).isEmpty()) {
                _path.append(QLatin1String("/..."));
            } else {
                for (int i = 0; i < action->numberOfArgs(); ++i) {
                    _path.append(QLatin1String("/*"));
                }
            }
            _path.replace(multipleSlashes, QLatin1String("/"));

            QString privateName = action->reverse();
            if (!privateName.startsWith(QLatin1Char('/'))) {
                privateName.prepend(QLatin1Char('/'));
            }

            table.append({_path, privateName});
        }
    }

    return Utils::buildTable(table,
                             {QLatin1String("Path"), QLatin1String("Private")},
                             QLatin1String("Loaded Path actions:"));
}

Cutelyst::DispatchType::MatchType
    DispatchTypePath::match(Context *c, QStringView path, const QStringList &args) const
{
    Q_D(const DispatchTypePath);

    StringActionsMap::const_iterator it;

    if (Q_UNLIKELY(path.isEmpty())) {
        it = d->paths.constFind(u"/");
    } else {
        it = d->paths.constFind(path);
    }

    if (it == d->paths.constEnd()) {
        return NoMatch;
    }

    MatchType ret    = NoMatch;
    int numberOfArgs = args.size();
    for (Action *action : it->actions) {
        // If the number of args is -1 (not defined)
        // it will slurp all args so we don't care
        // about how many args was passed
        if (action->numberOfArgs() == numberOfArgs) {
            Request *request = c->request();
            request->setArguments(args);
            request->setMatch(it->name);
            setupMatchedAction(c, action);
            return ExactMatch;
        } else if (action->numberOfArgs() == -1 && !c->action()) {
            // Only setup partial matches if no action is
            // currently set
            Request *request = c->request();
            request->setArguments(args);
            request->setMatch(it->name);
            setupMatchedAction(c, action);
            ret = PartialMatch;
        }
    }
    return ret;
}

bool DispatchTypePath::registerAction(Action *action)
{
    Q_D(DispatchTypePath);

    bool ret              = false;
    const auto attributes = action->attributes();
    const auto range      = attributes.equal_range(QLatin1String("Path"));
    for (auto i = range.first; i != range.second; ++i) {
        if (d->registerPath(*i, action)) {
            ret = true;
        }
    }

    // We always register valid actions
    return ret;
}

bool DispatchTypePath::inUse()
{
    Q_D(const DispatchTypePath);
    return !d->paths.isEmpty();
}

QString DispatchTypePath::uriForAction(Cutelyst::Action *action, const QStringList &captures) const
{
    QString ret;
    if (captures.isEmpty()) {
        const auto attributes = action->attributes();
        auto it               = attributes.constFind(QStringLiteral("Path"));
        if (it != attributes.constEnd()) {
            const QString &path = it.value();
            if (path.isEmpty()) {
                ret = QStringLiteral("/");
            } else if (!path.startsWith(QLatin1Char('/'))) {
                ret = QLatin1Char('/') + path;
            } else {
                ret = path;
            }
        }
    }
    return ret;
}

bool DispatchTypePathPrivate::registerPath(const QString &path, Action *action)
{
    QString _path = path;
    if (_path.startsWith(QLatin1Char('/')) && !_path.isEmpty()) {
        _path.remove(0, 1);
    }
    if (_path.isEmpty()) {
        _path = QStringLiteral("/");
    }

    auto it = paths.find(_path);
    if (it != paths.end()) {
        int actionNumberOfArgs = action->numberOfArgs();
        auto &actions          = it->actions;
        for (const Action *regAction : actions) {
            if (regAction->numberOfArgs() == actionNumberOfArgs) {
                qCCritical(CUTELYST_DISPATCHER_PATH)
                    << "Not registering Action" << action->name() << "of controller"
                    << action->controller()->objectName() << "because it conflicts with"
                    << regAction->name() << "of controller"
                    << regAction->controller()->objectName();
                return false;
            }
        }

        actions.push_back(action);
        std::sort(actions.begin(), actions.end(), [](Action *a, Action *b) -> bool {
            return a->numberOfArgs() < b->numberOfArgs();
        });
    } else {
        paths.insert(_path, DispatchTypePathReplacement{_path, {action}});
    }
    return true;
}

#include "moc_dispatchtypepath.cpp"
