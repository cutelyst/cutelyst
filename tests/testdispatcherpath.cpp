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

class TestDispatcherPath : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestDispatcherPath(QObject *parent = nullptr)
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

void TestDispatcherPath::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestDispatcherPath::getEngine()
{
    auto app    = new TestApplication;
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

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.body, output);
}

void TestDispatcherPath::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // Path dispatcher
    QTest::newRow("path-test01") << u"/test/controller"_s
                                 << QByteArrayLiteral("path /test/controller args ");
    QTest::newRow("path-test02") << u"/test/controller/hello"_s
                                 << QByteArrayLiteral("path /test/controller/hello args ");
    QTest::newRow("path-test03") << u"/global"_s << QByteArrayLiteral("path /global args ");
    QTest::newRow("path-test04") << u"/test/controller/many/"_s
                                 << QByteArrayLiteral("path /test/controller/many/ args ");
    QTest::newRow("path-test05") << u"/test/controller/many"_s
                                 << QByteArrayLiteral("path /test/controller/many args ");
    QTest::newRow("path-test06")
        << u"/test/controller/many/1/2/3/4/5/6/7/8/9/10/11/12"_s
        << QByteArrayLiteral("path /test/controller/many/1/2/3/4/5/6/7/8/9/10/11/12 args "
                             "1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test07") << u"/test/controller/one/1"_s
                                 << QByteArrayLiteral("path /test/controller/one/1 args 1");
    QTest::newRow("path-test09") << u"/test/controller/two/1/2"_s
                                 << QByteArrayLiteral("path /test/controller/two/1/2 args 1/2");
    QTest::newRow("path-test12") << u"/test/controller/manyOld"_s
                                 << QByteArrayLiteral("path /test/controller/manyOld args ");
    QTest::newRow("path-test13")
        << u"/test/controller/manyOld/1/2/3/4/5/6/7/8/9/10/11/12"_s
        << QByteArrayLiteral("path /test/controller/manyOld/1/2/3/4/5/6/7/8/9/10/11/12 args "
                             "1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test14") << u"/test/controller/manyOldWithNoArgs/"_s
                                 << QByteArrayLiteral(
                                        "path /test/controller/manyOldWithNoArgs/ args ");
    QTest::newRow("path-test15") << u"/test/controller/manyOldWithNoArgs"_s
                                 << QByteArrayLiteral(
                                        "path /test/controller/manyOldWithNoArgs args ");
    QTest::newRow("path-test16")
        << u"/test/controller/manyOldWithNoArgs/1/2/3/4/5/6/7/8/9/10/11/12"_s
        << QByteArrayLiteral("path /test/controller/manyOldWithNoArgs/1/2/3/4/5/6/7/8/9/10/11/12 "
                             "args 1/2/3/4/5/6/7/8/9/10/11/12");
    QTest::newRow("path-test17") << u"/test/controller/oneOld/1"_s
                                 << QByteArrayLiteral("path /test/controller/oneOld/1 args 1");
    QTest::newRow("path-test19") << u"/test/controller/twoOld/1/2"_s
                                 << QByteArrayLiteral("path /test/controller/twoOld/1/2 args 1/2");
    QTest::newRow("path-test21") << u"/"_s << QByteArrayLiteral("rootAction");

    // Test if we break chain with auto returning false
    QTest::newRow("path-autoFalse00") << u"/global?autoFalse=1"_s << QByteArrayLiteral("autoFalse");

    QTest::newRow("path-not-found00")
        << u"/foo_bar_not_found"_s << QByteArrayLiteral("404 - Not Found.");
}

QTEST_MAIN(TestDispatcherPath)

#include "testdispatcherpath.moc"

#endif
