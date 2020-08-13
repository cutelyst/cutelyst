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
#ifndef CPPROMETHEUS_CONTROLLER_H
#define CPPROMETHEUS_CONTROLLER_H

#include <Cutelyst/Controller>
#include <Cutelyst/Plugins/Prometheus/Prometheus>

namespace Cutelyst {

class Prometheus_ControllerPrivate;

/*!
 * \brief The Prometheus_Controller_Base class
 *
 * To change the default path "/metrics", set no_controller=true in the ini-file and derive from this class:
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Prometheus/prometheus_controller.h>
 * class CUTELYST_PLUGIN_PROMETHEUS_EXPORT mycontroller : public Prometheus_Controller_Base
 * {
 *     Q_OBJECT
 *
 * public:
 *     using Prometheus_Controller_Base::Prometheus_Controller_Base;
 *
 *     C_ATTR(endpoint, :Path("/YOURMETRICSPATH") :AutoArgs)
 *     virtual void endpoint(Context *c) override {Prometheus_Controller_Base::endpoint(c);}
 * };
 * \endcode
 * Instantiate mycontroller in your main file:
 * \code{.cpp}
 * auto p = new Prometheus(this);
 * new mycontroller(this, p);
 * \endcode
 */
class CUTELYST_PLUGIN_PROMETHEUS_EXPORT Prometheus_Controller_Base : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
    Q_DECLARE_PRIVATE(Prometheus_Controller)

public:
    explicit Prometheus_Controller_Base(QObject* parent, Prometheus *prometheus_plugin);
    virtual ~Prometheus_Controller_Base() override;

    virtual void endpoint(Context *c);

protected:
    Prometheus *m_prometheus_plugin;
    Prometheus_ControllerPrivate* d_ptr;
};

class CUTELYST_PLUGIN_PROMETHEUS_EXPORT Prometheus_Controller : public Prometheus_Controller_Base
{
    Q_OBJECT

public:
    using Prometheus_Controller_Base::Prometheus_Controller_Base;

    C_ATTR(endpoint, :Path("/metrics") :AutoArgs)
    virtual void endpoint(Context *c) override {Prometheus_Controller_Base::endpoint(c);}
};

}

#endif //CPPROMETHEUS_CONTROLLER_H

