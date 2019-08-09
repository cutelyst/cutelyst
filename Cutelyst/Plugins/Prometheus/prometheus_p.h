#ifndef CPPROMETHEUS_P_H
#define CPPROMETHEUS_P_H

#include "prometheus.h"

#include <prometheus/registry.h>

namespace Cutelyst {

class PrometheusPrivate
{
public:
    QString access_token;
    bool no_process_metrics = false;

    prometheus::Registry* registry = nullptr;
    struct FamilyGauge {
        prometheus::Family<prometheus::Gauge>* family;
        QHash<QString, prometheus::Gauge*> gauge;
    };
    QHash<QString, QHash<QString, FamilyGauge> > gauges;
};

}

#endif // CPPROMETHEUS_P_H
