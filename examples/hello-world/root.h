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

public:
    C_ATTR(hello,
           : Path
           : AutoArgs)
    void hello(Context *c);

    C_ATTR(json,
           : Local
           : AutoArgs)
    void json(Context *c);

    C_ATTR(echo,
           : Local
           : AutoArgs)
    void echo(Context *c);

    C_ATTR(ws,
           : Local
           : AutoArgs)
    void ws(Context *c);

    C_ATTR(session,
           : Local
           : AutoArgs)
    void session(Context *c);

    C_ATTR(read_session,
           : Local
           : AutoArgs)
    void read_session(Context *c);

    C_ATTR(async,
           : Local
           : AutoArgs)
    void async(Context *c, const QString &timeout);

private Q_SLOTS:
    bool Auto(Cutelyst::Context *c);

private:
    C_ATTR(End, )
    void End(Context *c);
};

#endif // ROOT_H
