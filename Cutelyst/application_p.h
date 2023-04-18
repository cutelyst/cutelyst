/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_APPLICATION_P_H
#define CUTELYST_APPLICATION_P_H

#include "application.h"
#include "componentfactory.h"
#include "context.h"
#include "dispatcher.h"
#include "engine.h"
#include "plugin.h"

namespace Cutelyst {

class ApplicationPrivate
{
    Q_DECLARE_PUBLIC(Application)
public:
    void setupHome();
    void setupChildren(const QObjectList &children);

    void logRequest(Request *req);
    void logRequestParameters(const ParamsMultiMap &params, const QString &title);
    void logRequestUploads(const QVector<Upload *> &uploads);
    Component *createComponentPlugin(const QString &name, QObject *parent, const QString &directory);

    Application *q_ptr;
    Dispatcher *dispatcher;
    QVector<Plugin *> plugins;
    QHash<QString, Controller *> controllersHash;
    QVector<Controller *> controllers;
    QHash<QStringView, View *> views;
    QVector<DispatchType *> dispatchers;
    QMap<QString, ComponentFactory *> factories;
    Headers headers;
    QVariantMap config;
    Engine *engine;
    bool useStats;
    bool init = false;
    QHash<QLocale, QVector<QTranslator *>> translators;
};

} // namespace Cutelyst

#endif // CUTELYST_APPLICATION_P_H
