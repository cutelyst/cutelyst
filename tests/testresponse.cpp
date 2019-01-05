#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QTest>
#include <QObject>
#include <QHostInfo>
#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkCookie>
#include <QCryptographicHash>
#include <QUrlQuery>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>

using namespace Cutelyst;

class TestResponse : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestResponse(QObject *parent = nullptr) : CoverageObject(parent) {}

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() {
        doTest();
    }

    void cleanupTestCase();

private:
    TestEngine *m_engine;

    TestEngine* getEngine();

    void doTest();

};

class ResponseTest : public Controller
{
    Q_OBJECT
public:
    explicit ResponseTest(QObject *parent) : Controller(parent) {}

    C_ATTR(status, :Local :AutoArgs)
    void status(Context *c) {
        c->response()->setStatus(c->request()->queryParam(QStringLiteral("data")).toInt());
        c->response()->setBody(QByteArray::number(c->response()->status()));
    }

    C_ATTR(contentEncoding, :Local :AutoArgs)
    void contentEncoding(Context *c) {
        c->response()->setContentEncoding(c->request()->queryParam(QStringLiteral("data")));
        c->response()->setBody(c->response()->contentEncoding());
    }

    C_ATTR(contentLength, :Local :AutoArgs)
    void contentLength(Context *c) {
        c->response()->setBody(c->request()->queryParam(QStringLiteral("data")));
        c->response()->setContentLength(c->response()->body().size());
        c->response()->setBody(QByteArray::number(c->response()->contentLength()));
    }

    C_ATTR(contentType, :Local :AutoArgs)
    void contentType(Context *c) {
        c->response()->setContentType(c->request()->queryParam(QStringLiteral("data")));
        c->response()->setBody(c->response()->contentType());
    }

    C_ATTR(contentTypeCharset, :Local :AutoArgs)
    void contentTypeCharset(Context *c) {
        c->response()->setContentType(c->request()->queryParam(QStringLiteral("data")));
        c->response()->setBody(c->response()->contentTypeCharset());
    }

    C_ATTR(setJsonBody, :Local :AutoArgs)
    void setJsonBody(Context *c) {
        QJsonObject obj;
        auto params = c->request()->queryParameters();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            obj.insert(it.key(), it.value());
            ++it;
        }
        c->response()->setJsonBody(QJsonDocument(obj));
    }

    C_ATTR(largeSetBody, :Local :AutoArgs)
    void largeSetBody(Context *c) {
        c->response()->setBody(QByteArrayLiteral("abcd").repeated(1024 * 1024));
    }

    C_ATTR(largeBody, :Local :AutoArgs)
    void largeBody(Context *c) {
        c->response()->body() = QByteArrayLiteral("abcd").repeated(1024 * 1024);
    }

    C_ATTR(redirect, :Local :AutoArgs)
    void redirect(Context *c) {
        c->response()->redirect(c->request()->queryParam(QStringLiteral("url")));
    }

    C_ATTR(redirectUrl, :Local :AutoArgs)
    void redirectUrl(Context *c) {
        c->response()->redirect(QUrl(c->request()->queryParam(QStringLiteral("url"))));
    }

    C_ATTR(setCookie, :Local :AutoArgs)
    void setCookie(Context *c) {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(QStringLiteral("name")).toLatin1(), params.value(QStringLiteral("value")).toLatin1());
        cookie.setDomain(params.value(QStringLiteral("domain")));
        cookie.setExpirationDate(QDateTime::fromString(params.value(QStringLiteral("expiration_date")), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(QStringLiteral("http_only"))).toBool());
        cookie.setPath(params.value(QStringLiteral("path")));
        cookie.setSecure(QVariant(params.value(QStringLiteral("secure"))).toBool());
        c->response()->setCookie(cookie);
        c->response()->setBody(cookie.toRawForm());
    }

    C_ATTR(setCookies, :Local :AutoArgs)
    void setCookies(Context *c) {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(QStringLiteral("name")).toLatin1(), params.value(QStringLiteral("value")).toLatin1());
        cookie.setDomain(params.value(QStringLiteral("domain")));
        cookie.setExpirationDate(QDateTime::fromString(params.value(QStringLiteral("expiration_date")), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(QStringLiteral("http_only"))).toBool());
        cookie.setPath(params.value(QStringLiteral("path")));
        cookie.setSecure(QVariant(params.value(QStringLiteral("secure"))).toBool());

        ParamsMultiMap bodyParams = c->request()->bodyParameters();
        QNetworkCookie cookie2(bodyParams.value(QStringLiteral("name")).toLatin1(), bodyParams.value(QStringLiteral("value")).toLatin1());
        cookie2.setDomain(bodyParams.value(QStringLiteral("domain")));
        cookie2.setExpirationDate(QDateTime::fromString(bodyParams.value(QStringLiteral("expiration_date")), Qt::ISODate));
        cookie2.setHttpOnly(QVariant(bodyParams.value(QStringLiteral("http_only"))).toBool());
        cookie2.setPath(bodyParams.value(QStringLiteral("path")));
        cookie2.setSecure(QVariant(bodyParams.value(QStringLiteral("secure"))).toBool());

        c->response()->setCookies({ cookie, cookie2 });
        c->response()->setBody(cookie.toRawForm());
    }

    C_ATTR(removeCookies, :Local :AutoArgs)
    void removeCookies(Context *c, const QString &cookieToBeRemoved) {
        ParamsMultiMap params = c->request()->queryParameters();
        QNetworkCookie cookie(params.value(QStringLiteral("name")).toLatin1(), params.value(QStringLiteral("value")).toLatin1());
        cookie.setDomain(params.value(QStringLiteral("domain")));
        cookie.setExpirationDate(QDateTime::fromString(params.value(QStringLiteral("expiration_date")), Qt::ISODate));
        cookie.setHttpOnly(QVariant(params.value(QStringLiteral("http_only"))).toBool());
        cookie.setPath(params.value(QStringLiteral("path")));
        cookie.setSecure(QVariant(params.value(QStringLiteral("secure"))).toBool());

        ParamsMultiMap bodyParams = c->request()->bodyParameters();
        QNetworkCookie cookie2(bodyParams.value(QStringLiteral("name")).toLatin1(), bodyParams.value(QStringLiteral("value")).toLatin1());
        cookie2.setDomain(bodyParams.value(QStringLiteral("domain")));
        cookie2.setExpirationDate(QDateTime::fromString(bodyParams.value(QStringLiteral("expiration_date")), Qt::ISODate));
        cookie2.setHttpOnly(QVariant(bodyParams.value(QStringLiteral("http_only"))).toBool());
        cookie2.setPath(bodyParams.value(QStringLiteral("path")));
        cookie2.setSecure(QVariant(bodyParams.value(QStringLiteral("secure"))).toBool());

        c->response()->setCookies({ cookie, cookie2 });
        c->response()->removeCookies(cookieToBeRemoved.toLatin1());
        c->response()->setBody(cookie.toRawForm());
    }

};

