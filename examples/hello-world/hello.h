/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef HELLO_H
#define HELLO_H

#include <Cutelyst/Application>

class HelloWorld : public Cutelyst::Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "org.cutelyst.HelloWorld")
public:
    Q_INVOKABLE explicit HelloWorld(QObject *parent = 0);
    ~HelloWorld();

    bool init() override;

    bool postFork() override;
};

#endif // HELLO_H
