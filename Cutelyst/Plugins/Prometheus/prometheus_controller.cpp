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
#include "prometheus_controller.h"
#include "prometheus_controller_p.h"
#include "prometheus_p.h"
#include "prometheus_registry.h"

#include <prometheus/text_serializer.h>

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Session/Session>
#include <QDebug>

using namespace Cutelyst;

Prometheus_Controller_Base::Prometheus_Controller_Base(QObject* parent, Prometheus *prometheus_plugin) : Controller(parent),
    m_prometheus_plugin(prometheus_plugin), d_ptr(new Prometheus_ControllerPrivate)
{
}

Prometheus_Controller_Base::~Prometheus_Controller_Base()
{
    delete d_ptr;
}

void Prometheus_Controller_Base::endpoint(Context *c)
{
    Q_D(Prometheus_Controller);

    // check authorization
    QString access_token = m_prometheus_plugin->accesstoken();
    if (!access_token.isEmpty()) {
        QString auth = c->request()->headers().authorization();
        if (!auth.startsWith(QLatin1String("Bearer ")) || auth.mid(7) != access_token) {
            c->response()->setStatus(Response::Unauthorized);
            c->response()->headers().setWwwAuthenticate(QLatin1String("Bearer"));
            c->response()->setBody(QStringLiteral("Unauthorized")); // prevent RenderView from executing
            return;
        }
    }

    m_prometheus_plugin->update_metrics();

    std::string metrics = prometheus::TextSerializer().Serialize( d->CollectMetrics( m_prometheus_plugin->registry()->registry() ) );
    if (!metrics.empty()) {
        c->response()->setBody( QString::fromStdString(metrics) );
        c->response()->setContentType( QStringLiteral("text/plain; version=0.0.4") );
    } else {
        c->response()->setStatus(Response::NoContent);
    }
}

std::vector<prometheus::MetricFamily> Prometheus_ControllerPrivate::CollectMetrics( prometheus::Registry* registry ) const
{
    auto collected_metrics = std::vector<prometheus::MetricFamily>{};

    if (!registry) {
        return collected_metrics;
    }

    auto&& metrics = registry->Collect();
    collected_metrics.insert(collected_metrics.end(),
                             std::make_move_iterator(metrics.begin()),
                             std::make_move_iterator(metrics.end()));

    return collected_metrics;
}
