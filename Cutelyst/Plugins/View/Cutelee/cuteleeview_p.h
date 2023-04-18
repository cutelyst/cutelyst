/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELEE_VIEW_P_H
#define CUTELEE_VIEW_P_H

#include "cuteleeview.h"
#include "view_p.h"

#include <cutelee/cachingloaderdecorator.h>
#include <cutelee/engine.h>
#include <cutelee/templateloader.h>

namespace Cutelyst {

class CuteleeViewPrivate : public ViewPrivate
{
public:
    virtual ~CuteleeViewPrivate() override = default;

    QStringList includePaths;
    QString extension = QStringLiteral(".html");
    QString wrapper;
    QString cutelystVar;
    Cutelee::Engine *engine;
    std::shared_ptr<Cutelee::FileSystemTemplateLoader> loader;
    std::shared_ptr<Cutelee::CachingLoaderDecorator> cache;
    QHash<QLocale, QTranslator *> translators;
    QMultiHash<QString, QString> translationCatalogs;
    void initEngine();
};

} // namespace Cutelyst

#endif // CUTELEE_VIEW_P_H
