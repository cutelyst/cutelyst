#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QObject>
#include <QTest>
#include <QUrlQuery>

using namespace Cutelyst;

class TestContext : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestContext(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void cleanupTestCase();

private:
    TestEngine *m_engine;

    TestEngine *getEngine();

    void doTest();
};

class ContextGetActionsTest : public Controller
{
    Q_OBJECT
    C_NAMESPACE("context")
public:
    explicit ContextGetActionsTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(actionName, :Local :AutoArgs)
    void actionName(Context *c) { c->response()->setBody(c->actionName()); }
};

class ContextTest_NS : public Controller
{
    Q_OBJECT
public:
    explicit ContextTest_NS(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(actionName, :Local :AutoArgs)
    void actionName(Context *c) { c->response()->setBody(c->actionName()); }

    C_ATTR(ns, :Local :AutoArgs)
    void ns(Context *c) { c->response()->setBody(c->ns()); }

    C_ATTR(controllerName, :Local :AutoArgs)
    void controllerName(Context *c) { c->response()->setBody(c->controllerName()); }

    C_ATTR(controller, :Local :AutoArgs)
    void controller(Context *c)
    {
        Controller *controller = c->controller(c->request()->queryParam(QStringLiteral("name")));
        if (!controller) {
            c->response()->setBody(QStringLiteral("__NOT_FOUND__"));
        } else {
            c->response()->setBody(QByteArray(controller->metaObject()->className()));
        }
    }

    C_ATTR(forwardToActionString, :Local :AutoArgs)
    void forwardToActionString(Context *c)
    {
        c->forward(c->request()->queryParam(QStringLiteral("action")));
    }

    C_ATTR(getAction, :Local :AutoArgs)
    void getAction(Context *c)
    {
        Action *action = c->getAction(c->request()->queryParam(QStringLiteral("action")),
                                      c->request()->queryParam(QStringLiteral("ns")));
        if (!action) {
            c->response()->setBody(QStringLiteral("__NOT_FOUND__"));
        } else {
            c->response()->setBody(action->reverse());
        }
    }

    C_ATTR(getActions, :Local :AutoArgs)
    void getActions(Context *c)
    {
        const ActionList actions = c->getActions(c->request()->queryParam(QStringLiteral("action")),
                                                 c->request()->queryParam(QStringLiteral("ns")));
        if (actions.isEmpty()) {
            c->response()->setBody(QStringLiteral("__NOT_FOUND__"));
        } else {
            QString ret;
            for (Action *action : actions) {
                ret.append(action->reverse() + QLatin1Char(';'));
            }
            c->response()->setBody(ret);
        }
    }

private:
    C_ATTR(Begin,)
    bool Begin(Context *) { return true; }

    C_ATTR(Auto,)
    bool Auto(Context *) { return true; }

    C_ATTR(End,)
    bool End(Context *) { return true; }
};

void TestContext::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestContext::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("50"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ContextGetActionsTest(app);
    new ContextTest_NS(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestContext::cleanupTestCase()
{
    delete m_engine;
}

void TestContext::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.body, output);
}

void TestContext::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // UriFor
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test00")
        << QStringLiteral("/uriFor?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root/"));
    QTest::newRow("urifor-test01")
        << QStringLiteral("/uriFor?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test02")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root/"));
    QTest::newRow("urifor-test03")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root//a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/new/path"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("urifor-test04")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral(
               "http://127.0.0.1/new/path/a/b/c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/new/path///"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("urifor-test05")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral(
               "http://127.0.0.1/new/path////a/b/c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("root")); // no leading slash
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("urifor-test06")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("path"),
                       QStringLiteral("")); // empty path to test controller->ns()
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("urifor-test07")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test08")
        << QStringLiteral("/uriFor/a space/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a space/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/root"));
    QTest::newRow("urifor-test09")
        << QStringLiteral("/uriFor/a%20space/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a space/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("path"), QStringLiteral("/"));
    QTest::newRow("urifor-test10")
        << QStringLiteral("/uriFor/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/a/b/c");

    // UriForAction Path
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/root"));
    QTest::newRow("uriforaction-test00")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("uriForAction not found");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/global"));
    QTest::newRow("uriforaction-test01")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/global");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    QTest::newRow("uriforaction-test02")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test03")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    QTest::newRow("uriforaction-test04")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/many"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test05")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    QTest::newRow("uriforaction-test06")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test07")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    QTest::newRow("uriforaction-test08")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/one"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test09")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c?foo=bar");

    // UriForAction Chained
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    QTest::newRow("uriforaction-test10")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    QTest::newRow("uriforaction-test11")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/root"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test12")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    QTest::newRow("uriforaction-test13")
        << QStringLiteral("/uriForAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/item");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    QTest::newRow("uriforaction-test14")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/rootItem"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriforaction-test15")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1"));
    QTest::newRow("uriforaction-test16")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/a/end/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2"));
    QTest::newRow("uriforaction-test17")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/c");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("encoded"), QStringLiteral("ç€¢"));
    QTest::newRow("uriforaction-test18")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/"
                             "c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("/test/controller/midleEnd"));
    query.addQueryItem(QStringLiteral("captures"), QStringLiteral("1/2/3")); // too many captures
    QTest::newRow("uriforaction-test19")
        << QStringLiteral("/uriForAction/a/b/c?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("uriForAction not found");

    QTest::newRow("context-test00")
        << QStringLiteral("/context/test_ns/actionName") << QByteArrayLiteral("actionName");
    QTest::newRow("context-test01")
        << QStringLiteral("/context/test_ns/ns") << QByteArrayLiteral("context/test_ns");
    QTest::newRow("context-test02")
        << QStringLiteral("/context/test_ns/controllerName") << QByteArrayLiteral("ContextTest_NS");
    QTest::newRow("context-test03")
        << QStringLiteral("/context/test_ns/controller") << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("RootController"));
    QTest::newRow("context-test04")
        << QStringLiteral("/context/test_ns/controller?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("RootController");

    query.clear();
    query.addQueryItem(QStringLiteral("name"), QStringLiteral("ContextGetActionsTest"));
    QTest::newRow("context-test05")
        << QStringLiteral("/context/test_ns/controller?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("ContextGetActionsTest");

    // Forward
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("actionName"));
    QTest::newRow("forward-test00") << QStringLiteral("/context/test_ns/forwardToActionString?") +
                                           query.toString(QUrl::FullyEncoded)
                                    << QByteArrayLiteral("forwardToActionString");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("ns"));
    QTest::newRow("forward-test01") << QStringLiteral("/context/test_ns/forwardToActionString?") +
                                           query.toString(QUrl::FullyEncoded)
                                    << QByteArrayLiteral("context/test_ns");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("forwardToActionString"));
    QTest::newRow("forward-test01")
        << QStringLiteral("/context/test_ns/forwardToActionString?") +
               query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Deep recursion detected (stack size 50) calling "
                             "context/test_ns/forwardToActionString, forwardToActionString");

    // GetAction
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("actionName"));
    QTest::newRow("getaction-test00")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("actionName"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getaction-test01")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/actionName");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("chain"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("test/controller"));
    QTest::newRow("getaction-test02")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/chain");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("global"));
    QTest::newRow("getaction-test03")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("global"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("test/controller"));
    QTest::newRow("getaction-test04")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/global");

    query.clear();
    query.addQueryItem(QStringLiteral("action"),
                       QStringLiteral("rootActionOnControllerWithoutNamespace"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral(""));
    QTest::newRow("getaction-test05")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace");

    query.clear();
    query.addQueryItem(QStringLiteral("action"),
                       QStringLiteral("rootActionOnControllerWithoutNamespace"));
    QTest::newRow("getaction-test06")
        << QStringLiteral("/context/test_ns/getAction?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace");

    // GetActions
    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("ns"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getactions-test00")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/ns;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("actionName"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getactions-test01")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/actionName;context/test_ns/actionName;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("chain"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("test/controller"));
    QTest::newRow("getactions-test02")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/chain;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("Begin"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getactions-test03")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Begin;context/test_ns/Begin;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("Auto"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getactions-test04")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Auto;context/test_ns/Auto;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("End"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("context/test_ns"));
    QTest::newRow("getactions-test05")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("End;context/test_ns/End;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"),
                       QStringLiteral("rootActionOnControllerWithoutNamespace"));
    query.addQueryItem(QStringLiteral("ns"), QStringLiteral("any/name/space/will/give/a/match"));
    QTest::newRow("getactions-test06")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace;");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("actionName"));
    query.addQueryItem(QStringLiteral("ns"),
                       QStringLiteral("any/name/space/will/NOT/give/a/match"));
    QTest::newRow("getactions-test06")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("ns"));
    query.addQueryItem(
        QStringLiteral("ns"),
        QStringLiteral("context/test_ns/with/this/extra/invalid/namespace/will/match"));
    QTest::newRow("getactions-test00")
        << QStringLiteral("/context/test_ns/getActions?") + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/ns;");
}

QTEST_MAIN(TestContext)

#include "testcontext.moc"

#endif
