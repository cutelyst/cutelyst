/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "statusmessage.h"

#include <Cutelyst/Application>
#include <Cutelyst/Plugins/Session/session.h>

#include <QRandomGenerator>
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_STATUSMESSAGE, "cutelyst.plugins.statusmessage", QtWarningMsg)

static thread_local StatusMessage *m_instance = nullptr;

namespace Cutelyst {

class StatusMessagePrivate
{
public:
    static void _q_postFork(Application *app);

    QString sessionPrefix     = QStringLiteral("status_msg");
    QString tokenParam        = QStringLiteral("mid");
    QString statusMsgStashKey = QStringLiteral("status_msg");
    QString errorMsgStashKey  = QStringLiteral("error_msg");
};

void StatusMessagePrivate::_q_postFork(Application *app)
{
    m_instance = app->plugin<StatusMessage *>();
}

} // namespace Cutelyst

StatusMessage::StatusMessage(Application *parent)
    : Plugin(parent)
    , d_ptr(new StatusMessagePrivate)
{
    m_instance = this;
}

StatusMessage::~StatusMessage()
{
    delete d_ptr;
}

QString StatusMessage::sessionPrefix() const noexcept
{
    Q_D(const StatusMessage);
    return d->sessionPrefix;
}

void StatusMessage::setSessionPrefix(const QString &sessionPrefix)
{
    Q_D(StatusMessage);
    d->sessionPrefix = sessionPrefix;
}

QString StatusMessage::tokenParam() const noexcept
{
    Q_D(const StatusMessage);
    return d->tokenParam;
}

void StatusMessage::setTokenParam(const QString &tokenParam)
{
    Q_D(StatusMessage);
    d->tokenParam = tokenParam;
}

QString StatusMessage::statusMsgStashKey() const noexcept
{
    Q_D(const StatusMessage);
    return d->statusMsgStashKey;
}

void StatusMessage::setStatusMsgStashKey(const QString &statusMsgStashKey)
{
    Q_D(StatusMessage);
    d->statusMsgStashKey = statusMsgStashKey;
}

QString StatusMessage::errorMgStashKey() const noexcept
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

    QStringList deleteKeys;
    const QString statusKey    = priv->sessionPrefix + u"status" + token;
    const QVariant statusValue = Session::value(c, statusKey);
    if (!statusValue.isNull()) {
        deleteKeys.append(statusKey);
        c->setStash(priv->statusMsgStashKey, statusValue);
    }

    const QString errorKey    = priv->sessionPrefix + u"error" + token;
    const QVariant errorValue = Session::value(c, errorKey);
    if (!errorValue.isNull()) {
        deleteKeys.append(errorKey);
        c->setStash(priv->errorMsgStashKey, errorValue);
    }

    if (!deleteKeys.isEmpty()) {
        Session::deleteValues(c, deleteKeys);
    }
}

inline QString createToken()
{
    return QString::number(QRandomGenerator::global()->generate() % 99999999)
        .rightJustified(8, u'0', true);
}

QString StatusMessage::error(Context *c, const QString &msg)
{
    QString token;
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_STATUSMESSAGE, "StatusMessage plugin not registered");
        return token;
    }

    token = createToken();
    Session::setValue(c, m_instance->d_ptr->sessionPrefix + u"error" + token, msg);
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
    Session::setValue(c, priv->sessionPrefix + u"error" + token, msg);
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
    Session::setValue(c, m_instance->d_ptr->sessionPrefix + u"status" + token, msg);
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
    Session::setValue(c, priv->sessionPrefix + u"status" + token, msg);
    map.insert(priv->tokenParam, token);
    return map;
}

bool StatusMessage::setup(Application *app)
{
    connect(app, &Application::postForked, this, &StatusMessagePrivate::_q_postFork);
    return true;
}

#include "moc_statusmessage.cpp"
