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
#include "prometheus_standard_metrics.h"
#include "prometheus_registry.h"

#include <prometheus/registry.h>

#include <QDebug>

#include <unistd.h>

using namespace Cutelyst;

Prometheus_Standard_Metrics::Prometheus_Standard_Metrics(Prometheus *prometheus_plugin, int update_interval_s) :
    QObject(prometheus_plugin)
{
    prometheus::Registry* registry = prometheus_plugin->registry()->registry();

    prometheus::Family<prometheus::Gauge>* family = &(prometheus::BuildGauge()
        .Name("process_cpu_seconds_total")
        .Help("Total user and system CPU time spent in seconds.")
        .Register(*registry));
    m_standard_metrics[QStringLiteral("process_cpu_seconds_total")] = &family->Add({});

    family = &(prometheus::BuildGauge()
        .Name("process_virtual_memory_bytes")
        .Help("Virtual memory size in bytes.")
        .Register(*registry));
    m_standard_metrics[QStringLiteral("process_virtual_memory_bytes")] = &family->Add({});

    family = &(prometheus::BuildGauge()
        .Name("process_resident_memory_bytes")
        .Help("Resident memory size in bytes.")
        .Register(*registry));
    m_standard_metrics[QStringLiteral("process_resident_memory_bytes")] = &family->Add({});

    family = &(prometheus::BuildGauge()
        .Name("process_start_time_seconds")
        .Help("Start time of the process since unix epoch in seconds.")
        .Register(*registry));
    m_standard_metrics[QStringLiteral("process_start_time_seconds")] = &family->Add({});

    family = &(prometheus::BuildGauge()
        .Name("process_open_fds")
        .Help("Number of open file descriptors.")
        .Register(*registry));
    m_standard_metrics[QStringLiteral("process_open_fds")] = &family->Add({});

#ifdef Q_OS_LINUX
    // get boot time since unix epoch
    QFile proc_stat(QStringLiteral("/proc/stat"));
    if (proc_stat.open(QFile::ReadOnly)) {
        while (true) {
            QByteArray line = proc_stat.readLine().trimmed();
            if (line.startsWith("btime ")) {
                m_btime = line.mid(6).toULong();
                break;
            }
            if (line.isEmpty())
                break;
        }
    }
    proc_stat.close();

    // prepare access to /proc filesystem for standard metrics calculation
    m_stat.setFileName(QStringLiteral("/proc/self/stat"));
    m_stat.open(QFile::ReadOnly);

    // prepare access to /proc/self/fd filesystem for standard metrics calculation
    m_fd_dir.setPath(QStringLiteral("/proc/self/fd"));
#endif

    // attach to update signal
    if (update_interval_s > 0) {
        connect(&m_updateTimer, &QTimer::timeout, this, &Prometheus_Standard_Metrics::update_metrics);
        m_updateTimer.start(update_interval_s * 1000);
    }
}

Prometheus_Standard_Metrics::~Prometheus_Standard_Metrics()
{
}

void Prometheus_Standard_Metrics::update_metrics()
{
    // calculate standard metrics
#ifdef Q_OS_LINUX
    m_stat.seek(0);
    QByteArray line = m_stat.readAll();
    int pos = line.lastIndexOf(')');
    if (pos != -1) {
        line = line.mid(pos+2);
        QList<QByteArray> field = line.split(' ');
        double utime = field.value(11).toDouble() / sysconf(_SC_CLK_TCK);
        double stime = field.value(12).toDouble() / sysconf(_SC_CLK_TCK);
        double starttime = field.value(19).toDouble() / sysconf(_SC_CLK_TCK);
        unsigned long vsize = field.value(20).toULong();
        unsigned long rss = field.value(21).toULong();
        auto gauge = m_standard_metrics[QStringLiteral("process_cpu_seconds_total")];
        gauge->Set(utime + stime);
        gauge = m_standard_metrics[QStringLiteral("process_start_time_seconds")];
        gauge->Set(m_btime + starttime);
        gauge = m_standard_metrics[QStringLiteral("process_virtual_memory_bytes")];
        gauge->Set(vsize);
        gauge = m_standard_metrics[QStringLiteral("process_resident_memory_bytes")];
        gauge->Set(rss);
    }
    auto gauge = m_standard_metrics[QStringLiteral("process_open_fds")];
    gauge->Set(m_fd_dir.count() - 2); // subtract "." and ".." folders
#endif
}
