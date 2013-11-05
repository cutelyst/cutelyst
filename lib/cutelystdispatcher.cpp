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

#include "cutelystdispatcher.h"

#include "cutelystcontroller.h"
#include "cutelystaction.h"
#include "cutelystdispatchtypepath.h"

#include <QMetaMethod>
#include <QStringBuilder>
#include <QDebug>

CutelystDispatcher::CutelystDispatcher(QObject *parent) :
    QObject(parent)
{
    m_dispatchers << new CutelystDispatchTypePath(this);
}

void CutelystDispatcher::setupActions()
{
    // Find all the User classes
    int metaType = QMetaType::User;
    while (QMetaType::isRegistered(metaType)) {
        const QMetaObject *meta = QMetaType::metaObjectForType(metaType);
        if (qstrcmp(meta->superClass()->className(), "CutelystController") == 0) {
            // App controller
            CutelystController *controller = qobject_cast<CutelystController*>(meta->newInstance());
            qDebug() << "Found a controller:" << controller << meta->className();

            int i = 0;
            while (i < meta->methodCount()) {
                QMetaMethod method = meta->method(i);
                if (method.methodType() == QMetaMethod::Method) {
                    qDebug() << Q_FUNC_INFO << method.name() << method.attributes() << method.methodType() << method.methodSignature();
                    qDebug() << Q_FUNC_INFO << method.parameterTypes() << method.tag() << method.access();
                    CutelystAction *action = new CutelystAction(method, controller);

                    QString privateName = controller->classNamespace() % QLatin1Char('/') % action->name();
                    if (!m_actions.contains(privateName)) {
                        m_actions.insert(privateName, action);

                        bool registered = false;
                        // Register the action with each dispatcher
                        foreach (CutelystDispatchType *dispatch, m_dispatchers) {
                            if (dispatch->registerAction(action)) {
                                registered = true;
                            }
                        }

                        if (!registered) {
                            qWarning() << "***Could NOT register the action" << privateName << "with any dispatcher";
                        }
                    } else {
                        delete action;
                    }
                }
                ++i;
            }
        }

        if (meta->classInfoCount()) {
            QMetaClassInfo classInfo = meta->classInfo(0);
            qDebug() << Q_FUNC_INFO << classInfo.name() << classInfo.value();
        }
        ++metaType;
    }

    printActions();
}

void CutelystDispatcher::printActions()
{
    qDebug() << "Loaded private actions:";
    QString privateTitle("Private");
    QString classTitle("Class");
    QString methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QHash<QString, CutelystAction*>::ConstIterator it = m_actions.constBegin();
    while (it != m_actions.constEnd()) {
        CutelystAction *action = it.value();
        privateLength = qMax(privateLength, it.key().length());
        classLength = qMax(classLength, action->className().length());
        actionLength = qMax(actionLength, action->name().length());
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";
    qDebug() << "|" << privateTitle.leftJustified(privateLength).toUtf8().data()
             << "|" << classTitle.leftJustified(classLength).toUtf8().data()
             << "|" << methodTitle.leftJustified(actionLength).toUtf8().data()
             << "|";
    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";

    it = m_actions.constBegin();
    while (it != m_actions.constEnd()) {
        CutelystAction *action = it.value();
        qDebug() << "|" << it.key().leftJustified(privateLength).toUtf8().data()
                 << "|" << action->className().leftJustified(classLength).toUtf8().data()
                 << "|" << action->name().leftJustified(actionLength).toUtf8().data()
                 << "|";
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";

    // List all public actions
    foreach (CutelystDispatchType *dispatch, m_dispatchers) {
        dispatch->list();
    }
}
