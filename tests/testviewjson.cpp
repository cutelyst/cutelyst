#ifndef RENDERVIEWTEST_H
#define RENDERVIEWTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>
#include <QRegularExpression>

#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/View>
#include <Cutelyst/Plugins/View/JSON/viewjson.h>

using namespace Cutelyst;

class TestViewJSON : public Controller
{
    Q_OBJECT
public:
    explicit TestViewJSON(QObject *parent) : Controller(parent) {}

    C_ATTR(test0, :Local)
    void test0(Context *c) {
        c->response()->setContentType(QStringLiteral("text/plain"));
        c->setStash(QStringLiteral("foo"), QByteArrayLiteral("bar"));
        c->setStash(QStringLiteral("bar"), QByteArrayLiteral("baz"));
        c->forward(c->view(QString()));
    }

    C_ATTR(test1, :Local)
    void test1(Context *c) {
        c->setStash(QStringLiteral("foo"), QByteArrayLiteral("bar"));
        c->setStash(QStringLiteral("bar"), QByteArrayLiteral("baz"));
        c->setStash(QStringLiteral("SingleKey"), QByteArrayLiteral("ok"));
        c->forward(c->view(QStringLiteral("view1")));
    }

    C_ATTR(test2, :Local)
    void test2(Context *c) {
        c->setStash(QStringLiteral("foo"), QByteArrayLiteral("bar"));
        c->setStash(QStringLiteral("bar"), QByteArrayLiteral("baz"));
        c->setStash(QStringLiteral("SingleKey"), QByteArrayLiteral("ok"));
        c->setStash(QStringLiteral("One"), 1);
        c->setStash(QStringLiteral("Two"), 2);
        c->forward(c->view(QStringLiteral("view2")));
    }

    C_ATTR(test3, :Local)
    void test3(Context *c) {
        c->setStash(QStringLiteral("foo"), QByteArrayLiteral("bar"));
        c->setStash(QStringLiteral("bar"), QByteArrayLiteral("baz"));
        c->setStash(QStringLiteral("SingleKey"), QByteArrayLiteral("ok"));
        c->setStash(QStringLiteral("One"), 1);
        c->setStash(QStringLiteral("Two"), 2);
        c->setStash(QStringLiteral("3"), 3);
        c->setStash(QStringLiteral("4"), 4);
        c->forward(c->view(QStringLiteral("view3")));
    }

    C_ATTR(test4, :Local)
    void test4(Context *c) {
        c->setStash(QStringLiteral("1"), 1);
        c->forward(c->view(QStringLiteral("view4")));
    }
};

class TestActionRenderView : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestActionRenderView(QObject *parent = nullptr) : CoverageObject(parent) {}

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

void TestActionRenderView::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestActionRenderView::getEngine()
{
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new TestViewJSON(app);

    new ViewJson(app);
    auto v1 = new ViewJson(app, QStringLiteral("view1"));
    v1->setExposeStash(QStringLiteral("SingleKey"));
    v1->setXJsonHeader(true);

    auto v2 = new ViewJson(app, QStringLiteral("view2"));
    v2->setExposeStash({ QStringLiteral("One"), QStringLiteral("Two") });
    v2->setXJsonHeader(true);

    auto v3 = new ViewJson(app, QStringLiteral("view3"));
    v3->setExposeStash(QRegularExpression(QStringLiteral("\\d")));

    auto v4 = new ViewJson(app, QStringLiteral("view4"));
    v4->setOutputFormat(ViewJson::Indented);

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestActionRenderView::cleanupTestCase()
{
    delete m_engine;
}

void TestActionRenderView::doTest()
{
    QFETCH(QString, method);
    QFETCH(QString, url);
    QFETCH(bool, sendXJsonVersion);
    QFETCH(int, statusCode);
    QFETCH(QByteArray, output);
    QFETCH(QString, contentType);
    QFETCH(bool, hasXJson);

    QUrl urlAux(url.mid(1));
    Headers sendHeaders;
    if (sendXJsonVersion) {
        sendHeaders.pushRawHeader(QStringLiteral("X_PROTOTYPE_VERSION"), QStringLiteral("1.5.0"));
    }
    QVariantMap result = m_engine->createRequest(method,
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 sendHeaders,
                                                 nullptr);

    QCOMPARE(result.value(QStringLiteral("statusCode")).toInt(), statusCode);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
    Headers headers = result.value(QStringLiteral("headers")).value<Headers>();
    QCOMPARE(headers.header(QStringLiteral("CONTENT_TYPE")), contentType);
    QCOMPARE(headers.contains(QStringLiteral("X_JSON")), hasXJson);
}

void TestActionRenderView::testController_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("sendXJsonVersion");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<QString>("contentType");
    QTest::addColumn<bool>("hasXJson");

    QTest::newRow("viewjson-test-00") << QStringLiteral("GET") << QStringLiteral("/test/view/json/test0") << true
                                      << 200 << QByteArrayLiteral("{\"bar\":\"baz\",\"foo\":\"bar\"}") << QStringLiteral("application/json") << false;
    QTest::newRow("viewjson-test-01") << QStringLiteral("GET") << QStringLiteral("/test/view/json/test1") << true
                                      << 200 << QByteArrayLiteral("{\"SingleKey\":\"ok\"}") << QStringLiteral("application/json") << true;
    QTest::newRow("viewjson-test-02") << QStringLiteral("GET") << QStringLiteral("/test/view/json/test2") << false
                                      << 200 << QByteArrayLiteral("{\"One\":1,\"Two\":2}") << QStringLiteral("application/json") << false;
    QTest::newRow("viewjson-test-03") << QStringLiteral("GET") << QStringLiteral("/test/view/json/test3") << true
                                      << 200 << QByteArrayLiteral("{\"3\":3,\"4\":4}") << QStringLiteral("application/json") << false;
    QTest::newRow("viewjson-test-04") << QStringLiteral("GET") << QStringLiteral("/test/view/json/test4") << false
                                      << 200 << QByteArrayLiteral("{\n    \"1\": 1\n}\n") << QStringLiteral("application/json") << false;
}

QTEST_MAIN(TestActionRenderView)

#include "testviewjson.moc"

#endif
