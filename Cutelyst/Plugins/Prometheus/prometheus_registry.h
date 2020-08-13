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
#ifndef CPPROMETHEUS_REGISTRY_H
#define CPPROMETHEUS_REGISTRY_H

#include <Cutelyst/Plugins/Prometheus/Prometheus>

#include <prometheus/registry.h> // FIXME put into private header

#include <QFile>
#include <QDir>
#include <QTimer>

namespace prometheus {
class Gauge;
}

namespace Cutelyst {

class CUTELYST_PLUGIN_PROMETHEUS_EXPORT Prometheus_Registry
{
public:
    Prometheus_Registry();
    ~Prometheus_Registry();

    // disable copy
private:
    Q_DISABLE_COPY(Prometheus_Registry)

public:
    struct FamilyGauge {
        prometheus::Family<prometheus::Gauge>* family;
        QHash<QString, prometheus::Gauge*> gauge;
    };

    prometheus::Registry* registry();

    Prometheus_Registry::FamilyGauge& familyGauge( const QString& controllerName, const QString& actionName );
    prometheus::Gauge* gauge( FamilyGauge& familyGauge, const QString& action );
    void setGauge( const QString& controllerName, const QString& actionName, const QString& action, double value );
    void setGauge( FamilyGauge& familyGauge, const QString& action, double value );

protected:
    prometheus::Registry m_registry;
    QHash<QString, QHash<QString, FamilyGauge>> m_gauges;
};

} // namespace

#endif // CPPROMETHEUS_REGISTRY_H
