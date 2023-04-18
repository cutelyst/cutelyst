/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTPLUGIN_H
#define CUTELYSTPLUGIN_H

#include <Cutelyst/cutelyst_global.h>

#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

namespace Cutelyst {

class Application;
class CUTELYST_LIBRARY Plugin : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new plugin object with the given Application parent.
     */
    Plugin(Application *parent);

    /**
     * Reimplement this if you need to connect to
     * the signals emitted from Cutelyst::Application
     */
    virtual bool setup(Application *app);
};

} // namespace Cutelyst

#endif // CUTELYSTPLUGIN_H
