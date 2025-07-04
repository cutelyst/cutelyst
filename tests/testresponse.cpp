#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>

#include <QCryptographicHash>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QObject>
#include <QTest>
#include <QUrlQuery>
#include <QUuid>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class TestResponse : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestResponse(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void cleanupTestCase();

private:
    TestEngine *m_engine = nullptr;

    TestEngine *getEngine();

    void doTest();
};

class ResponseTest : public Controller
{
    Q_OBJECT
public:
    explicit ResponseTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(status, :Local :AutoArgs)
    void status(Context *c)
    {
        c->response()->setStatus(c->request()->queryParam(u"data"_s).toInt());
        c->response()->setBody(QByteArray::number(c->response()->status()));
    }

    C_ATTR(contentEncoding, :Local :AutoArgs)
    void contentEncoding(Context *c)
    {
        c->response()->setContentEncoding(c->request()->queryParam(u"data"_s).toLatin1());
        c->response()->setBody(c->response()->contentEncoding());
    }

    C_ATTR(contentLength, :Local :AutoArgs)
    void contentLength(Context *c)
    {
        auto buffer = new QBuffer;
        buffer->open(QBuffer::ReadWrite);
        buffer->write("something_to_test_for_updated_content_type");
        c->response()->setBody(buffer);

        c->response()->setBody(c->request()->queryParam(u"data"_s));
    }

    C_ATTR(contentLengthIODevice, :Local :AutoArgs)
    void contentLengthIODevice(Context *c)
    {
        c->response()->setBody("something_to_test_for_updated_content_type"_ba);

        auto buffer = new QBuffer;
        buffer->open(QBuffer::ReadWrite);
        buffer->write(c->request()->queryParam(u"data"_s).toLatin1());
        c->response()->setBody(buffer);
    }

    C_ATTR(contentType, :Local :AutoArgs)
    void contentType(Context *c)
    {
        c->response()->setContentType(c->request()->queryParam(u"data"_s).toLatin1());
        c->response()->setBody(c->response()->contentType());
    }

    C_ATTR(contentTypeCharset, :Local :AutoArgs)
    void contentTypeCharset(Context *c)
    {
        c->response()->setContentType(c->request()->queryParam(u"data"_s).toLatin1());
        c->response()->setBody(c->response()->contentTypeCharset());
    }

    C_ATTR(setJsonBody, :Local :AutoArgs)
    void setJsonBody(Context *c)
    {
        QJsonObject obj;
        const auto params = c->request()->queryParameters();
        for (const auto &[key, value] : params.asKeyValueRange()) {
            obj.insert(key, value);
        }
        c->response()->setJsonObjectBody(obj);
    }

    C_ATTR(largeSetBody, :Local :AutoArgs)
    void largeSetBody(Context *c)
    {
        c->response()->setBody(QByteArrayLiteral("abcd").repeated(1024 * 1024));
    }

    C_ATTR(largeBody, :Local :AutoArgs)
    void largeBody(Context *c)
    {
        c->response()->body() = QByteArrayLiteral("abcd").repeated(1024 * 1024);
    }

    C_ATTR(redirect, :Local :AutoArgs)
    void redirect(Context *c) { c->response()->redirect(c->request()->queryParam(u"url"_s)); }

    C_ATTR(redirectUrl, :Local :AutoArgs)
    void redirectUrl(Context *c)
    {
        c->response()->redirect(QUrl(c->request()->queryParam(u"url"_s)));
    }

