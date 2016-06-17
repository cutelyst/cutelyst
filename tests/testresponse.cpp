#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>
#include <QHostInfo>
#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

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
    ResponseTest(QObject *parent) : Controller(parent) {}

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
        auto params = c->request()->parameters();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            obj.insert(it.key(), it.value());
            ++it;
        }
        c->response()->setJsonBody(QJsonDocument(obj));
    }

    C_ATTR(redirect, :Local :AutoArgs)
    void redirect(Context *c) {
        c->response()->redirect(c->request()->queryParam(QStringLiteral("url")));
    }

};

void TestResponse::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestResponse::getEngine()
{
    TestEngine *engine = new TestEngine(QVariantMap(), this);
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    new ResponseTest(app);
    if (!engine->initApplication(app, true)) {
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
    QByteArray body;

    Headers responseHeaders;
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

    QTest::newRow("contentLength-test01") << get << QStringLiteral("/response/test/contentLength?data=HelloWithEncoded\xC3\xA7\xE2\x82\xAC\xC2\xA2") << headers << QByteArray()
                                          << QByteArrayLiteral("200 OK")
                                          << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("2")} }
                                          << QByteArrayLiteral("30");

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
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("barz"));
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
                                     << Headers{ {QStringLiteral("Content-Length"), QStringLiteral("308")}, {QStringLiteral("Content-Type"), QStringLiteral("text/html; charset=utf-8")}, {QStringLiteral("Location"),QStringLiteral("http://cutelyst.org/foo#something")} }
                                     << QByteArrayLiteral("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    <title>Moved</title>\n  </head>\n  <body>\n     <p>This item has moved <a href=http://cutelyst.org/foo#something>here</a>.</p>\n  </body>\n</html>\n");

}

QTEST_MAIN(TestResponse)

#include "testresponse.moc"

#endif
