/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_P_H
#define CUTELYST_P_H

#include "context.h"
#include "enginerequest.h"
#include "plugin.h"
#include "request_p.h"
#include "response.h"

#include <QStack>
#include <QVariantHash>

class QEventLoop;
namespace Cutelyst {

class Stats;
class ContextPrivate
{
public:
    inline ContextPrivate(Application *_app, Engine *_ngine, Dispatcher *_dispatcher, const QVector<Plugin *> &_plugins)
        : plugins(_plugins)
        , app(_app)
        , engine(_ngine)
        , dispatcher(_dispatcher)
    {
    }

    QString statsStartExecute(Component *code);
    void statsFinishExecute(const QString &statsInfo);

    QStringList error;
    QVariantHash stash;
    QLocale locale;
    QStack<Component *> stack;
    QVector<Plugin *> plugins;
    QVector<Component *> pendingAsync;

    Application *app;
    Engine *engine;
    Dispatcher *dispatcher;

    // Pointer to Engine data
    EngineRequest *engineRequest = nullptr;

    Request *request    = nullptr;
    Response *response  = nullptr;
    Action *action      = nullptr;
    View *view          = nullptr;
    Stats *stats        = nullptr;
    int asyncAction     = 0;
    int actionRefCount  = 0;
    int chainedCaptured = 0;
    int chainedIx       = 0;
    bool detached       = false;
    bool state          = false;
};

class DummyRequest : public QObject
    , public EngineRequest
{
    Q_OBJECT
public:
    DummyRequest(QObject *parent)
        : QObject(parent)
    {
    }

    virtual qint64 doWrite(const char *, qint64) override { return -1; }

    /*!
     * Reimplement this to write the headers back to the client
     */
    virtual bool writeHeaders(quint16, const Headers &) override { return false; }
};

} // namespace Cutelyst

#endif // CUTELYST_P_H
