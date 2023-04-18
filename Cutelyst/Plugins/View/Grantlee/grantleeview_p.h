/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef GRANTLEE_VIEW_P_H
#define GRANTLEE_VIEW_P_H

#include "grantleeview.h"
#include "view_p.h"

#include <grantlee/cachingloaderdecorator.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

namespace Cutelyst {

class GrantleeViewPrivate : public ViewPrivate
{
public:
    virtual ~GrantleeViewPrivate() override = default;

    QStringList includePaths;
    QString extension = QStringLiteral(".html");
    QString wrapper;
    QString cutelystVar;
    Grantlee::Engine *engine;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> loader;
    QSharedPointer<Grantlee::CachingLoaderDecorator> cache;
    QHash<QLocale, QTranslator *> translators;
    QHash<QString, QString> translationCatalogs;
};

} // namespace Cutelyst

#endif // GRANTLEE_VIEW_P_H