    C_ATTR(setCookie, :Local :AutoArgs)
    void setCookie(Context *c)
    {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(u"name"_s).toLatin1(),
                              params.value(u"value"_s).toLatin1());
        cookie.setDomain(params.value(u"domain"_s));
        cookie.setExpirationDate(
            QDateTime::fromString(params.value(u"expiration_date"_s), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(u"http_only"_s)).toBool());
        cookie.setPath(params.value(u"path"_s));
        cookie.setSecure(QVariant(params.value(u"secure"_s)).toBool());
        c->response()->setCookie(cookie);
        c->response()->setBody(cookie.toRawForm());
    }

    C_ATTR(setCookies, :Local :AutoArgs)
    void setCookies(Context *c)
    {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(u"name"_s).toLatin1(),
                              params.value(u"value"_s).toLatin1());
        cookie.setDomain(params.value(u"domain"_s));
        cookie.setExpirationDate(
            QDateTime::fromString(params.value(u"expiration_date"_s), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(u"http_only"_s)).toBool());
        cookie.setPath(params.value(u"path"_s));
        cookie.setSecure(QVariant(params.value(u"secure"_s)).toBool());

        ParamsMultiMap bodyParams = c->request()->bodyParameters();
        QNetworkCookie cookie2(bodyParams.value(u"name"_s).toLatin1(),
                               bodyParams.value(u"value"_s).toLatin1());
        cookie2.setDomain(bodyParams.value(u"domain"_s));
        cookie2.setExpirationDate(
            QDateTime::fromString(bodyParams.value(u"expiration_date"_s), Qt::ISODate));
        cookie2.setHttpOnly(QVariant(bodyParams.value(u"http_only"_s)).toBool());
        cookie2.setPath(bodyParams.value(u"path"_s));
        cookie2.setSecure(QVariant(bodyParams.value(u"secure"_s)).toBool());

        c->response()->setCookies({cookie, cookie2});
        c->response()->setBody(cookie.toRawForm());
    }

    C_ATTR(removeCookies, :Local :AutoArgs)
    void removeCookies(Context *c, const QString &cookieToBeRemoved)
    {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(u"name"_s).toLatin1(),
                              params.value(u"value"_s).toLatin1());
        cookie.setDomain(params.value(u"domain"_s));
        cookie.setExpirationDate(
            QDateTime::fromString(params.value(u"expiration_date"_s), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(u"http_only"_s)).toBool());
        cookie.setPath(params.value(u"path"_s));
        cookie.setSecure(QVariant(params.value(u"secure"_s)).toBool());

        ParamsMultiMap bodyParams = c->request()->bodyParameters();
        QNetworkCookie cookie2(bodyParams.value(u"name"_s).toLatin1(),
                               bodyParams.value(u"value"_s).toLatin1());
        cookie2.setDomain(bodyParams.value(u"domain"_s));
        cookie2.setExpirationDate(
            QDateTime::fromString(bodyParams.value(u"expiration_date"_s), Qt::ISODate));
        cookie2.setHttpOnly(QVariant(bodyParams.value(u"http_only"_s)).toBool());
        cookie2.setPath(bodyParams.value(u"path"_s));
        cookie2.setSecure(QVariant(bodyParams.value(u"secure"_s)).toBool());

        c->response()->setCookies({cookie, cookie2});
        c->response()->removeCookies(cookieToBeRemoved.toLatin1());
        c->response()->setBody(cookie.toRawForm());
    }

    C_ATTR(sendJson, :Local :AutoArgs)
    void sendJson(Context *c)
    {
        c->response()->setJsonBody("{}"_ba);

        c->response()->setJsonBody(u"{}"_s);

        QJsonObject obj;
        c->response()->setJsonObjectBody(obj);

        QJsonArray array;
        c->response()->setJsonArrayBody(array);
    }
};

void TestResponse::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestResponse::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ResponseTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestResponse::cleanupTestCase()
{
    delete m_engine;
}

void TestResponse::doTest()
{
    QFETCH(QByteArray, method);
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(int, responseStatus);
    QFETCH(Headers, responseHeaders);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        method, urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), headers, &body);

    QCOMPARE(result.statusCode, responseStatus);
    auto resultHeaders = result.headers;
    if (responseHeaders != resultHeaders) {
        qDebug() << resultHeaders << responseHeaders;
        QCOMPARE(resultHeaders, responseHeaders);
    }
    QCOMPARE(result.body, output);
}

