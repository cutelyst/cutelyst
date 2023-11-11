#ifndef HEADERSTEST_H
#define HEADERSTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <QtCore/QObject>
#include <QtTest/QTest>

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

    // insensitive
    headers.setHeader("x-test"_qba, "test1"_qba);
    QCOMPARE(headers.header("x-test"_qba), "test1"_qba);

    headers.setHeader("x-TEST"_qba, "test2"_qba);
    QCOMPARE(headers.header("x-test"), "test2");

    headers.setHeader("x-TEST"_qba, "test3");
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

    headers.setAuthorizationBasic(u"user"_qs, u"pass"_qs);
    QCOMPARE(headers.authorization(), "Basic dXNlcjpwYXNz");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasicObject().user, u"user"_qs);

    const auto authorizationHeader = "Basic dXNlcjpwYXNz, Bearer xyz"_qba;
    headers.setHeader("Authorization"_qba, authorizationHeader);
    QCOMPARE(headers.authorization(), authorizationHeader);
    QCOMPARE(headers.authorizationBearer(), "xyz"_qba);
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasic(), "user:pass");
    QCOMPARE(headers.authorizationBasicObject().user, u"user"_qs);

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
}

QTEST_MAIN(TestHeaders)
#include "testheaders.moc"

#endif
