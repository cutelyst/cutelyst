#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>
#include <QHostInfo>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

using namespace Cutelyst;

class TestRequest : public CoverageObject
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

class RequestTest : public Controller
{
    Q_OBJECT
public:
    RequestTest(QObject *parent) : Controller(parent) {}

    C_ATTR(address, :Local :AutoArgs)
    void address(Context *c) {
        c->response()->setBody(c->request()->address().toString());
    }

    C_ATTR(hostname, :Local :AutoArgs)
    void hostname(Context *c) {
        c->response()->setBody(c->request()->hostname());
    }

    C_ATTR(port, :Local :AutoArgs)
    void port(Context *c) {
        c->response()->setBody(QByteArray::number(c->request()->port()));
    }

    C_ATTR(uri, :Local :AutoArgs)
    void uri(Context *c) {
        c->response()->setBody(c->request()->uri().toString());
    }

    C_ATTR(base, :Local :AutoArgs)
    void base(Context *c) {
        c->response()->setBody(c->request()->base());
    }

    C_ATTR(path, :Local :AutoArgs)
    void path(Context *c) {
        c->response()->setBody(c->request()->path());
    }

    C_ATTR(match, :Local :AutoArgs)
    void match(Context *c) {
        c->response()->setBody(c->request()->match());
    }

    C_ATTR(method, :Local :AutoArgs)
    void method(Context *c) {
        c->response()->setBody(c->request()->method());
    }

    C_ATTR(isPost, :Local :AutoArgs)
    void isPost(Context *c) {
        c->response()->setBody(QVariant(c->request()->isPost()).toString());
    }

    C_ATTR(isGet, :Local :AutoArgs)
    void isGet(Context *c) {
        c->response()->setBody(QVariant(c->request()->isGet()).toString());
    }

    C_ATTR(protocol, :Local :AutoArgs)
    void protocol(Context *c) {
        c->response()->setBody(c->request()->protocol());
    }

    C_ATTR(remoteUser, :Local :AutoArgs)
    void remoteUser(Context *c) {
        c->response()->setBody(c->request()->remoteUser());
    }
};

void TestRequest::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestRequest::getEngine()
{
    TestEngine *engine = new TestEngine(QVariantMap(), this);
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    new RequestTest(app);
    if (!engine->initApplication(app, true)) {
        return nullptr;
    }
    return engine;
}


void TestRequest::cleanupTestCase()
{
    delete m_engine;
}

void TestRequest::doTest()
{
    QFETCH(QString, method);
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QByteArray result = m_engine->createRequest(method,
                                                urlAux.path(),
                                                urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                headers,
                                                nullptr);

    QCOMPARE( result, output );
}

void TestRequest::testController_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("output");

    QString get = QStringLiteral("GET");
    QString post = QStringLiteral("POST");

//    QUrlQuery query;
    Headers headers;
    QTest::newRow("request-test00") << get << QStringLiteral("/request/test/address") << headers << QByteArrayLiteral("127.0.0.1");
    QTest::newRow("request-test01") << get << QStringLiteral("/request/test/hostname") << headers
                                    << QHostInfo::fromName(QStringLiteral("127.0.0.1")).hostName().toLatin1();
    QTest::newRow("request-test02") << get << QStringLiteral("/request/test/port") << headers << QByteArrayLiteral("3000");
    QTest::newRow("request-test03") << get << QStringLiteral("/request/test/uri") << headers << QByteArrayLiteral("http://127.0.0.1/request/test/uri");
    QTest::newRow("request-test04") << get << QStringLiteral("/request/test/base") << headers << QByteArrayLiteral("http://127.0.0.1/");
    QTest::newRow("request-test05") << get << QStringLiteral("/request/test/path") << headers << QByteArrayLiteral("request/test/path");
    QTest::newRow("request-test06") << get << QStringLiteral("/request/test/match") << headers << QByteArrayLiteral("request/test/match");

    QTest::newRow("request-test07") << get << QStringLiteral("/request/test/method") << headers << QByteArrayLiteral("GET");
    QTest::newRow("request-test08") << post << QStringLiteral("/request/test/method") << headers << QByteArrayLiteral("POST");
    QTest::newRow("request-test09") << QStringLiteral("HEAD") << QStringLiteral("/request/test/method") << headers << QByteArrayLiteral("HEAD");

    QTest::newRow("request-test10") << get << QStringLiteral("/request/test/isPost") << headers << QByteArrayLiteral("false");
    QTest::newRow("request-test11") << QStringLiteral("PoSt") << QStringLiteral("/request/test/isPost") << headers << QByteArrayLiteral("false");
    QTest::newRow("request-test12") << post << QStringLiteral("/request/test/isPost") << headers << QByteArrayLiteral("true");

    QTest::newRow("request-test13") << post << QStringLiteral("/request/test/isGet") << headers << QByteArrayLiteral("false");
    QTest::newRow("request-test14") << QStringLiteral("GeT") << QStringLiteral("/request/test/isGet") << headers << QByteArrayLiteral("false");
    QTest::newRow("request-test15") << get << QStringLiteral("/request/test/isGet") << headers << QByteArrayLiteral("true");

    QTest::newRow("request-test16") << get << QStringLiteral("/request/test/protocol") << headers << QByteArrayLiteral("HTTP/1.1");
    QTest::newRow("request-test17") << get << QStringLiteral("/request/test/remoteUser") << headers << QByteArrayLiteral("");


}

QTEST_MAIN(TestRequest)

#include "testrequest.moc"

#endif
