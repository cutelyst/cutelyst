#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>

#include <QDir>
#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class ActionRESTController : public Controller
{
    Q_OBJECT
    C_NAMESPACE("/action/rest")
public:
    explicit ActionRESTController(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(test1, :Local :ActionClass(REST))
    void test1(Context *c) { c->response()->body() += QByteArrayLiteral("test1."); }

    C_ATTR(test1_GET, :Private)
    void test1_GET(Context *c) { c->response()->body() += QByteArrayLiteral("test1 GET."); }

    C_ATTR(test1_HEAD, :Private)
    void test1_HEAD(Context *c) { c->response()->body() += QByteArrayLiteral("test1 HEAD."); }

    C_ATTR(test1_not_implemented, :Private)
    void test1_not_implemented(Context *c)
    {
        c->response()->body() += QByteArrayLiteral("test1 NOT IMPLEMENTED.");
    }

    C_ATTR(test2, :Local :ActionClass(REST))
    void test2(Context *c) { c->response()->body() += QByteArrayLiteral("test2."); }

    C_ATTR(test2_GET, :Private)
    void test2_GET(Context *c) { c->response()->body() += QByteArrayLiteral("test2 GET."); }

    C_ATTR(test2_DELETE, :Private)
    void test2_DELETE(Context *c) { c->response()->body() += QByteArrayLiteral("test2 DELETE."); }
};

class TestActionREST : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestActionREST(QObject *parent = nullptr)
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

void TestActionREST::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestActionREST::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("10"));

    QDir buildDir = QDir::current();
    std::ignore   = buildDir.cd(u".."_s);

    QDir current        = buildDir;
    QString pluginPaths = current.absolutePath();

    std::ignore = current.cd(u"Cutelyst/Actions/REST"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release/Cutelyst/Actions/REST"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug/Cutelyst/Actions/REST"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    qDebug() << "setting CUTELYST_PLUGINS_DIR to" << pluginPaths;
    qputenv("CUTELYST_PLUGINS_DIR", pluginPaths.toLocal8Bit());

    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ActionRESTController(app);

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestActionREST::cleanupTestCase()
{
    delete m_engine;
}

void TestActionREST::doTest()
{
    QFETCH(QByteArray, method);
    QFETCH(QString, url);
    QFETCH(int, statusCode);
    QFETCH(QByteArray, output);
    QFETCH(QString, allow);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        method, urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.statusCode, statusCode);
    QCOMPARE(result.body, output);
    QCOMPARE(result.headers.header("Allow"), allow.toLatin1());
}

void TestActionREST::testController_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<QString>("allow");

    const auto head         = "HEAD"_ba;
    const auto get          = "GET"_ba;
    const auto put          = "PUT"_ba;
    const auto options      = "OPTIONS"_ba;
    const auto methodDELETE = "DELETE"_ba;

    QTest::newRow("rest-test1-00") << get << u"/action/rest/test1/"_s << 200
                                   << QByteArrayLiteral("test1.test1 GET.") << QString{};
    QTest::newRow("rest-test1-01") << head << u"/action/rest/test1/"_s << 200
                                   << QByteArrayLiteral("test1.test1 HEAD.") << QString{};
    QTest::newRow("rest-test1-02")
        << options << u"/action/rest/test1/"_s << 200 << QByteArrayLiteral("") << u"GET, HEAD"_s;

    // Test custom NOT implemented
    QTest::newRow("rest-test1-03")
        << put << u"/action/rest/test1/"_s << 200
        << QByteArrayLiteral("test1.test1 NOT IMPLEMENTED.") << QString{};

    QTest::newRow("rest-test2-00") << get << u"/action/rest/test2/"_s << 200
                                   << QByteArrayLiteral("test2.test2 GET.") << QString{};
    // HEAD when unavailable redispatches to GET
    QTest::newRow("rest-test2-01") << head << u"/action/rest/test2/"_s << 200
                                   << QByteArrayLiteral("test2.test2 GET.") << QString{};

    QTest::newRow("rest-test2-02") << options << u"/action/rest/test2/"_s << 200
                                   << QByteArrayLiteral("") << u"DELETE, GET, HEAD"_s;

    // Test default NOT implemented
    QTest::newRow("rest-test2-03")
        << put << u"/action/rest/test2/"_s << 405
        << QByteArrayLiteral("Method PUT not implemented for http://127.0.0.1/action/rest/test2/")
        << u"DELETE, GET, HEAD"_s;
    QTest::newRow("rest-test2-04") << methodDELETE << u"/action/rest/test2/"_s << 200
                                   << QByteArrayLiteral("test2.test2 DELETE.") << QString{};
}

QTEST_MAIN(TestActionREST)

#include "testactionrest.moc"

#endif
