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

    const static QRegularExpression multipleSlashes(u"/{1,}"_qs);

    QVector<QStringList> table;

    auto keys = d->paths.keys();

    std::sort(keys.begin(), keys.end(), [](QStringView a, QStringView b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });
    for (const auto &path : keys) {
        const auto paths = d->paths.value(path);
        for (Action *action : paths.actions) {
            QString _path = u'/' + path;
            if (action->attribute(u"Args"_qs).isEmpty()) {
                _path.append(u"/...");
            } else {
                for (int i = 0; i < action->numberOfArgs(); ++i) {
                    _path.append(u"/*");
                }
            }
            _path.replace(multipleSlashes, u"/"_qs);

            QString privateName = action->reverse();
            if (!privateName.startsWith(u'/')) {
                privateName.prepend(u'/');
            }

            table.append({_path, privateName});
        }
    }

    return Utils::buildTable(table, {u"Path"_qs, u"Private"_qs}, u"Loaded Path actions:"_qs);
}

Cutelyst::DispatchType::MatchType
    DispatchTypePath::match(Context *c, QStringView path, const QStringList &args) const
{
    Q_D(const DispatchTypePath);

    auto it = d->paths.constFind(path);
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
    const auto range      = attributes.equal_range(u"Path"_qs);
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
        auto it               = attributes.constFind(u"Path"_qs);
        if (it != attributes.constEnd()) {
            const QString &path = it.value();
            if (path.isEmpty()) {
                ret = u"/"_qs;
            } else if (!path.startsWith(u'/')) {
                ret = u'/' + path;
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
    // TODO see if we can make controllers fix this
    if (_path.isEmpty()) {
        _path = u"/"_qs;
    } else if (!_path.startsWith(u'/')) {
        _path.prepend(u'/');
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
