/*
 * SPDX-FileCopyrightText: (C) 2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMBEDDEDAPP_H
#define EMBEDDEDAPP_H

#include <Cutelyst/Application>

class EmbeddedApp : public Cutelyst::Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "org.cutelyst.EmbeddedApp")
public:
    Q_INVOKABLE explicit EmbeddedApp(QObject *parent = 0);
    ~EmbeddedApp();

    bool init() override;

Q_SIGNALS:
    void indexCalled(Cutelyst::Context *c);
};

#endif // EMBEDDEDAPP_H
