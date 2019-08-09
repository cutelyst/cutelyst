/*
 * Copyright (C) 2019 Sebastian Held <sebastian.held@gmx.de>
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
#include "prometheus_p.h"
#include "prometheus_controller.h"
#include "prometheus_standard_metrics.h"

#include "application.h"
#include "application_p.h"
#include "context.h"
#include "context_p.h"
#include "engine.h"
#include "stats.h"
#include "stats_p.h"

#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_PROMETHEUS, "cutelyst.plugin.prometheus", QtWarningMsg)

Prometheus::Prometheus(Application *parent) : Plugin(parent), d_ptr(new PrometheusPrivate)
{
    Q_D(Prometheus);

    d->registry = new prometheus::Registry;

    const QVariantMap config = parent->engine()->config(QLatin1String("Cutelyst_Prometheus_Plugin"));
    bool no_controller = config.value(QStringLiteral("no_controller")).toBool();
    if (!no_controller) {
        new Prometheus_Controller(parent, this);
    }
}

Prometheus::~Prometheus()
{
    Q_D(Prometheus);

    delete d->registry;
    delete d_ptr;
}

bool Prometheus::setup(Application *app)
{
    Q_D(Prometheus);

    const QVariantMap config = app->engine()->config(QLatin1String("Cutelyst_Prometheus_Plugin"));
    d->access_token = config.value(QStringLiteral("access_token")).toString();

    connect(app, &Application::afterDispatch, this, &Prometheus::afterDispatch);

    // enable collection of statistics
    app->d_ptr->useStats = true;

    // create standard metrics according to https://prometheus.io/docs/instrumenting/writing_clientlibs/#standard-and-runtime-collectors
    d->no_process_metrics = config.value(QStringLiteral("no_process_metrics"), false).toBool();
    if (!d->no_process_metrics) {
        new Prometheus_Standard_Metrics(this);
    }

    return true;
}

void Prometheus::afterDispatch(Context *c)
{
    Q_D(Prometheus);

    Stats* stats = c->d_ptr->stats;
    if (!stats)
        return;
    StatsPrivate* statsPrivate = stats->d_ptr;

    if (Q_UNLIKELY(!d->gauges[c->controllerName()].contains(c->actionName()))) {
        // create gauges for this action
        PrometheusPrivate::FamilyGauge familyGauge;
        familyGauge.family = &(prometheus::BuildGauge()
            .Name("processing_time")
            .Help("How many seconds does this action take?")
            .Labels({{"controllerName", c->controllerName().toStdString()}, {"actionName", c->actionName().toStdString()}})
            .Register(*d->registry));
        for (const auto &stat : statsPrivate->actions) {
            familyGauge.gauge[stat.action] = &familyGauge.family->Add({{"action", stat.action.toStdString()}});
        }
        d->gauges[c->controllerName()].insert( c->actionName(), familyGauge );
    }

    PrometheusPrivate::FamilyGauge& familyGauge = d->gauges[c->controllerName()][c->actionName()];
    for (const auto &stat : statsPrivate->actions) {
        auto gauge = familyGauge.gauge.value(stat.action);
        if (gauge)
            gauge->Set((stat.end - stat.begin)/1e9);
    }
}

prometheus::Registry* Prometheus::get_Registry()
{
    Q_D(Prometheus);

    return d->registry;
}

void Prometheus::update_metrics()
{
    Q_EMIT on_update_metrics();
}

QString Prometheus::get_Accesstoken() const
{
    Q_D(const Prometheus);

    return d->access_token;
}
