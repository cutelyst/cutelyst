/*
 * SPDX-FileCopyrightText: (C) 2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYST_COOKIE_P_H
#define CUTELYST_COOKIE_P_H

#include "cookie.h"

#include <QSharedData>

namespace Cutelyst {

class CookiePrivate : public QSharedData
{
public:
    CookiePrivate() = default;

    Cookie::SameSite sameSite = Cookie::SameSite::Default;
};

} // namespace Cutelyst

#endif // CUTELYST_COOKIE_P_H
