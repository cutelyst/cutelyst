#ifndef RENDERVIEWTEST_H
#define RENDERVIEWTEST_H

#include "coverageobject.h"

#include <Cutelyst/Plugins/View/JSON/viewjson.h>
#include <Cutelyst/View>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>

#include <QRegularExpression>
#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class TestViewJSON : public Controller
{
    Q_OBJECT
public:
    explicit TestViewJSON(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(test0, :Local)
    void test0(Context *c)
    {
        c->response()->setContentType("text/plain"_ba);
        c->setStash(u"foo"_s, QByteArrayLiteral("bar"));
        c->setStash(u"bar"_s, QByteArrayLiteral("baz"));
        c->forward(c->view());
    }

    C_ATTR(test1, :Local)
    void test1(Context *c)
    {
        c->setStash(u"foo"_s, QByteArrayLiteral("bar"));
        c->setStash(u"bar"_s, QByteArrayLiteral("baz"));
        c->setStash(u"SingleKey"_s, QByteArrayLiteral("ok"));
        c->forward(c->view(u"view1"));
    }

    C_ATTR(test2, :Local)
    void test2(Context *c)
    {
        c->setStash(u"foo"_s, QByteArrayLiteral("bar"));
        c->setStash(u"bar"_s, QByteArrayLiteral("baz"));
        c->setStash(u"SingleKey"_s, QByteArrayLiteral("ok"));
        c->setStash(u"One"_s, 1);
        c->setStash(u"Two"_s, 2);
        c->forward(c->view(u"view2"_s));
    }

    C_ATTR(test3, :Local)
    void test3(Context *c)
    {
        c->setStash(u"foo"_s, QByteArrayLiteral("bar"));
        c->setStash(u"bar"_s, QByteArrayLiteral("baz"));
        c->setStash(u"SingleKey"_s, QByteArrayLiteral("ok"));
        c->setStash(u"One"_s, 1);
        c->setStash(u"Two"_s, 2);
        c->setStash(u"3"_s, 3);
        c->setStash(u"4"_s, 4);
        c->forward(c->view(u"view3"_s));
    }

    C_ATTR(test4, :Local)
    void test4(Context *c)
    {
        c->setStash(u"1"_s, 1);
        c->forward(c->view(u"view4"_s));
    }
};

class TestActionRenderView : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestActionRenderView(QObject *parent = nullptr)
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

void TestActionRenderView::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestActionRenderView::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new TestViewJSON(app);

    new ViewJson(app);
    auto v1 = new ViewJson(app, u"view1"_s);
    v1->setExposeStash(u"SingleKey"_s);
    v1->setXJsonHeader(true);

    auto v2 = new ViewJson(app, u"view2"_s);
    v2->setExposeStash({u"One"_s, u"Two"_s});
    v2->setXJsonHeader(true);

    auto v3 = new ViewJson(app, u"view3"_s);
    v3->setExposeStash(QRegularExpression(u"\\d"_s));

    auto v4 = new ViewJson(app, u"view4"_s);
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
    QFETCH(QByteArray, method);
    QFETCH(QString, url);
    QFETCH(bool, sendXJsonVersion);
    QFETCH(int, statusCode);
    QFETCH(QByteArray, output);
    QFETCH(QString, contentType);
    QFETCH(bool, hasXJson);

    QUrl urlAux(url);
    Headers sendHeaders;
    if (sendXJsonVersion) {
        sendHeaders.pushHeader("X-Prototype-Version"_ba, "1.5.0");
    }
    auto result = m_engine->createRequest(
        method, urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), sendHeaders, nullptr);

    QCOMPARE(result.statusCode, statusCode);
    QCOMPARE(result.body, output);
    Headers headers = result.headers;
    QCOMPARE(headers.header("Content-Type"), contentType.toLatin1());
    QCOMPARE(headers.contains("X-Json"), hasXJson);
}

void TestActionRenderView::testController_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("sendXJsonVersion");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<QString>("contentType");
    QTest::addColumn<bool>("hasXJson");

    const auto get = "GET"_ba;

    QTest::newRow("viewjson-test-00")
        << get << u"/test/view/json/test0"_s << true << 200
        << QByteArrayLiteral("{\"bar\":\"baz\",\"foo\":\"bar\"}") << u"application/json"_s << false;
    QTest::newRow("viewjson-test-01")
        << get << u"/test/view/json/test1"_s << true << 200
        << QByteArrayLiteral("{\"SingleKey\":\"ok\"}") << u"application/json"_s << true;
    QTest::newRow("viewjson-test-02")
        << get << u"/test/view/json/test2"_s << false << 200
        << QByteArrayLiteral("{\"One\":1,\"Two\":2}") << u"application/json"_s << false;
    QTest::newRow("viewjson-test-03")
        << get << u"/test/view/json/test3"_s << true << 200
        << QByteArrayLiteral("{\"3\":3,\"4\":4}") << u"application/json"_s << false;
    QTest::newRow("viewjson-test-04")
        << get << u"/test/view/json/test4"_s << false << 200
        << QByteArrayLiteral("{\n    \"1\": 1\n}\n") << u"application/json"_s << false;
}

QTEST_MAIN(TestActionRenderView)

#include "testviewjson.moc"

#endif
