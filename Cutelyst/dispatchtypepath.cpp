/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "dispatchtypepath_p.h"

#include "common.h"
#include "controller.h"

#include <QStringBuilder>
#include <QBuffer>
#include <QRegularExpression>
#include <QDebug>

using namespace Cutelyst;

DispatchTypePath::DispatchTypePath(QObject *parent) :
    DispatchType(parent)
{
}

QByteArray DispatchTypePath::list() const
{
    Q_D(const DispatchTypePath);

    QByteArray buffer;
    QTextStream out(&buffer, QIODevice::WriteOnly);
    QRegularExpression multipleSlashes("/{1,}");

    out << "Loaded Path actions:" << endl;
    QByteArray pathTitle("Path");
    QByteArray privateTitle("Private");
    int pathLength = pathTitle.length();
    int privateLength = privateTitle.length();

    QList<QByteArray> keys = d->paths.keys();
    qSort(keys.begin(), keys.end());
    Q_FOREACH (const QByteArray &path, keys) {
        Q_FOREACH (Action *action, d->paths.value(path)) {
            QString _path = QLatin1Char('/') % path;
            QByteArray args = action->attributes().value("Args");
            if (args.isEmpty()) {
                _path.append(QLatin1String("/..."));
            } else {
                for (int i = 0; i < action->numberOfArgs(); ++i) {
                    _path.append(QLatin1String("/*"));
                }
            }
            _path.replace(multipleSlashes, QLatin1String("/"));
            pathLength = qMax(pathLength, _path.length());

            QByteArray privateName = action->privateName();
            if (!privateName.startsWith('/')) {
                privateName.prepend('/');
            }
            privateLength = qMax(privateLength, privateName.length());
        }
    }

    out << "." << QByteArray().fill('-', pathLength + 2).data()
        << "+" << QByteArray().fill('-', privateLength + 2).data()
        << "." << endl;
    out << "| " << pathTitle.leftJustified(pathLength).data()
        << " | " << privateTitle.leftJustified(privateLength).data()
        << " |" << endl;
    out << "." << QByteArray().fill('-', pathLength + 2).data()
        << "+" << QByteArray().fill('-', privateLength + 2).data()
        << "." << endl;

    Q_FOREACH (const QByteArray &path, keys) {
        Q_FOREACH (Action *action, d->paths.value(path)) {
            QString _path = QLatin1Char('/') % path;
            if (!action->attributes().contains("Args")) {
                _path.append(QLatin1String("/..."));
            } else {
                for (int i = 0; i < action->numberOfArgs(); ++i) {
                    _path.append(QLatin1String("/*"));
                }
            }
            _path.replace(multipleSlashes, QLatin1String("/"));

            QByteArray privateName = action->privateName();
            if (!privateName.startsWith('/')) {
                privateName.prepend('/');
            }

            out << "| " << _path.leftJustified(pathLength).toUtf8().data()
                << " | " << privateName.leftJustified(privateLength).data()
                << " | " << endl;
        }
    }

    out << "." << QByteArray().fill('-', pathLength + 2).data()
        << "+" << QByteArray().fill('-', privateLength + 2).data()
        << "." << endl;

    return buffer;
}

bool DispatchTypePath::match(Context *ctx, const QByteArray &path, const QStringList &args) const
{
    Q_D(const DispatchTypePath);

    QByteArray _path = path;
    if (_path.isEmpty()) {
        _path = QByteArray("/", 1);
    }

    int numberOfArgs = args.size();
    const ActionList &actions = d->paths.value(_path);
    Q_FOREACH (Action *action, actions) {
        if (action->match(numberOfArgs)) {
            setupMatchedAction(ctx, action, _path);
            return true;
        }
    }
    return false;
}

bool DispatchTypePath::registerAction(Action *action)
{
    bool ret = false;
    QMultiHash<QByteArray, QByteArray> attributes = action->attributes();
    QMultiHash<QByteArray, QByteArray>::iterator i = attributes.find("Path");
    while (i != attributes.end() && i.key() == "Path") {
        if (registerPath(i.value(), action)) {
            ret = true;
        }

        ++i;
    }

    // We always register valid actions
    return ret;
}

QByteArray DispatchTypePath::uriForAction(const Action *action, const QStringList &captures) const
{
    QByteArray path = action->attributes().value("Path");
    if (!path.isNull()) {
        if (path.isEmpty()) {
            return QByteArray("/", 1);
        } else {
            return path;
        }
    }
    return QByteArray();
}

bool actionLessThan(Action *a1, Action *a2)
{
    return a1->numberOfArgs() < a2->numberOfArgs();
}

bool DispatchTypePath::registerPath(const QByteArray &path, Action *action)
{
    Q_D(DispatchTypePath);

    QByteArray _path = path;
    if (_path.startsWith('/')) {
        _path.remove(0, 1);
    }
    if (_path.isEmpty()) {
        // TODO when we try to match a path
        // it comes without a leading / so
        // when would this be used?
        _path = QByteArray("/", 1);
    }

    if (d->paths.contains(_path)) {
        ActionList actions = d->paths.value(_path);
        int actionNumberOfArgs = action->numberOfArgs();
        Q_FOREACH (const Action *regAction, actions) {
            if (regAction->numberOfArgs() == actionNumberOfArgs) {
                qCWarning(CUTELYST_DISPATCHER) << "Not registering Action"
                                               << action->name()
                                               << "of controller"
                                               << action->controller()->objectName()
                                               << "because it conflicts with "
                                               << regAction->name();
                return false;
            }
        }

        actions.append(action);
        qSort(actions.begin(), actions.end(), actionLessThan);
        d->paths[_path] = actions;
    } else {
        d->paths.insert(_path, ActionList() << action);
    }
    return true;
}
