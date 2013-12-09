/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "dispatchtypepath.h"

#include "action.h"

#include <QRegularExpression>
#include <QStringBuilder>
#include <QDebug>

#include <iostream>

using namespace std;
using namespace Cutelyst;

DispatchTypePath::DispatchTypePath(QObject *parent) :
    CutelystDispatchType(parent)
{
}

void DispatchTypePath::list() const
{
    cout << "Loaded Path actions:" << endl;
    QString pathTitle("Path");
    QString privateTitle("Private");
    int pathLength = pathTitle.length();
    int privateLength = privateTitle.length();
    QMap<QString, Action*>::ConstIterator it = m_paths.constBegin();
    while (it != m_paths.constEnd()) {
        Action *action = it.value();
        QString path = QLatin1Char('/') % it.key();
        QString args = action->attributes().value(QLatin1String("Args"));
        if (args.isEmpty()) {
            path.append(QLatin1String("/..."));
        } else {
            for (int i = 0; i < action->numberOfArgs(); ++i) {
                path.append(QLatin1String("/*"));
            }
        }
        path.replace(QRegularExpression("/{1,}"), QLatin1String("/"));
        pathLength = qMax(pathLength, path.length() + 1);

        QString privateName = action->privateName();
        if (!privateName.startsWith(QLatin1String("/"))) {
            privateName.prepend(QLatin1String("/"));
        }
        privateLength = qMax(privateLength, privateName.length());

        ++it;
    }

    cout << "." << QString().fill(QLatin1Char('-'), pathLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "." << endl;
    cout << "|" << pathTitle.leftJustified(pathLength).toUtf8().data()
         << "|" << privateTitle.leftJustified(privateLength).toUtf8().data()
         << "|" << endl;
    cout << "." << QString().fill(QLatin1Char('-'), pathLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "." << endl;

    it = m_paths.constBegin();
    while (it != m_paths.constEnd()) {
        Action *action = it.value();
        QString path = QLatin1Char('/') % it.key();
        if (!action->attributes().contains(QLatin1String("Args"))) {
            path.append(QLatin1String("/..."));
        } else {
            for (int i = 0; i < action->numberOfArgs(); ++i) {
                path.append(QLatin1String("/*"));
            }
        }
        path.replace(QRegularExpression("/{1,}"), QLatin1String("/"));

        QString privateName = action->privateName();
        if (!privateName.startsWith(QLatin1String("/"))) {
            privateName.prepend(QLatin1String("/"));
        }

        cout << "|" << path.leftJustified(pathLength).toUtf8().data()
             << "|" << privateName.leftJustified(privateLength).toUtf8().data()
             << "|" << endl;
        ++it;
    }

    cout << "." << QString().fill(QLatin1Char('-'), pathLength).toUtf8().data()
         << "+" << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
         << "." << endl << endl;
}

bool DispatchTypePath::match(Context *ctx, const QString &path) const
{
    QString _path = path;
    if (_path.isEmpty()) {
        _path = QLatin1Char('/');
    }

    QMap<QString, Action*>::ConstIterator i = m_paths.constFind(_path);
    while (i != m_paths.constEnd() && i.key() == _path) {
        if (i.value()->match(ctx)) {
            setupMatchedAction(ctx, i.value(), _path);
            return true;
        }

        ++i;
    }
    return false;
}

bool DispatchTypePath::registerAction(Action *action)
{
    int pathsCount = m_paths.size();

    QMultiHash<QString, QString> attributes = action->attributes();
    QMultiHash<QString, QString>::iterator i = attributes.find(QLatin1String("Path"));
    while (i != attributes.end() && i.key() == QLatin1String("Path")) {
        registerPath(i.value(), action);

        ++i;
    }

    return m_paths.size() != pathsCount;
}

QString DispatchTypePath::uriForAction(Action *action, const QStringList &captures) const
{
    QString path = action->attributes().value(QLatin1String("Path"));
    if (!path.isNull()) {
        if (path.isEmpty()) {
            return QLatin1String("/");
        } else {
            return path;
        }
    }
    return QString();
}

void DispatchTypePath::registerPath(const QString &path, Action *action)
{
    QString _path = path;
    if (_path.startsWith(QLatin1Char('/'))) {
        _path.remove(0, 1);
    }
    if (_path.isEmpty()) {
        // TODO when we try to match a path
        // it comes without a leading / so
        // when would this be used?
        _path = QLatin1Char('/');
    }

    m_paths.insertMulti(_path, action);
}
