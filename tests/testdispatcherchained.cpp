#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;

class TestDispatcherChained : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestDispatcherChained(QObject *parent = nullptr)
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

void TestDispatcherChained::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestDispatcherChained::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestDispatcherChained::cleanupTestCase()
{
    delete m_engine;
}

void TestDispatcherChained::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.body, output);
}

void TestDispatcherChained::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // Chained dispatcher
    QTest::newRow("chained-test00") << u"/root"_s << QByteArrayLiteral("/root");
    QTest::newRow("chained-test01") << u"/root/"_s << QByteArrayLiteral("/root");
    QTest::newRow("chained-test02") << u"/root////"_s << QByteArrayLiteral("/root");
    QTest::newRow("chained-test03") << u"/root/item"_s << QByteArrayLiteral("/root");
    QTest::newRow("chained-test04") << u"/root/item/"_s << QByteArrayLiteral("/root");

    QTest::newRow("chained-test05") << u"/chain/item"_s << QByteArrayLiteral("/chain/item[MANY]/");
    QTest::newRow("chained-test07")
        << u"/chain/item/foo"_s << QByteArrayLiteral("/chain/item[ONE]/foo");
    QTest::newRow("chained-test08")
        << u"/chain/item/foo/bar"_s << QByteArrayLiteral("/chain/item[MANY]/foo/bar");

    QTest::newRow("chained-test09")
        << u"/chain/midle/one/two/end"_s << QByteArrayLiteral("/chain/midle/one/two/end");
    QTest::newRow("chained-test10")
        << u"/chain/midle/TWO/ONE/end"_s << QByteArrayLiteral("/chain/midle/TWO/ONE/end");
    QTest::newRow("chained-test12") << u"/chain/midle/TWO/ONE/end/1/2/3/4/5"_s
                                    << QByteArrayLiteral("/chain/midle/TWO/ONE/end/1/2/3/4/5");
}

QTEST_MAIN(TestDispatcherChained)

#include "testdispatcherchained.moc"

#endif
