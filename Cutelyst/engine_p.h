/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_ENGINE_P_H
#define CUTELYST_ENGINE_P_H

#include "engine.h"

namespace Cutelyst {

class EnginePrivate
{
public:
    QVariantMap opts;
    QVariantMap config;
    Application *app;
    int workerCore;
};

} // namespace Cutelyst

#endif // CUTELYST_ENGINE_P_H