void TestResponse::testController_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<int>("responseStatus");
    QTest::addColumn<Headers>("responseHeaders");
    QTest::addColumn<QByteArray>("output");

    const auto get  = "GET"_ba;
    const auto post = "POST"_ba;

    QUrlQuery query;
    Headers headers;

    QTest::newRow("status-test00")
        << get << u"/response/test/status?data=200"_s << headers << QByteArray() << 200
        << Headers{{"Content-Length", "3"}} << QByteArrayLiteral("200");

    QTest::newRow("status-test01")
        << get << u"/response/test/status?data=404"_s << headers << QByteArray() << 404
        << Headers{{"Content-Length", "3"}} << QByteArrayLiteral("404");

    QTest::newRow("status-test02")
        << get << u"/response/test/status?data=301"_s << headers << QByteArray() << 301
        << Headers{{"Content-Length", "3"}} << QByteArrayLiteral("301");

    QTest::newRow("status-test03")
        << get << u"/response/test/status?data=400"_s << headers << QByteArray() << 400
        << Headers{{"Content-Length", "3"}} << QByteArrayLiteral("400");

    QTest::newRow("contentEncoding-test00")
        << get << u"/response/test/contentEncoding?data=UTF-8"_s << headers << QByteArray() << 200
        << Headers{{"Content-Length", "5"}, {"Content-Encoding", "UTF-8"}}
        << QByteArrayLiteral("UTF-8");

    QTest::newRow("contentEncoding-test01")
        << get << u"/response/test/contentEncoding?data=UTF-16"_s << headers << QByteArray() << 200
        << Headers{{"Content-Length", "6"}, {"Content-Encoding", "UTF-16"}}
        << QByteArrayLiteral("UTF-16");

    QTest::newRow("contentLength-test00")
        << get << u"/response/test/contentLength?data=Hello"_s << headers << QByteArray() << 200
        << Headers{{"Content-Length", "5"}} << QByteArrayLiteral("Hello");

    QTest::newRow("contentLengthIODevice-test00")
        << get << u"/response/test/contentLengthIODevice?data=HelloWorld"_s << headers
        << QByteArray() << 200 << Headers{{"Content-Length", "10"}}
        << QByteArrayLiteral("HelloWorld");

    query.clear();
    query.addQueryItem(u"data"_s, u"appplication/json"_s);
    QTest::newRow("contentType-test00")
        << get << u"/response/test/contentType?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "17"}, {"Content-Type", "appplication/json"}}
        << QByteArrayLiteral("appplication/json");

    query.clear();
    query.addQueryItem(u"data"_s, u"TEXT/PLAIN; charset=UTF-8"_s);
    QTest::newRow("contentType-test01")
        << get << u"/response/test/contentType?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "10"}, {"Content-Type", "TEXT/PLAIN; charset=UTF-8"}}
        << QByteArrayLiteral("text/plain");

    query.clear();
    query.addQueryItem(u"data"_s, u"appplication/json"_s);
    QTest::newRow("contentTypeCharset-test00")
        << get
        << u"/response/test/contentTypeCharset?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 200
        << Headers{{"Content-Length", "0"},
                   {"Content-Type", "appplication/json"},}
        << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(u"data"_s, u"TEXT/PLAIN; charset=utf-8"_s);
    QTest::newRow("contentTypeCharset-test01")
        << get
        << u"/response/test/contentTypeCharset?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 200
        << Headers{{"Content-Length", "5"},
                   {"Content-Type", "TEXT/PLAIN; charset=utf-8"},}
        << QByteArrayLiteral("UTF-8");

    query.clear();
    QTest::newRow("largeBody-test00")
        << get << u"/response/test/largeBody"_s << headers << QByteArray()
        << 200
        << Headers{{"Content-Length", "4194304"},}
        << QByteArrayLiteral("abcd").repeated(1024 * 1024);

    query.clear();
    QTest::newRow("largeSetBody-test01")
        << get << u"/response/test/largeSetBody"_s << headers << QByteArray()
        << 200
        << Headers{{"Content-Length", "4194304"},}
        << QByteArrayLiteral("abcd").repeated(1024 * 1024);

    query.clear();
    query.addQueryItem(u"foo"_s, u"barz"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("setJsonBody-test00")
        << get << u"/response/test/setJsonBody?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 200
        << Headers{{"Content-Length", "14"},
                   {"Content-Type", "application/json"},}
        << QByteArrayLiteral("{\"foo\":\"barz\"}");

    query.clear();
    query.addQueryItem(u"foo"_s, u"bar"_s);
    query.addQueryItem(u"x"_s, u"y"_s);
    QTest::newRow("setJsonBody-test01")
        << get << u"/response/test/setJsonBody?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 200
        << Headers{{"Content-Length", "21"},
                   {"Content-Type", "application/json"},}
        << QByteArrayLiteral("{\"foo\":\"bar\",\"x\":\"y\"}");

    query.clear();
    query.addQueryItem(u"url"_s, u"http://cutelyst.org/foo#something"_s);
    QTest::newRow("redirect-test00")
        << get << u"/response/test/redirect?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 302
        << Headers{{"Content-Length", "217"},
                   {"Content-Type", "text/html; charset=utf-8"},
                   {"Location",
                    "http://cutelyst.org/foo#something"},}
        << QByteArrayLiteral(
               "<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    "
               "<title>Moved</title>\n  </head>\n  <body>\n     <p>This item has moved <a "
               "href=\"http://cutelyst.org/foo#something\">here</a>.</p>\n  </body>\n</html>\n");

    query.clear();
    query.addQueryItem(u"url"_s, u"http://cutelyst.org/foo#something"_s);
    QTest::newRow("redirecturl-test0")
        << get << u"/response/test/redirectUrl?"_s + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << 302
        << Headers{{"Content-Length", "217"},
                   {"Content-Type", "text/html; charset=utf-8"},
                   {"Location",
                    "http://cutelyst.org/foo#something"},}
        << QByteArrayLiteral(
               "<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    "
               "<title>Moved</title>\n  </head>\n  <body>\n     <p>This item has moved <a "
               "href=\"http://cutelyst.org/foo#something\">here</a>.</p>\n  </body>\n</html>\n");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    QTest::newRow("setCookie-test00")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200 << Headers{{"Content-Length", "7"}, {"Set-Cookie", "foo=bar"}}
        << QByteArrayLiteral("foo=bar");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    QTest::newRow("setCookie-test01")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "28"}, {"Set-Cookie", "foo=bar; domain=cutelyst.org"}}
        << QByteArrayLiteral("foo=bar; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    QTest::newRow("setCookie-test02")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "67"},
                   {"Set-Cookie",

                    "foo=bar; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org"}}
        << QByteArrayLiteral("foo=bar; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    QTest::newRow("setCookie-test03")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "77"},
                   {"Set-Cookie",
                    "foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                    "domain=cutelyst.org"}}
        << QByteArrayLiteral(
               "foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    QTest::newRow("setCookie-test04")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "89"},
                   {"Set-Cookie",
                    "foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                    "domain=cutelyst.org; path=/path"}}
        << QByteArrayLiteral("foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"bar"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    query.addQueryItem(u"secure"_s, u"true"_s);
    QTest::newRow("setCookie-test05")
        << get << u"/response/test/setCookie?"_s + query.toString(QUrl::FullyEncoded) << headers
        << QByteArray() << 200
        << Headers{{"Content-Length", "97"},
                   {"Set-Cookie",
                    "foo=bar; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 "
                    "GMT; domain=cutelyst.org; path=/path"}}
        << QByteArrayLiteral("foo=bar; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(u"name"_s, u"bar"_s);
    query.addQueryItem(u"value"_s, u"baz"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    query.addQueryItem(u"secure"_s, u"true"_s);
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("setCookies-test00")
        << post << u"/response/test/setCookies?"_s + query.toString(QUrl::FullyEncoded) << headers
        << query.toString(QUrl::FullyEncoded).toLatin1() << 200
        << Headers{{"Content-Length", "97"},
                   {"Set-Cookie",
                    "bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 "
                    "GMT; domain=cutelyst.org; path=/path"}}
        << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(u"name"_s, u"bar"_s);
    query.addQueryItem(u"value"_s, u"baz"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    query.addQueryItem(u"secure"_s, u"true"_s);
    QUrlQuery cookies;
    cookies.addQueryItem(u"name"_s, u"foo"_s);
    cookies.addQueryItem(u"value"_s, u"bar"_s);
    cookies.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    cookies.addQueryItem(u"expiration_date"_s, u"2016-06-21T11:08:15Z"_s);
    cookies.addQueryItem(u"path"_s, u"/path"_s);
    cookies.addQueryItem(u"secure"_s, u"true"_s);
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("setCookies-test01")
        << post << u"/response/test/setCookies?"_s + query.toString(QUrl::FullyEncoded)
        << headers << cookies.toString(QUrl::FullyEncoded).toLatin1() << 200
        << Headers{{"Content-Length", "97"},
                   {"Set-Cookie",
                    "bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 "
                                   "GMT; domain=cutelyst.org; path=/path"},
                   {"Set-Cookie",
                    "foo=bar; secure; expires=Tue, 21-Jun-2016 11:08:15 GMT; "
                    "domain=cutelyst.org; path=/path"},}
        << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(u"name"_s, u"bar"_s);
    query.addQueryItem(u"value"_s, u"baz"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    query.addQueryItem(u"secure"_s, u"true"_s);
    cookies.clear();
    cookies.addQueryItem(u"name"_s, u"foo"_s);
    cookies.addQueryItem(u"value"_s, u"bar"_s);
    cookies.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    cookies.addQueryItem(u"expiration_date"_s, u"2016-06-21T11:08:15Z"_s);
    cookies.addQueryItem(u"path"_s, u"/path"_s);
    cookies.addQueryItem(u"secure"_s, u"true"_s);
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("removeCookies-test00")
        << post << u"/response/test/removeCookies/foo?"_s + query.toString(QUrl::FullyEncoded)
        << headers << cookies.toString(QUrl::FullyEncoded).toLatin1() << 200
        << Headers{{"Content-Length", "97"},
                   {"Set-Cookie",
                    "bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 "
                    "GMT; domain=cutelyst.org; path=/path"}}
        << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(u"name"_s, u"foo"_s);
    query.addQueryItem(u"value"_s, u"baz"_s);
    query.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    query.addQueryItem(u"expiration_date"_s, u"2016-06-21T10:08:15Z"_s);
    query.addQueryItem(u"http_only"_s, u"true"_s);
    query.addQueryItem(u"path"_s, u"/path"_s);
    query.addQueryItem(u"secure"_s, u"true"_s);
    cookies.clear();
    cookies.addQueryItem(u"name"_s, u"foo"_s);
    cookies.addQueryItem(u"value"_s, u"bar"_s);
    cookies.addQueryItem(u"domain"_s, u"cutelyst.org"_s);
    cookies.addQueryItem(u"expiration_date"_s, u"2016-06-21T11:08:15Z"_s);
    cookies.addQueryItem(u"path"_s, u"/path"_s);
    cookies.addQueryItem(u"secure"_s, u"true"_s);
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("removeCookies-test01")
        << post << u"/response/test/removeCookies/foo?"_s + query.toString(QUrl::FullyEncoded)
        << headers << cookies.toString(QUrl::FullyEncoded).toLatin1() << 200
        << Headers{{"Content-Length", "97"}}
        << QByteArrayLiteral("foo=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; "
                             "domain=cutelyst.org; path=/path");
}

QTEST_MAIN(TestResponse)

#include "testresponse.moc"

#endif
