/*
 * SPDX-FileCopyrightText: (C) 2019-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEW_P_H
#define VIEW_P_H

#include "component_p.h"

namespace Cutelyst {
class ViewPrivate : public ComponentPrivate
{
public:
    virtual ~ViewPrivate() override = default;

    qint32 minimalSizeToDeflate = -1;
};
} // namespace Cutelyst

#endif // VIEW_P_H
