/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "view.h"
#include "context.h"
#include "response.h"
#include "request.h"

using namespace Cutelyst;

CutelystView::CutelystView(QObject *parent) :
    QObject(parent)
{
}

bool CutelystView::process(Context *ctx)
{
    if (ctx->res()->contentType().isEmpty()) {
        ctx->res()->setContentType("text/html; charset=utf-8");
    }

    if (ctx->req()->method() == "HEAD") {
        return true;
    }

    if (!ctx->res()->body().isNull()) {
        return true;
    }

    if (ctx->res()->status() == 204 ||
            (ctx->res()->status() >= 300 &&
             ctx->res()->status() < 400)) {
            return true;
    }

    return render(ctx);
}

bool CutelystView::render(Context *ctx)
{
    Q_UNUSED(ctx)
    qFatal("directly inherits from Catalyst::View. You need to\n"
           " inherit from a subclass like Cutelyst::View::ClearSilver instead.\n");
    return false;
}
