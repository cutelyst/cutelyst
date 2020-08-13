#ifndef CPPROMETHEUS_CONTROLLER_P_H
#define CPPROMETHEUS_CONTROLLER_P_H

#include <Cutelyst/Controller>
#include <Cutelyst/Plugins/Prometheus/Prometheus>

#include <prometheus/registry.h>

namespace Cutelyst {

class Prometheus_ControllerPrivate
{
public:
    std::vector<prometheus::MetricFamily> CollectMetrics( prometheus::Registry* registry ) const;
};

}

#endif //CPPROMETHEUS_CONTROLLER_P_H
