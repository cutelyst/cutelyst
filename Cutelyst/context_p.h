/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CUTELYST_P_H
#define CUTELYST_P_H

#include "context.h"
#include "plugin.h"
#include "response.h"
#include "request_p.h"
#include "enginerequest.h"

#include <QVariantHash>
#include <QStack>

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
    { }

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

    Request *request = nullptr;
    Response *response = nullptr;
    Action *action = nullptr;
    View *view = nullptr;
    Stats *stats = nullptr;
    bool asyncDetached = false;
    bool detached = false;
    bool state = false;
};

class DummyRequest : public QObject, public EngineRequest
{
    Q_OBJECT
public:
    DummyRequest(QObject *parent) : QObject(parent) {}

    virtual qint64 doWrite(const char *, qint64) override { return -1; }

    /*!
     * Reimplement this to write the headers back to the client
     */
    virtual bool writeHeaders(quint16 , const Headers &) override { return false; }
};

}

#endif // CUTELYST_P_H
