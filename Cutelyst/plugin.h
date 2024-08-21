/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTPLUGIN_H
#define CUTELYSTPLUGIN_H

#include <Cutelyst/cutelyst_export.h>

#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

namespace Cutelyst {

class Application;
/**
 * @ingroup core
 * @class Cutelyst::Plugin plugin.h Cutelyst/Plugin
 * @brief Base class for %Cutelyst \ref plugins.
 *
 * Base class for %Cutelyst \ref plugins.
 */
class CUTELYST_EXPORT Plugin : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new %Plugin object with the given Application \a parent.
     */
    Plugin(Application *parent);

    /**
     * Reimplement this if you need to connect to
     * the signals emitted from Cutelyst::Application.
     */
    virtual bool setup(Application *app);
};

} // namespace Cutelyst

#endif // CUTELYSTPLUGIN_H
