/*
 * SPDX-FileCopyrightText: (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class Root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    Root(QObject *app);
    ~Root();

    C_ATTR(index,
           : Path)
    void index(Context *c);

Q_SIGNALS:
    void indexCalled(Cutelyst::Context *c);
};

#endif // ROOT_H
