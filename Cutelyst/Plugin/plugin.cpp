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

#include "plugin.h"

#include "context.h"

using namespace Cutelyst;
using namespace Plugin;

AbstractPlugin::AbstractPlugin(QObject *parent) :
    QObject(parent)
{
}

bool AbstractPlugin::setup(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

QVariant AbstractPlugin::pluginProperty(Context *ctx, const QString &key, const QVariant &defaultValue) const
{
    return ctx->pluginProperty(const_cast<Plugin::AbstractPlugin*>(this), key, defaultValue);
}

void AbstractPlugin::setPluginProperty(Context *ctx, const QString &key, const QVariant &value)
{
    ctx->setPluginProperty(this, key, value);
}
