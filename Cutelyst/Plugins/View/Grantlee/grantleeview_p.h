/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef GRANTLEE_VIEW_P_H
#define GRANTLEE_VIEW_P_H

#include "grantleeview.h"
#include "component_p.h"

#include <grantlee/engine.h>
#include <grantlee/templateloader.h>
#include <grantlee/cachingloaderdecorator.h>

namespace Cutelyst {

class GrantleeViewPrivate : public ComponentPrivate
{
public:
    QStringList includePaths;
    QString extension = QStringLiteral(".html");
    QString wrapper;
    QString cutelystVar;
    Grantlee::Engine *engine;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> loader;
    QSharedPointer<Grantlee::CachingLoaderDecorator> cache;
    QHash<QLocale, QTranslator*> translators;
    QHash<QString, QString> translationCatalogs;
};

}

#endif // GRANTLEE_VIEW_P_H
