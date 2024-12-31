#ifndef HEADERSTEST_H
#define HEADERSTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class TestHeaders : public CoverageObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCombining();
};

void TestHeaders::testCombining()
{
    Headers headers;

    // insensitive
    headers.setHeader("x-test"_ba, "test1"_ba);
    QCOMPARE(headers.header("x-test"), "test1"_ba);
    QCOMPARE(headers.header(u"x-test"), "test1"_ba);
    QCOMPARE(headers.header(u8"x-test"), "test1"_ba);
    QCOMPARE(headers.header(u"x-test"_s), "test1"_ba);
    QCOMPARE(headers.header("x-test"_ba), "test1"_ba);

    headers.setHeader("x-TEST"_ba, "test2"_ba);
    QCOMPARE(headers.header("x-test"), "test2");

    headers.setHeader("x-TEST"_ba, "test3");
    QCOMPARE(headers.header("x-test"), "test3");

    // header helpers
    headers.setContentType("TEXT/HTML");
    QCOMPARE(headers.contentType(), "text/html");
    QCOMPARE(headers.header("content-type"), "TEXT/HTML");
    QCOMPARE(headers.contentTypeCharset(), QByteArray{});

    headers.setContentType("TEXT/HTML; charset=utf-8");
    QCOMPARE(headers.contentType(), "text/html");
    QCOMPARE(headers.header("content-type"), "TEXT/HTML; charset=utf-8");

    headers.setContentTypeCharset("utf-8");
    QCOMPARE(headers.contentTypeCharset(), "UTF-8");
    // Make sure content-type still fine
    QCOMPARE(headers.contentType(), "text/html");
    QCOMPARE(headers.header("content-type"), "TEXT/HTML; charset=utf-8");

    headers.setContentTypeCharset("utf-16");
    QCOMPARE(headers.header("content-type"), "TEXT/HTML; charset=utf-16");

    // This removes the charset part...
    headers.setContentType("text/plain");
    QCOMPARE(headers.header("content-type"), "text/plain");

    headers.setContentTypeCharset("utf-16");
    headers.setContentTypeCharset({});
    QCOMPARE(headers.header("content-type"), "text/plain");

    headers.setContentType("text/plain");
    QCOMPARE(headers.contentIsText(), true);
    QCOMPARE(headers.contentIsHtml(), false);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), false);

    headers.setContentType("text/html");
    QCOMPARE(headers.contentIsText(), true);
    QCOMPARE(headers.contentIsHtml(), true);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), false);

    headers.setContentType("application/xhtml+xml");
    QCOMPARE(headers.contentIsText(), false);
    QCOMPARE(headers.contentIsHtml(), true);
    QCOMPARE(headers.contentIsXHtml(), true);
    QCOMPARE(headers.contentIsXml(), true);

    headers.setContentType("application/xml");
    QCOMPARE(headers.contentIsText(), false);
    QCOMPARE(headers.contentIsHtml(), false);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), true);

    headers.setContentLength(654321);
    QCOMPARE(headers.contentLength(), 654321);

    headers.setContentLength(123456);
    QCOMPARE(headers.contentLength(), 123456);

    headers.setContentEncoding("utf-8");
    QCOMPARE(headers.contentEncoding(), "utf-8");

    QDateTime dt = QDateTime::currentDateTime();
    QTime time   = dt.time();
    // make sure ms is 0 as we loose this precision
    time.setHMS(time.hour(), time.minute(), time.second());
    dt.setTime(time);

    headers.setDateWithDateTime(dt);
    QCOMPARE(headers.date(), dt);
    QCOMPARE(headers.date(), dt.toUTC());

    headers.setAuthorizationBasic(u"user"_s, u"pass"_s);
    QCOMPARE(headers.authorization(), "Basic dXNlcjpwYXNz");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasicObject().user, u"user"_s);

    const auto authorizationHeader = "Basic dXNlcjpwYXNz, Bearer xyz"_ba;
    headers.setHeader("Authorization"_ba, authorizationHeader);
    QCOMPARE(headers.authorization(), authorizationHeader);
    QCOMPARE(headers.authorizationBearer(), "xyz"_ba);
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasicObject().user, u"user"_s);

    Headers copy = headers;
    QCOMPARE(copy, headers);

    Headers copy2(headers);
    QCOMPARE(copy2, headers);

    Headers headersEmpty;
    QVERIFY(headersEmpty != headers);

    headers.clear();
    QCOMPARE(headers.contentType().isEmpty(), true);
    QCOMPARE(headers.contentType().isNull(), true);

    headers.clear();
    headers.setContentType("");
    QCOMPARE(headers.contentType().isEmpty(), true);
    QCOMPARE(headers.contentType().isNull(), false);

    headers.clear();
    headers.setContentDisposition("");
    QCOMPARE(headers.contentDisposition().isEmpty(), true);
    QCOMPARE(headers.contentDisposition().isNull(), false);

    headers.clear();
    headers.setContentDisposition("attachment; filename=\"foo.txt\"");
    QCOMPARE(headers.contentDisposition(), "attachment; filename=\"foo.txt\"");

    headers.clear();
    headers.setContentDispositionAttachment();
    QCOMPARE(headers.contentDisposition(), "attachment");

    headers.clear();
    headers.setContentDispositionAttachment("foo.txt");
    QCOMPARE(headers.contentDisposition(), "attachment; filename=\"foo.txt\"");

    headers.setHeader("x-hbn-foo", "bar");
    QVERIFY(headers.contains("x-hbn-foo"));

    headers.removeHeader("x-hbn-foo");
    QVERIFY(!headers.contains("x-hbn-foo"));

    {
        Headers h1;
        h1.pushHeader("b", "1");
        h1.pushHeader("b", "2");
        h1.pushHeader("a", "1");
        h1.pushHeader("b", "3");

        Headers h2{
            {"b", "1"},
            {"b", "2"},
            {"a", "1"},
            {"b", "3"},
        };
        QCOMPARE(h1, h2);

        h1.setHeader("b", "3");
        Headers h3{
            {"b", "3"},
            {"a", "1"},
        };
        QCOMPARE(h1, h3);
    }
}

QTEST_MAIN(TestHeaders)
#include "testheaders.moc"

#endif
