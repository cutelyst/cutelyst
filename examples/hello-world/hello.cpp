/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hello.h"

#include "root.h"

#include <Cutelyst/Plugins/Session/Session>

#include <QCoreApplication>
#include <QDebug>

using namespace Qt::StringLiterals;

HelloWorld::HelloWorld(QObject *parent)
    : Cutelyst::Application(parent)
{
    QCoreApplication::setApplicationName(u"HelloWorld"_s);
    qDebug() << "HelloWorld::HelloWorld" << QCoreApplication::applicationPid();
}

HelloWorld::~HelloWorld()
{
}

bool HelloWorld::init()
{
    new Root(this);

    if (qEnvironmentVariableIsSet("TEST_SESSION")) {
        new Session(this);
    }

    return true;
}

bool HelloWorld::postFork()
{
    qDebug() << "HelloWorld::postFork" << QCoreApplication::applicationPid();

    return true;
}
