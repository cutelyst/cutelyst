/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "common.h"
#include "context_p.h"
#include "engine_p.h"
#include "request_p.h"
#include "response_p.h"

#include <QByteArray>
#include <QDir>
#include <QJsonDocument>
#include <QSettings>
#include <QThread>
#include <QUrl>

using namespace Cutelyst;

/**
 \ingroup core
 \class Cutelyst::Engine engine.h Cutelyst/Engine
 \brief The %Cutelyst %Engine.

 This class is responsible receiving the request
 and sending the response. It must be reimplemented
 by real HTTP engines due some pure virtual methods.

 The subclass must create an engine per thread (worker core),
 if the Application passed to the constructor has a worker core
 greater than 0 it will issue a new Application instance, failing
 to do so a fatal error is generated (usually indicating that
 the Application does not have a Q_INVOKABLE constructor).
*/

/*!
 @param app The application loaded
 @param workerCore The thread number
 @param opts The configuation options
 */
Engine::Engine(Cutelyst::Application *app, int workerCore, const QVariantMap &opts)
    : d_ptr(new EnginePrivate)
{
    Q_D(Engine);

    connect(
        this, &Engine::processRequestAsync, this, &Engine::processRequest, Qt::QueuedConnection);

    d->opts       = opts;
    d->workerCore = workerCore;
    d->app        = app;
}

Engine::~Engine()
{
    delete d_ptr;
}

Application *Engine::app() const
{
    Q_D(const Engine);
    Q_ASSERT(d->app);
    return d->app;
}

int Engine::workerCore() const
{
    Q_D(const Engine);
    return d->workerCore;
}

bool Engine::initApplication()
{
    Q_D(Engine);

    if (thread() != QThread::currentThread()) {
        qCCritical(CUTELYST_ENGINE) << "Cannot init application on a different thread";
        return false;
    }

    if (!d->app->setup(this)) {
        qCCritical(CUTELYST_ENGINE) << "Failed to setup application";
        return false;
    }

    return true;
}

bool Engine::postForkApplication()
{
    Q_D(Engine);

    if (!d->app) {
        qCCritical(CUTELYST_ENGINE) << "Failed to postForkApplication on a null application";
        return false;
    }

    QThread::currentThread()->setObjectName(QString::number(d->workerCore));

    return d->app->enginePostFork();
}

Headers &Engine::defaultHeaders()
{
    Q_D(Engine);
    return d->app->defaultHeaders();
}

void Engine::processRequest(EngineRequest *request)
{
    Q_D(Engine);
    d->app->handleRequest(request);
}

QVariantMap Engine::opts() const
{
    Q_D(const Engine);
    return d->opts;
}

QVariantMap Engine::config(const QString &entity) const
{
    Q_D(const Engine);
    return d->config.value(entity).toMap();
}

void Engine::setConfig(const QVariantMap &config)
{
    Q_D(Engine);
    d->config = config;
}

QVariantMap Engine::loadIniConfig(const QString &filename)
{
    QVariantMap ret;
    QSettings settings(filename, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        qCWarning(CUTELYST_ENGINE) << "Failed to load INI file:" << settings.status();
        return ret;
    }

    const auto groups = settings.childGroups();
    for (const QString &group : groups) {
        QVariantMap configGroup;
        settings.beginGroup(group);
        const auto child = settings.childKeys();
        for (const QString &key : child) {
            configGroup.insert(key, settings.value(key));
        }
        settings.endGroup();
        ret.insert(group, configGroup);
    }

    return ret;
}

QVariantMap Engine::loadJsonConfig(const QString &filename)
{
    QVariantMap ret;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ret;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    ret = doc.toVariant().toMap();

    return ret;
}

#include "moc_engine.cpp"
