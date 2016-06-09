#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

using namespace Cutelyst;

class TestDispatcher : public CoverageObject
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

void TestDispatcher::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestDispatcher::getEngine()
{
    TestEngine *engine = new TestEngine(QVariantMap(), this);
    if (!engine->initApplication(new TestApplication, true)) {
        return nullptr;
    }
    return engine;
}


void TestDispatcher::cleanupTestCase()
{
    delete m_engine;
}

void TestDispatcher::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QByteArray result = m_engine->createRequest(QStringLiteral("GET"),
                                                urlAux.path(),
                                                urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                Headers(),
                                                nullptr);

    QCOMPARE( result, output );
}

void TestDispatcher::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // UriFor
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test00") << QStringLiteral("/uriFor?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root/"));
    QTest::newRow("urifor-test01") << QStringLiteral("/uriFor?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test02") << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root/"));
    QTest::newRow("urifor-test03") << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/new/path"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("urifor-test04") << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/new/path/a/b/c?encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2&foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/new/path//"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("urifor-test05") << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/new/path/a/b/c?encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2&foo=bar");

    // UriForAction Path
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/root"));
    QTest::newRow("uriforaction-test00") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("uriForAction not found");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/global"));
    QTest::newRow("uriforaction-test01") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/global");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    QTest::newRow("uriforaction-test02") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/many");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test03") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/many?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    QTest::newRow("uriforaction-test04") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test05") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    QTest::newRow("uriforaction-test06") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/one");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test07") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/one?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    QTest::newRow("uriforaction-test08") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test09") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c?foo=bar");

    // UriForAction Chained
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    QTest::newRow("uriforaction-test10") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    QTest::newRow("uriforaction-test11") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test12") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    QTest::newRow("uriforaction-test13") << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/item");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    QTest::newRow("uriforaction-test14") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test15") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1"));
    QTest::newRow("uriforaction-test16") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/a/end/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2"));
    QTest::newRow("uriforaction-test17") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("uriforaction-test17") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/c?encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2&foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2/3")); // too many captures
    QTest::newRow("uriforaction-test18") << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded) << QByteArrayLiteral("uriForAction not found");
}

QTEST_MAIN(TestDispatcher)

#include "testcontext.moc"

#endif
