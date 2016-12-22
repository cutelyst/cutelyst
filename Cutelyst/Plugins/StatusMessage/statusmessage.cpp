/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "statusmessage.h"

#include <Cutelyst/Application>
#include <Cutelyst/Plugins/Session/session.h>

#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_STATUSMESSAGE, "cutelyst.plugins.statusmessage")

namespace Cutelyst {

class StatusMessagePrivate
{
public:
    QString sessionPrefix = QStringLiteral("status_msg");
    QString tokenParam = QStringLiteral("mid");
    QString statusMsgStashKey = QStringLiteral("status_msg");
    QString errorMsgStashKey = QStringLiteral("error_msg");
};

}

static thread_local StatusMessage *m_instance = nullptr;

StatusMessage::StatusMessage(Application *parent) : Plugin(parent), d_ptr(new StatusMessagePrivate)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
    m_instance = this;
}

StatusMessage::~StatusMessage()
{
    delete d_ptr;
}

QString StatusMessage::sessionPrefix() const
{
    Q_D(const StatusMessage);
    return d->sessionPrefix;
}

void StatusMessage::setSessionPrefix(const QString &sessionPrefix)
{
    Q_D(StatusMessage);
    d->sessionPrefix = sessionPrefix;
}

QString StatusMessage::tokenParam() const
{
    Q_D(const StatusMessage);
    return d->tokenParam;
}

void StatusMessage::setTokenParam(const QString &tokenParam)
{
    Q_D(StatusMessage);
    d->tokenParam = tokenParam;
}

QString StatusMessage::statusMsgStashKey() const
{
    Q_D(const StatusMessage);
    return d->statusMsgStashKey;
}

void StatusMessage::setStatusMsgStashKey(const QString &statusMsgStashKey)
{
    Q_D(StatusMessage);
    d->statusMsgStashKey = statusMsgStashKey;
}

QString StatusMessage::errorMgStashKey() const
{
    Q_D(const StatusMessage);
    return d->errorMsgStashKey;
}

void StatusMessage::setErrorMgStashKey(const QString &errorMgStashKey)
{
    Q_D(StatusMessage);
    d->errorMsgStashKey = errorMgStashKey;
}

void StatusMessage::load(Context *c)
{
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return;
    }
    StatusMessagePrivate *priv = m_instance->d_ptr;

    const QString token = c->request()->queryParam(priv->tokenParam);
    if (token.isEmpty()) {
        return;
    }

    const QString statusKey = priv->sessionPrefix + QLatin1String("status") + token;
    const QVariant statusValue = Session::value(c, statusKey);
    if (!statusValue.isNull()) {
        Session::deleteValue(c, statusKey);
        c->setStash(priv->statusMsgStashKey, statusValue);
    }

    const QString errorKey = priv->sessionPrefix + QLatin1String("error") + token;
    const QVariant errorValue = Session::value(c, errorKey);
    if (!errorValue.isNull()) {
        Session::deleteValue(c, errorKey);
        c->setStash(priv->errorMsgStashKey, errorValue);
    }
}

inline QString createToken()
{
    return QString::number(qrand() % 99999999).rightJustified(8, QLatin1Char('0'), true);
}

QString StatusMessage::error(Context *c, const QString &msg)
{
    QString token;
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return token;
    }

    token = createToken();
    Session::setValue(c, m_instance->d_ptr->sessionPrefix + QLatin1String("error") + token, msg);
    return token;
}

ParamsMultiMap StatusMessage::errorQuery(Context *c, const QString &msg, ParamsMultiMap query)
{
    ParamsMultiMap map(query);
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return map;
    }
    StatusMessagePrivate *priv = m_instance->d_ptr;

    const QString token = createToken();
    Session::setValue(c, priv->sessionPrefix + QLatin1String("error") + token, msg);
    map.insert(priv->tokenParam, token);
    return map;
}

QString StatusMessage::status(Context *c, const QString &msg)
{
    QString token;
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return token;
    }

    token = createToken();
    Session::setValue(c, m_instance->d_ptr->sessionPrefix + QLatin1String("status") + token, msg);
    return token;
}

ParamsMultiMap StatusMessage::statusQuery(Context *c, const QString &msg, ParamsMultiMap query)
{
    ParamsMultiMap map(query);
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return map;
    }
    StatusMessagePrivate *priv = m_instance->d_ptr;

    const QString token = createToken();
    Session::setValue(c, priv->sessionPrefix + QLatin1String("status") + token, msg);
    map.insert(priv->tokenParam, token);
    return map;
}

QString StatusMessage::setError(Context *c, const QString &msg)
{
    return error(c, msg);
}

QString StatusMessage::setStatus(Context *c, const QString &msg)
{
    return status(c, msg);
}

#include "moc_statusmessage.cpp"
