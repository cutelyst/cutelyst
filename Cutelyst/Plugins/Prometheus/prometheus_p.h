#ifndef CPPROMETHEUS_P_H
#define CPPROMETHEUS_P_H

#include "prometheus.h"

#include <prometheus/registry.h>
#include <QSharedPointer>

namespace Cutelyst {

class PrometheusPrivate
{
public:
    QString access_token;

    struct FamilyGauge {
        prometheus::Family<prometheus::Gauge>* family;
        QHash<QString, prometheus::Gauge*> gauge;
    };
    QHash<QString, QHash<QString, FamilyGauge> > gauges;
};

}

#endif // CPPROMETHEUS_P_H
