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
    headers.setContentType(QStringLiteral("TEXT/HTML"));
    QCOMPARE(headers.contentType(), QStringLiteral("text/html"));
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("TEXT/HTML"));
    QCOMPARE(headers.contentTypeCharset(), QString());

    headers.setContentType(QStringLiteral("TEXT/HTML; charset=utf-8"));
    QCOMPARE(headers.contentType(), QStringLiteral("text/html"));
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("TEXT/HTML; charset=utf-8"));

    headers.setContentTypeCharset(QStringLiteral("utf-8"));
    QCOMPARE(headers.contentTypeCharset(), QStringLiteral("UTF-8"));
    // Make sure content-type still fine
    QCOMPARE(headers.contentType(), QStringLiteral("text/html"));
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("TEXT/HTML; charset=utf-8"));

    headers.setContentTypeCharset(QStringLiteral("utf-16"));
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("TEXT/HTML; charset=utf-16"));

    // This removes the charset part...
    headers.setContentType(QStringLiteral("text/plain"));
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("text/plain"));

    headers.setContentTypeCharset(QStringLiteral("utf-16"));
    headers.setContentTypeCharset(QString());
    QCOMPARE(headers.header(QStringLiteral("content-type")), QStringLiteral("text/plain"));

    headers.setContentType(QStringLiteral("text/plain"));
    QCOMPARE(headers.contentIsText(), true);
    QCOMPARE(headers.contentIsHtml(), false);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), false);

    headers.setContentType(QStringLiteral("text/html"));
    QCOMPARE(headers.contentIsText(), true);
    QCOMPARE(headers.contentIsHtml(), true);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), false);

    headers.setContentType(QStringLiteral("application/xhtml+xml"));
    QCOMPARE(headers.contentIsText(), false);
    QCOMPARE(headers.contentIsHtml(), true);
    QCOMPARE(headers.contentIsXHtml(), true);
    QCOMPARE(headers.contentIsXml(), true);

    headers.setContentType(QStringLiteral("application/xml"));
    QCOMPARE(headers.contentIsText(), false);
    QCOMPARE(headers.contentIsHtml(), false);
    QCOMPARE(headers.contentIsXHtml(), false);
    QCOMPARE(headers.contentIsXml(), true);

    headers.setContentLength(654321);
    QCOMPARE(headers.contentLength(), 654321);

    headers.setContentLength(123456);
    QCOMPARE(headers.contentLength(), 123456);

    headers.setContentEncoding(QStringLiteral("utf-8"));
    QCOMPARE(headers.contentEncoding(), QStringLiteral("utf-8"));

    QDateTime dt = QDateTime::currentDateTime();
    QTime time = dt.time();
    // make sure ms is 0 as we loose this precision
    time.setHMS(time.hour(), time.minute(), time.second());
    dt.setTime(time);

    headers.setDateWithDateTime(dt);
    QCOMPARE(headers.date(), dt);
    QCOMPARE(headers.date(), dt.toUTC());

    headers.setAuthorizationBasic(QStringLiteral("user"), QStringLiteral("pass"));
    QCOMPARE(headers.authorization(), QStringLiteral("Basic dXNlcjpwYXNz"));
    QCOMPARE(headers.authorizationBasic(), QStringLiteral("user:pass"));
    QCOMPARE(headers.authorizationBasic(), QStringLiteral("user:pass"));
    QCOMPARE(headers.authorizationBasicObject().user, QStringLiteral("user"));

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
    headers.setContentType(QStringLiteral(""));
    QCOMPARE(headers.contentType().isEmpty(), true);
    QCOMPARE(headers.contentType().isNull(), false);

    headers.clear();
    headers.setContentDisposition(QStringLiteral(""));
    QCOMPARE(headers.contentDisposition().isEmpty(), true);
    QCOMPARE(headers.contentDisposition().isNull(), false);

    headers.clear();
    headers.setContentDisposition(QStringLiteral("attachment; filename=\"foo.txt\""));
    QCOMPARE(headers.contentDisposition(), QStringLiteral("attachment; filename=\"foo.txt\""));

    headers.clear();
    headers.setContentDispositionAttachment();
    QCOMPARE(headers.contentDisposition(), QStringLiteral("attachment"));

    headers.clear();
    headers.setContentDispositionAttachment(QStringLiteral("foo.txt"));
    QCOMPARE(headers.contentDisposition(), QStringLiteral("attachment; filename=\"foo.txt\""));
}

QTEST_MAIN(TestHeaders)
#include "testheaders.moc"

#endif
