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
#ifndef CPPROMETHEUS_STANDARD_METRICS_H
#define CPPROMETHEUS_STANDARD_METRICS_H

#include <Cutelyst/Plugins/Prometheus/Prometheus>

#include <QFile>
#include <QDir>

namespace prometheus {
class Gauge;
}

namespace Cutelyst {

class CUTELYST_PLUGIN_PROMETHEUS_EXPORT Prometheus_Standard_Metrics : public QObject
{
public:
    explicit Prometheus_Standard_Metrics(Prometheus *prometheus_plugin);
    virtual ~Prometheus_Standard_Metrics() override;

protected Q_SLOTS:
    void on_update_metrics();

protected:
    Prometheus *m_prometheus_plugin;

    QHash<QString, prometheus::Gauge*> m_standard_metrics;
    unsigned long m_btime = 0;
    QFile m_stat;
    QDir m_fd_dir;
};

}

#endif // CPPROMETHEUS_STANDARD_METRICS_H
