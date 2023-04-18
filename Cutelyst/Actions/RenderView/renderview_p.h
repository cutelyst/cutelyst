/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RENDERVIEW_P_H
#define RENDERVIEW_P_H

#include "action_p.h"
#include "renderview.h"

#include <Cutelyst/view.h>

namespace Cutelyst {

class RenderViewPrivate : public ActionPrivate
{
public:
    View *view;
};

} // namespace Cutelyst

#endif // RENDERVIEW_P_H
