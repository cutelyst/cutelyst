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

class TestDispatcherPath : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestDispatcherPath(QObject *parent = nullptr) : CoverageObject(parent) {}

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

void TestDispatcherPath::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestDispatcherPath::getEngine()
{
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}


void TestDispatcherPath::cleanupTestCase()
{
    delete m_engine;
}

void TestDispatcherPath::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(QStringLiteral("GET"),
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 Headers(),
                                                 nullptr);

    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestDispatcherPath::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // Path dispatcher
    QTest::newRow("path-test00") << QStringLiteral("/test/unknown_resource") << QByteArrayLiteral("Unknown resource 'test/unknown_resource'.");
    QTest::newRow("path-test01") << QStringLiteral("/test/controller") << QByteArrayLiteral("path /test/controller args ");
    QTest::newRow("path-test02") << QStringLiteral("/test/controller/hello") << QByteArrayLiteral("path /test/controller/hello args ");
    QTest::newRow("path-test03") << QStringLiteral("/global") << QByteArrayLiteral("path /global args ");
    QTest::newRow("path-test04") << QStringLiteral("/test/controller/many/") << QByteArrayLiteral("path /test/controller/many/ args ");
    QTest::newRow("path-test05") << QStringLiteral("/test/controller/many") << QByteArrayLiteral("path /test/controller/many args ");
    QTest::newRow("path-test06") << QStringLiteral("/test/controller/many/1/2/3/4/5/6/7/8/9/10/11/12") << QByteArrayLiteral("path /test/controller/many/1/2/3/4/5/6/7/8/9/10/11/12 args 1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test07") << QStringLiteral("/test/controller/one/1") << QByteArrayLiteral("path /test/controller/one/1 args 1");
    QTest::newRow("path-test08") << QStringLiteral("/test/controller/one/1//") << QByteArrayLiteral("path /test/controller/one/1// args 1");
    QTest::newRow("path-test09") << QStringLiteral("/test/controller/two/1/2") << QByteArrayLiteral("path /test/controller/two/1/2 args 1/2");
    QTest::newRow("path-test10") << QStringLiteral("/test/controller/two/1/2//") << QByteArrayLiteral("path /test/controller/two/1/2// args 1/2");
    QTest::newRow("path-test11") << QStringLiteral("/test/controller/manyOld/") << QByteArrayLiteral("path /test/controller/manyOld/ args ");
    QTest::newRow("path-test12") << QStringLiteral("/test/controller/manyOld") << QByteArrayLiteral("path /test/controller/manyOld args ");
    QTest::newRow("path-test13") << QStringLiteral("/test/controller/manyOld/1/2/3/4/5/6/7/8/9/10/11/12") << QByteArrayLiteral("path /test/controller/manyOld/1/2/3/4/5/6/7/8/9/10/11/12 args 1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test14") << QStringLiteral("/test/controller/manyOldWithNoArgs/") << QByteArrayLiteral("path /test/controller/manyOldWithNoArgs/ args ");
    QTest::newRow("path-test15") << QStringLiteral("/test/controller/manyOldWithNoArgs") << QByteArrayLiteral("path /test/controller/manyOldWithNoArgs args ");
    QTest::newRow("path-test16") << QStringLiteral("/test/controller/manyOldWithNoArgs/1/2/3/4/5/6/7/8/9/10/11/12") << QByteArrayLiteral("path /test/controller/manyOldWithNoArgs/1/2/3/4/5/6/7/8/9/10/11/12 args 1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test17") << QStringLiteral("/test/controller/oneOld/1") << QByteArrayLiteral("path /test/controller/oneOld/1 args 1");
    QTest::newRow("path-test18") << QStringLiteral("/test/controller/oneOld/1//") << QByteArrayLiteral("path /test/controller/oneOld/1// args 1");
    QTest::newRow("path-test19") << QStringLiteral("/test/controller/twoOld/1/2") << QByteArrayLiteral("path /test/controller/twoOld/1/2 args 1/2");
    QTest::newRow("path-test20") << QStringLiteral("/test/controller/twoOld/1/2//") << QByteArrayLiteral("path /test/controller/twoOld/1/2// args 1/2");
    QTest::newRow("path-test21") << QStringLiteral("/") << QByteArrayLiteral("rootAction");
}

QTEST_MAIN(TestDispatcherPath)

#include "testdispatcherpath.moc"

#endif
