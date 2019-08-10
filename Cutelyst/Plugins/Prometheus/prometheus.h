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
#ifndef CPPROMETHEUS_H
#define CPPROMETHEUS_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>
#include <Cutelyst/context.h>
#include <Cutelyst/controller.h>

namespace Cutelyst {

class PrometheusPrivate;
class Prometheus_Registry;

/*!
 * \brief Provides metrics for <a href="https://prometheus.io">Prometheus</a>.
 *
 * The Prometheus plugin implements a simple metrics collector. It serves the metrics via the endpoint /metrics.
 * Per default, the access to /metrics is not protected and can be access by every client.
 *
 * The plugin enables statistics for all endpoints. For example, a request to https://yourhost/hilfe looks like this in <a href="https://prometheus.io">Prometheus</a>:
 * \code
 * processing_time{action="/Auto",actionName="index",controllerName="Hilfe",job="MyCutelystApp"}
 * processing_time{action="/hilfe/index",actionName="index",controllerName="Hilfe",job="MyCutelystApp"}
 * processing_time{action="/End",actionName="index",controllerName="Hilfe",job="MyCutelystApp"}
 * processing_time{action="Cutelyst::GrantleeView->execute",actionName="index",controllerName="Hilfe",job="MyCutelystApp"}
 * \endcode
 *
 * To protect the endpoint, add the following section to your ini-file:
 * \code{.ini}
 * [Cutelyst_Prometheus_Plugin]
 * access_token=awQo5SKBUIDO7DfIJOxLju2Jd6561HqXDzwba0fR
 * \endcode
 * The access_token must be the same, configured for %Prometheus. Example prometheus.yml:
 * \code{.yml}
 * scrape_configs:
 *   - job_name: MyCutelystApp
 *     honor_timestamps: false
 *     scrape_interval: 5s
 *     scrape_timeout: 5s
 *     metrics_path: /metrics
 *     scheme: https
 *     bearer_token: awQo5SKBUIDO7DfIJOxLju2Jd6561HqXDzwba0fR
 *     static_configs:
 *       - targets:
 *         - your_host_name
 * \endcode
 *
 * To disable the calculation of Prometheus standard metrics, set no_process_metrics to true:
 * \code{.ini}
 * [Cutelyst_Prometheus_Plugin]
 * no_process_metrics=true
 * \endcode
 * The standard metrics feature currently only works on Linux.
 *
 * <H3>Usage</H3>
 *
 * For general usage simply add the plugin to your application.
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Prometheus/Prometheus>
 *
 * bool MyCutelystApp::init()
 * {
 *     // other initialization stuff
 *
 *     new Prometheus(this);
 *
 *     // more initialization stuff
 * }
 * \endcode
 *
 * To create your own metrics, refer to Prometheus_Standard_Metrics for an example
 *
 *
 * <H3>Configuration file options</H3>
 *
 * There are some options you can set in your application configuration file in the \c Cutelyst_Prometheus_Plugin section.
 *
 * \par no_process_metrics
 * \parblock
 * Boolean value, default: false
 *
 * Can be used to disable process metrics calculation.
 *
 * Metrics calculation only works on Linux. All other systems behave like no_process_metrics is set to true.
 * \endparblock
 *
 * \par no_controller
 * \parblock
 * Boolean value, default: false
 *
 * Can be used to disable the included controller. Check the documentation of Prometheus_Controller_Base
 * for an easy solution to change the endpoint name.
 * \endparblock
 *
 * \par access_token
 * \parblock
 * String value, default: empty
 *
 * If set, the access to the endpoint /metrics is restricted to request with this token set as the Bearer token.
 * \endparblock
 *
 *
 * <H3>Build options</H3>
 * This plugin is not enabled by default. Use <CODE>-DPLUGIN_PROMETHEUS:BOOL=ON</CODE> for your cmake configuration. To link it to your
 * application use \c %Cutelyst::Prometheus. You might be required to also set <CODE>-Dprometheus-cpp_DIR:PATH=</CODE> to point to your
 * https://github.com/jupp0r/prometheus-cpp installation.
 *
 * This plugin only requires the core component from library prometheus-cpp. An example configuration for that library could be:
 * \code{.unparsed}
 * cmake /path/to/prometheus-cpp/src -DENABLE_COMPRESSION=off -DENABLE_PULL=off -DENABLE_PUSH=off -DENABLE_TESTING=off -DCMAKE_CXX_FLAGS=-fPIC
 * \endcode
 *
 * \par Logging category
 * \c cutelyst.plugin.prometheus
 *
 * \since Cutelyst 2.9.0
.
 */
class CUTELYST_PLUGIN_PROMETHEUS_EXPORT Prometheus : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Prometheus)

public:
    Prometheus(Application *parent);
    virtual ~Prometheus() override;

    /**
     * Reimplemented from Plugin::setup().
     */
    virtual bool setup(Application *app) override;

    /*!
     * \brief Get Registry
     *
     * The registry can be used to provide additional metrics.
     * \returns a pointer to the registry
     */
    Prometheus_Registry* registry();

    /*!
     * \brief get_Accesstoken
     * \returns the access token from ini-file
     */
    QString accesstoken() const;

public Q_SLOTS:
    /*!
     * \brief %Request metrics update
     *
     * Is called by Prometheus_Controller to update the metrics it delivers. Called for each access to endpoint /metrics.
     */
    void update_metrics();

Q_SIGNALS:
    /*!
     * \brief Update metrics
     *
     * This signal fires when \ref update_metrics() is called by Prometheus_Controller.
     * Attach to this signal, to update custom metrics.
     */
    void on_update_metrics();

protected:
    PrometheusPrivate *d_ptr;

private:
    void afterDispatch(Context *c);
};

}

#endif // CPPROMETHEUS_H
