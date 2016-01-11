/*
  This file is part of the Grantlee template system.

  Copyright (c) 2009,2010 Stephen Kelly <steveire@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version
  2.1 of the Licence, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef HEADERSTEST_H
#define HEADERSTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>

#include "headers.h"
#include "coverageobject.h"

using namespace Cutelyst;

class TestHeaders : public CoverageObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCombining();
};

void TestHeaders::testCombining()
{
    Headers headers;

    // insensitive and underscore
    headers.setHeader(QStringLiteral("x-test"), QStringLiteral("test1"));
    QCOMPARE(headers.header(QStringLiteral("x-test")), QStringLiteral("test1"));

    headers.setHeader(QStringLiteral("x-TEST"), QStringLiteral("test2"));
    QCOMPARE(headers.header(QStringLiteral("x-test")), QStringLiteral("test2"));

    headers.setHeader(QStringLiteral("x-TEST"), QStringLiteral("test3"));
    QCOMPARE(headers.header(QStringLiteral("x_test")), QStringLiteral("test3"));

    // header helpers
    headers.setContentType(QStringLiteral("text/html"));
    QCOMPARE(headers.contentType(), QStringLiteral("text/html"));

    headers.setContentLength(654321);
    QCOMPARE(headers.contentLength(), 654321);

    QDateTime dt = QDateTime::currentDateTime();
    QTime time = dt.time();
    // make sure ms is 0 as we loose this precision
    time.setHMS(time.hour(), time.minute(), time.second());
    dt.setTime(time);

    headers.setDateWithDateTime(dt);
    QCOMPARE(headers.date(), dt);
    QCOMPARE(headers.date(), dt.toUTC());
}

QTEST_MAIN( TestHeaders )
#include "testheaders.moc"

#endif
