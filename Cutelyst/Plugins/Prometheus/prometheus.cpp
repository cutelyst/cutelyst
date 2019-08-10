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
#include "prometheus_registry.h"

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

// we need one Registry per application
static QScopedPointer<Prometheus_Registry> prometheus_registry;

Prometheus::Prometheus(Application *parent) : Plugin(parent), d_ptr(new PrometheusPrivate)
{
    const QVariantMap config = parent->engine()->config(QLatin1String("Cutelyst_Prometheus_Plugin"));

    if (!prometheus_registry) {
        prometheus_registry.reset( new Prometheus_Registry );

        // create standard metrics according to https://prometheus.io/docs/instrumenting/writing_clientlibs/#standard-and-runtime-collectors
        // once per registry
        bool no_process_metrics = config.value(QStringLiteral("no_process_metrics"), false).toBool();
        if (!no_process_metrics) {
            int update_interval_s = config.value(QStringLiteral("update_interval_s"), 5).toInt();
            new Prometheus_Standard_Metrics(this, update_interval_s);
        }
    }

    bool no_controller = config.value(QStringLiteral("no_controller")).toBool();
    if (!no_controller) {
        new Prometheus_Controller(parent, this);
    }
}

Prometheus::~Prometheus()
{
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

    return true;
}

void Prometheus::afterDispatch( Context *c )
{
    const Stats* stats = c->d_ptr->stats;
    if (Q_UNLIKELY(!stats))
        return;
    const StatsPrivate* statsPrivate = stats->d_ptr;

    auto familyGauge = registry()->familyGauge( c->controllerName(), c->actionName() );
    for (const auto& stat : statsPrivate->actions) {
        registry()->setGauge( familyGauge, stat.action, (stat.end - stat.begin)/1e9 );
    }
}

Prometheus_Registry* Prometheus::registry()
{
    return prometheus_registry.data();
}

void Prometheus::update_metrics()
{
    Q_EMIT on_update_metrics();
}

QString Prometheus::accesstoken() const
{
    Q_D(const Prometheus);

    return d->access_token;
}
