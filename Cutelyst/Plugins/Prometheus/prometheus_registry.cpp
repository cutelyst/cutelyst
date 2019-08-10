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
#include "prometheus_registry.h"

#include <prometheus/registry.h>

#include <QDebug>

#include <unistd.h>

using namespace Cutelyst;

Prometheus_Registry::Prometheus_Registry()
{
}

Prometheus_Registry::~Prometheus_Registry()
{
}

prometheus::Registry* Prometheus_Registry::registry()
{
    return &m_registry;
}

Prometheus_Registry::FamilyGauge& Prometheus_Registry::familyGauge( const QString& controllerName, const QString& actionName )
{
    if (Q_LIKELY(m_gauges.contains(controllerName) and m_gauges[controllerName].contains(actionName))) {
        return m_gauges[controllerName][actionName];
    }

    FamilyGauge familyGauge;
    familyGauge.family = &(prometheus::BuildGauge()
        .Name("processing_time")
        .Help("How many seconds does this action take?")
        .Labels({{"controllerName", controllerName.toStdString()}, {"actionName", actionName.toStdString()}})
        .Register(m_registry));

    m_gauges[controllerName][actionName] = familyGauge;

    return m_gauges[controllerName][actionName];
}

prometheus::Gauge* Prometheus_Registry::gauge( FamilyGauge& familyGauge, const QString& action )
{
    if (Q_LIKELY(familyGauge.gauge.contains(action))) {
        return familyGauge.gauge[action];
    }

    familyGauge.gauge[action] = &familyGauge.family->Add({{"action", action.toStdString()}});

    return familyGauge.gauge[action];
}

void Prometheus_Registry::setGauge( const QString& controllerName, const QString& actionName, const QString& action, double value )
{
    FamilyGauge family = familyGauge( controllerName, actionName );
    prometheus::Gauge* g = gauge( family, action );
    g->Set( value );
}

void Prometheus_Registry::setGauge( FamilyGauge& familyGauge, const QString& action, double value )
{
    prometheus::Gauge* g = gauge( familyGauge, action );
    g->Set( value );
}