void TestResponse::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestResponse::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
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
    QFETCH(QString, method);
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, responseStatus);
    QFETCH(Headers, responseHeaders);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(method,
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 headers,
                                                 &body);

    QCOMPARE(result.value(QStringLiteral("status")).toByteArray(), responseStatus);
    auto resultHeaders = result.value(QStringLiteral("headers")).value<Headers>();
    if (responseHeaders != resultHeaders) {
        qDebug() << resultHeaders.data() << responseHeaders.data();
        QCOMPARE(resultHeaders, responseHeaders);
    }
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestResponse::testController_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("responseStatus");
    QTest::addColumn<Headers>("responseHeaders");
    QTest::addColumn<QByteArray>("output");

    QString get = QStringLiteral("GET");
    QString post = QStringLiteral("POST");

    QUrlQuery query;
    Headers headers;

    QTest::newRow("status-test00") << get << QStringLiteral("/response/test/status?data=200") << headers << QByteArray()
                                   << QByteArrayLiteral("200 OK")
                                   << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("3")} }
                                   << QByteArrayLiteral("200");

    QTest::newRow("status-test01") << get << QStringLiteral("/response/test/status?data=404") << headers << QByteArray()
                                   << QByteArrayLiteral("404 Not Found")
                                   << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("3")} }
                                   << QByteArrayLiteral("404");

    QTest::newRow("status-test02") << get << QStringLiteral("/response/test/status?data=301") << headers << QByteArray()
                                   << QByteArrayLiteral("301 Moved Permanently")
                                   << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("3")} }
                                   << QByteArrayLiteral("301");

    QTest::newRow("status-test03") << get << QStringLiteral("/response/test/status?data=400") << headers << QByteArray()
                                   << QByteArrayLiteral("400 Bad Request")
                                   << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("3")} }
                                   << QByteArrayLiteral("400");

    QTest::newRow("contentEncoding-test00") << get << QStringLiteral("/response/test/contentEncoding?data=UTF-8") << headers << QByteArray()
                                            << QByteArrayLiteral("200 OK")
                                            << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("5")},{QStringLiteral("Content-Encoding"), QStringLiteral("UTF-8")} }
                                            << QByteArrayLiteral("UTF-8");

    QTest::newRow("contentEncoding-test01") << get << QStringLiteral("/response/test/contentEncoding?data=UTF-16") << headers << QByteArray()
                                            << QByteArrayLiteral("200 OK")
                                            << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("6")},{QStringLiteral("Content-Encoding"), QStringLiteral("UTF-16")} }
                                            << QByteArrayLiteral("UTF-16");

    QTest::newRow("contentLength-test00") << get << QStringLiteral("/response/test/contentLength?data=Hello") << headers << QByteArray()
                                          << QByteArrayLiteral("200 OK")
                                          << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("1")} }
                                          << QByteArrayLiteral("5");

    query.clear();
    query.addQueryItem(QStringLiteral("data"), QStringLiteral("appplication/json"));
    QTest::newRow("contentType-test00") << get << QStringLiteral("/response/test/contentType?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                        << QByteArrayLiteral("200 OK")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("17")}, {QStringLiteral("Content-Type"), QStringLiteral("appplication/json")} }
                                        << QByteArrayLiteral("appplication/json");

    query.clear();
    query.addQueryItem(QStringLiteral("data"), QStringLiteral("TEXT/PLAIN; charset=UTF-8"));
    QTest::newRow("contentType-test01") << get << QStringLiteral("/response/test/contentType?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                        << QByteArrayLiteral("200 OK")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("10")}, {QStringLiteral("Content-Type"), QStringLiteral("TEXT/PLAIN; charset=UTF-8")} }
                                        << QByteArrayLiteral("text/plain");

    query.clear();
    query.addQueryItem(QStringLiteral("data"), QStringLiteral("appplication/json"));
    QTest::newRow("contentTypeCharset-test00") << get << QStringLiteral("/response/test/contentTypeCharset?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                               << QByteArrayLiteral("200 OK")
                                               << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("0")}, {QStringLiteral("Content-Type"), QStringLiteral("appplication/json")} }
                                               << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("data"), QStringLiteral("TEXT/PLAIN; charset=utf-8"));
    QTest::newRow("contentTypeCharset-test01") << get << QStringLiteral("/response/test/contentTypeCharset?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                               << QByteArrayLiteral("200 OK")
                                               << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("5")}, {QStringLiteral("Content-Type"), QStringLiteral("TEXT/PLAIN; charset=utf-8")} }
                                               << QByteArrayLiteral("UTF-8");

    query.clear();
    QTest::newRow("largeBody-test00") << get << QStringLiteral("/response/test/largeBody") << headers << QByteArray()
                                        << QByteArrayLiteral("200 OK")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("4194304")} }
                                        << QByteArrayLiteral("abcd").repeated(1024 * 1024);

    query.clear();
    QTest::newRow("largeSetBody-test01") << get << QStringLiteral("/response/test/largeSetBody") << headers << QByteArray()
                                         << QByteArrayLiteral("200 OK")
                                         << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("4194304")} }
                                         << QByteArrayLiteral("abcd").repeated(1024 * 1024);

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("barz"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("setJsonBody-test00") << get << QStringLiteral("/response/test/setJsonBody?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                        << QByteArrayLiteral("200 OK")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("14")}, {QStringLiteral("Content-Type"), QStringLiteral("application/json")} }
                                        << QByteArrayLiteral("{\"foo\":\"barz\"}");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("x"), QStringLiteral("y"));
    QTest::newRow("setJsonBody-test01") << get << QStringLiteral("/response/test/setJsonBody?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                        << QByteArrayLiteral("200 OK")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("21")}, {QStringLiteral("Content-Type"), QStringLiteral("application/json")} }
                                        << QByteArrayLiteral("{\"foo\":\"bar\",\"x\":\"y\"}");

    query.clear();
    query.addQueryItem(QStringLiteral("url"), QStringLiteral("http://cutelyst.org/foo#something"));
    QTest::newRow("redirect-test00") << get << QStringLiteral("/response/test/redirect?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                     << QByteArrayLiteral("302 Found")
                                     << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("217")}, {QStringLiteral("Content-Type"), QStringLiteral("text/html; charset=utf-8")}, {QStringLiteral("Location"),QStringLiteral("http://cutelyst.org/foo#something")} }
                                     << QByteArrayLiteral("<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    <title>Moved</title>\n  </head>\n  <body>\n     <p>This item has moved <a href=\"http://cutelyst.org/foo#something\">here</a>.</p>\n  </body>\n</html>\n");

    query.clear();
    query.addQueryItem(QStringLiteral("url"), QStringLiteral("http://cutelyst.org/foo#something"));
    QTest::newRow("redirecturl-test0") << get << QStringLiteral("/response/test/redirectUrl?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                        << QByteArrayLiteral("302 Found")
                                        << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("217")}, {QStringLiteral("Content-Type"), QStringLiteral("text/html; charset=utf-8")}, {QStringLiteral("Location"),QStringLiteral("http://cutelyst.org/foo#something")} }
                                        << QByteArrayLiteral("<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    <title>Moved</title>\n  </head>\n  <body>\n     <p>This item has moved <a href=\"http://cutelyst.org/foo#something\">here</a>.</p>\n  </body>\n</html>\n");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    QTest::newRow("setCookie-test00") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("7")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar")} }
                                      << QByteArrayLiteral("foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    QTest::newRow("setCookie-test01") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("28")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; domain=cutelyst.org")} }
                                      << QByteArrayLiteral("foo=bar; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    QTest::newRow("setCookie-test02") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("67")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org")} }
                                      << QByteArrayLiteral("foo=bar; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    QTest::newRow("setCookie-test03") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("77")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org")} }
                                      << QByteArrayLiteral("foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    QTest::newRow("setCookie-test04") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("89")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path")} }
                                      << QByteArrayLiteral("foo=bar; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    query.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    QTest::newRow("setCookie-test05") << get << QStringLiteral("/response/test/setCookie?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray()
                                      << QByteArrayLiteral("200 OK")
                                      << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("97")}, {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path")} }
                                      << QByteArrayLiteral("foo=bar; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    query.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("setCookies-test00") << post << QStringLiteral("/response/test/setCookies?") + query.toString(QUrl::FullyEncoded) << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                       << QByteArrayLiteral("200 OK")
                                       << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("97")}, {QStringLiteral("Set-Cookie"), QStringLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path")} }
                                       << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");


    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    query.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    QUrlQuery cookies;
    cookies.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    cookies.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    cookies.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    cookies.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T11:08:15Z"));
    cookies.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    cookies.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("setCookies-test01") << post << QStringLiteral("/response/test/setCookies?") + query.toString(QUrl::FullyEncoded) << headers << cookies.toString(QUrl::FullyEncoded).toLatin1()
                                       << QByteArrayLiteral("200 OK")
                                       << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("97")},
    {QStringLiteral("Set-Cookie"), QStringLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path")},
    {QStringLiteral("Set-Cookie"), QStringLiteral("foo=bar; secure; expires=Tue, 21-Jun-2016 11:08:15 GMT; domain=cutelyst.org; path=/path")} }
                                       << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");


    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    query.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    cookies.clear();
    cookies.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    cookies.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    cookies.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    cookies.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T11:08:15Z"));
    cookies.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    cookies.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("removeCookies-test00") << post << QStringLiteral("/response/test/removeCookies/foo?") + query.toString(QUrl::FullyEncoded) << headers << cookies.toString(QUrl::FullyEncoded).toLatin1()
                                          << QByteArrayLiteral("200 OK")
                                          << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("97")},
    {QStringLiteral("Set-Cookie"), QStringLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path")} }
                                          << QByteArrayLiteral("bar=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");


    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("value"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    query.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T10:08:15Z"));
    query.addQueryItem(QStringLiteral("http_only"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    query.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    cookies.clear();
    cookies.addQueryItem(QStringLiteral("name"), QStringLiteral("foo"));
    cookies.addQueryItem(QStringLiteral("value"), QStringLiteral("bar"));
    cookies.addQueryItem(QStringLiteral("domain"), QStringLiteral("cutelyst.org"));
    cookies.addQueryItem(QStringLiteral("expiration_date"), QStringLiteral("2016-06-21T11:08:15Z"));
    cookies.addQueryItem(QStringLiteral("path"), QStringLiteral("/path"));
    cookies.addQueryItem(QStringLiteral("secure"), QStringLiteral("true"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("removeCookies-test01") << post << QStringLiteral("/response/test/removeCookies/foo?") + query.toString(QUrl::FullyEncoded) << headers << cookies.toString(QUrl::FullyEncoded).toLatin1()
                                          << QByteArrayLiteral("200 OK")
                                          << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("97")} }
                                          << QByteArrayLiteral("foo=baz; secure; HttpOnly; expires=Tue, 21-Jun-2016 10:08:15 GMT; domain=cutelyst.org; path=/path");

}

QTEST_MAIN(TestResponse)

#include "testresponse.moc"

#endif
