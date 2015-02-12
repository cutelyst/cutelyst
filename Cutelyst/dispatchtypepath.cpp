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
    DispatchType(parent),
    d_ptr(new DispatchTypePathPrivate)
{
}

DispatchTypePath::~DispatchTypePath()
{
    delete d_ptr;
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

    QStringList keys = d->paths.keys();
    qSort(keys.begin(), keys.end());
    Q_FOREACH (const QString &path, keys) {
        Q_FOREACH (Action *action, d->paths.value(path)) {
            QString _path = QLatin1Char('/') % path;
            QString args = action->attributes().value("Args");
            if (args.isEmpty()) {
                _path.append(QLatin1String("/..."));
            } else {
                for (int i = 0; i < action->numberOfArgs(); ++i) {
                    _path.append(QLatin1String("/*"));
                }
            }
            _path.replace(multipleSlashes, QLatin1String("/"));
            pathLength = qMax(pathLength, _path.length());

            QString privateName = action->reverse();
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

    Q_FOREACH (const QString &path, keys) {
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

            QString privateName = action->reverse();
            if (!privateName.startsWith('/')) {
                privateName.prepend('/');
            }

            out << "| " << _path.leftJustified(pathLength).toLatin1().data()
                << " | " << privateName.leftJustified(privateLength).toLatin1().data()
                << " | " << endl;
        }
    }

    out << "." << QByteArray().fill('-', pathLength + 2).data()
        << "+" << QByteArray().fill('-', privateLength + 2).data()
        << "." << endl;

    return buffer;
}

bool DispatchTypePath::match(Context *ctx, const QString &path, const QStringList &args) const
{
    Q_D(const DispatchTypePath);

    QString _path = path;
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
    QMap<QString, QString> attributes = action->attributes();
    QMap<QString, QString>::iterator i = attributes.find("Path");
    while (i != attributes.end() && i.key() == "Path") {
        if (registerPath(i.value(), action)) {
            ret = true;
        }

        ++i;
    }

    // We always register valid actions
    return ret;
}

bool DispatchTypePath::inUse() const
{
    Q_D(const DispatchTypePath);
    return !d->paths.isEmpty();
}

QString DispatchTypePath::uriForAction(Cutelyst::Action *action, const QStringList &captures) const
{
    if (captures.isEmpty()) {
        QMap<QString, QString> attributes = action->attributes();
        QMap<QString, QString>::ConstIterator i = attributes.constFind(QStringLiteral("Path"));
        while (i != attributes.constEnd() && i.key() == "Path") {
            QString path = i.value();
            if (path.isEmpty()) {
                path = QStringLiteral("/");
            }

            if (!path.startsWith('/')) {
                path.prepend('/');
            }

            return path;
        }
    }
    return QString();
}

bool actionLessThan(Action *a1, Action *a2)
{
    return a1->numberOfArgs() < a2->numberOfArgs();
}

bool DispatchTypePath::registerPath(const QString &path, Action *action)
{
    Q_D(DispatchTypePath);

    QString _path = path;
    if (_path.startsWith('/')) {
        _path.remove(0, 1);
    }
    if (_path.isEmpty()) {
        // TODO when we try to match a path
        // it comes without a leading / so
        // when would this be used?
        _path = QStringLiteral("/");
    }

    if (d->paths.contains(_path)) {
        ActionList actions = d->paths.value(_path);
        int actionNumberOfArgs = action->numberOfArgs();
        Q_FOREACH (const Action *regAction, actions) {
            if (regAction->numberOfArgs() == actionNumberOfArgs) {
                qCCritical(CUTELYST_DISPATCHER) << "Not registering Action"
                                                << action->name()
                                                << "of controller"
                                                << action->controller()->objectName()
                                                << "because it conflicts with "
                                                << regAction->name();
                exit(1);
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
