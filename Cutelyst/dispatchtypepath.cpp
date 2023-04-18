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

    QStringList keys = d->paths.keys();
    keys.sort(Qt::CaseInsensitive);
    for (const QString &path : keys) {
        const auto paths = d->paths.value(path);
        for (Action *action : paths) {
            QString _path = QLatin1Char('/') + path;
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

    return Utils::buildTable(table, {QLatin1String("Path"), QLatin1String("Private")}, QLatin1String("Loaded Path actions:"));
}

Cutelyst::DispatchType::MatchType DispatchTypePath::match(Context *c, const QString &path, const QStringList &args) const
{
    Q_D(const DispatchTypePath);

    QString _path = path;
    if (_path.isEmpty()) {
        _path = QStringLiteral("/");
    }

    const auto it = d->paths.constFind(_path);
    if (it == d->paths.constEnd()) {
        return NoMatch;
    }

    MatchType ret    = NoMatch;
    int numberOfArgs = args.size();
    for (Action *action : it.value()) {
        // If the number of args is -1 (not defined)
        // it will slurp all args so we don't care
        // about how many args was passed
        if (action->numberOfArgs() == numberOfArgs) {
            Request *request = c->request();
            request->setArguments(args);
            request->setMatch(_path);
            setupMatchedAction(c, action);
            return ExactMatch;
        } else if (action->numberOfArgs() == -1 &&
                   !c->action()) {
            // Only setup partial matches if no action is
            // currently set
            Request *request = c->request();
            request->setArguments(args);
            request->setMatch(_path);
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
        for (const Action *regAction : it.value()) {
            if (regAction->numberOfArgs() == actionNumberOfArgs) {
                qCCritical(CUTELYST_DISPATCHER_PATH) << "Not registering Action"
                                                     << action->name()
                                                     << "of controller"
                                                     << action->controller()->objectName()
                                                     << "because it conflicts with"
                                                     << regAction->name()
                                                     << "of controller"
                                                     << regAction->controller()->objectName();
                return false;
            }
        }

        it.value().push_back(action);
        std::sort(it.value().begin(), it.value().end(), [](Action *a, Action *b) -> bool {
            return a->numberOfArgs() < b->numberOfArgs();
        });
    } else {
        paths.insert(_path, {action});
    }
    return true;
}

#include "moc_dispatchtypepath.cpp"
