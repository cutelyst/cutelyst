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
    TestEngine *m_engine = nullptr;

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
        const Controller *controller = c->controller(c->request()->queryParam(u"name"_s));
        if (!controller) {
            c->response()->setBody(u"__NOT_FOUND__"_s);
        } else {
            c->response()->setBody(QByteArray(controller->metaObject()->className()));
        }
    }

    C_ATTR(forwardToActionString, :Local :AutoArgs)
    void forwardToActionString(Context *c) { c->forward(c->request()->queryParam(u"action"_s)); }

    C_ATTR(getAction, :Local :AutoArgs)
    void getAction(Context *c)
    {
        const Action *action =
            c->getAction(c->request()->queryParam(u"action"_s), c->request()->queryParam(u"ns"_s));
        if (!action) {
            c->response()->setBody(u"__NOT_FOUND__"_s);
        } else {
            c->response()->setBody(action->reverse());
        }
    }

    C_ATTR(getActions, :Local :AutoArgs)
    void getActions(Context *c)
    {
        const ActionList actions =
            c->getActions(c->request()->queryParam(u"action"_s), c->request()->queryParam(u"ns"_s));
        if (actions.isEmpty()) {
            c->response()->setBody(u"__NOT_FOUND__"_s);
        } else {
            QString ret;
            for (const Action *action : actions) {
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
    query.addQueryItem(u"path"_s, u"/root"_s);
    QTest::newRow("urifor-test00") << u"/uriFor?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(u"path"_s, u"/root/"_s);
    QTest::newRow("urifor-test01") << u"/uriFor?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root/");

    query.clear();
    query.addQueryItem(u"path"_s, u"/root"_s);
    QTest::newRow("urifor-test02") << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(u"path"_s, u"/root/"_s);
    QTest::newRow("urifor-test03") << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root//a/b/c");

    query.clear();
    query.addQueryItem(u"path"_s, u"/new/path"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    query.addQueryItem(u"encoded"_s, u"ç€¢"_s);
    QTest::newRow("urifor-test04")
        << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral(
               "http://127.0.0.1/new/path/a/b/c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(u"path"_s, u"/new/path///"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    query.addQueryItem(u"encoded"_s, u"ç€¢"_s);
    QTest::newRow("urifor-test05")
        << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral(
               "http://127.0.0.1/new/path////a/b/c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(u"path"_s, u"root"_s); // no leading slash
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("urifor-test06") << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(u"path"_s,
                       u""_s); // empty path to test controller->ns()
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("urifor-test07")
        << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(u"path"_s, u"/root"_s);
    QTest::newRow("urifor-test08") << u"/uriFor/a space/b/c?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/root/a space/b/c");

    query.clear();
    query.addQueryItem(u"path"_s, u"/root"_s);
    QTest::newRow("urifor-test09")
        << u"/uriFor/a%20space/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a space/b/c");

    query.clear();
    query.addQueryItem(u"path"_s, u"/"_s);
    QTest::newRow("urifor-test10") << u"/uriFor/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("http://127.0.0.1/a/b/c");

    // UriForAction Path
    query.clear();
    query.addQueryItem(u"action"_s, u"/root"_s);
    QTest::newRow("uriforaction-test00") << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
                                         << QByteArrayLiteral("uriForAction not found");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/global"_s);
    QTest::newRow("uriforaction-test01") << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
                                         << QByteArrayLiteral("http://127.0.0.1/global");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/many"_s);
    QTest::newRow("uriforaction-test02")
        << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/many"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test03")
        << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many?foo=bar");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/many"_s);
    QTest::newRow("uriforaction-test04")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/many"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test05")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/many/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/one"_s);
    QTest::newRow("uriforaction-test06")
        << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/one"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test07")
        << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one?foo=bar");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/one"_s);
    QTest::newRow("uriforaction-test08")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/one"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test09")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/test/controller/one/a/b/c?foo=bar");

    // UriForAction Chained
    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/root"_s);
    QTest::newRow("uriforaction-test10") << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
                                         << QByteArrayLiteral("http://127.0.0.1/root");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/root"_s);
    QTest::newRow("uriforaction-test11")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/root"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test12")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/rootItem"_s);
    QTest::newRow("uriforaction-test13") << u"/uriForAction?"_s + query.toString(QUrl::FullyEncoded)
                                         << QByteArrayLiteral("http://127.0.0.1/root/item");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/rootItem"_s);
    QTest::newRow("uriforaction-test14")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/rootItem"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    QTest::newRow("uriforaction-test15")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/root/item/a/b/c?foo=bar");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/midleEnd"_s);
    query.addQueryItem(u"captures"_s, u"1"_s);
    QTest::newRow("uriforaction-test16")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/a/end/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/midleEnd"_s);
    query.addQueryItem(u"captures"_s, u"1/2"_s);
    QTest::newRow("uriforaction-test17")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/c");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/midleEnd"_s);
    query.addQueryItem(u"captures"_s, u"1/2"_s);
    query.addQueryItem(u"foo"_s, u"bar"_s);
    query.addQueryItem(u"encoded"_s, u"ç€¢"_s);
    QTest::newRow("uriforaction-test18")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("http://127.0.0.1/chain/midle/1/2/end/a/b/"
                             "c?foo=bar&encoded=\xC3\xA7\xE2\x82\xAC\xC2\xA2");

    query.clear();
    query.addQueryItem(u"action"_s, u"/test/controller/midleEnd"_s);
    query.addQueryItem(u"captures"_s, u"1/2/3"_s); // too many captures
    QTest::newRow("uriforaction-test19")
        << u"/uriForAction/a/b/c?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("uriForAction not found");

    QTest::newRow("context-test00")
        << u"/context/test_ns/actionName"_s << QByteArrayLiteral("actionName");
    QTest::newRow("context-test01")
        << u"/context/test_ns/ns"_s << QByteArrayLiteral("context/test_ns");
    QTest::newRow("context-test02")
        << u"/context/test_ns/controllerName"_s << QByteArrayLiteral("ContextTest_NS");
    QTest::newRow("context-test03")
        << u"/context/test_ns/controller"_s << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(u"name"_s, u"RootController"_s);
    QTest::newRow("context-test04")
        << u"/context/test_ns/controller?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("RootController");

    query.clear();
    query.addQueryItem(u"name"_s, u"ContextGetActionsTest"_s);
    QTest::newRow("context-test05")
        << u"/context/test_ns/controller?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("ContextGetActionsTest");

    // Forward
    query.clear();
    query.addQueryItem(u"action"_s, u"actionName"_s);
    QTest::newRow("forward-test00")
        << u"/context/test_ns/forwardToActionString?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("forwardToActionString");

    query.clear();
    query.addQueryItem(u"action"_s, u"ns"_s);
    QTest::newRow("forward-test01")
        << u"/context/test_ns/forwardToActionString?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns");

    query.clear();
    query.addQueryItem(u"action"_s, u"forwardToActionString"_s);
    QTest::newRow("forward-test01")
        << u"/context/test_ns/forwardToActionString?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Deep recursion detected (stack size 50) calling "
                             "context/test_ns/forwardToActionString, forwardToActionString");

    // GetAction
    query.clear();
    query.addQueryItem(u"action"_s, u"actionName"_s);
    QTest::newRow("getaction-test00")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(u"action"_s, u"actionName"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getaction-test01")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/actionName");

    query.clear();
    query.addQueryItem(u"action"_s, u"chain"_s);
    query.addQueryItem(u"ns"_s, u"test/controller"_s);
    QTest::newRow("getaction-test02")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/chain");

    query.clear();
    query.addQueryItem(u"action"_s, u"global"_s);
    QTest::newRow("getaction-test03")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(u"action"_s, u"global"_s);
    query.addQueryItem(u"ns"_s, u"test/controller"_s);
    QTest::newRow("getaction-test04")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/global");

    query.clear();
    query.addQueryItem(u"action"_s, u"rootActionOnControllerWithoutNamespace"_s);
    query.addQueryItem(u"ns"_s, u""_s);
    QTest::newRow("getaction-test05")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace");

    query.clear();
    query.addQueryItem(u"action"_s, u"rootActionOnControllerWithoutNamespace"_s);
    QTest::newRow("getaction-test06")
        << u"/context/test_ns/getAction?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace");

    // GetActions
    query.clear();
    query.addQueryItem(u"action"_s, u"ns"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getactions-test00")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/ns;");

    query.clear();
    query.addQueryItem(u"action"_s, u"actionName"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getactions-test01")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/actionName;context/test_ns/actionName;");

    query.clear();
    query.addQueryItem(u"action"_s, u"chain"_s);
    query.addQueryItem(u"ns"_s, u"test/controller"_s);
    QTest::newRow("getactions-test02")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("test/controller/chain;");

    query.clear();
    query.addQueryItem(u"action"_s, u"Begin"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getactions-test03")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Begin;context/test_ns/Begin;");

    query.clear();
    query.addQueryItem(u"action"_s, u"Auto"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getactions-test04")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("Auto;context/test_ns/Auto;");

    query.clear();
    query.addQueryItem(u"action"_s, u"End"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns"_s);
    QTest::newRow("getactions-test05")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("End;context/test_ns/End;");

    query.clear();
    query.addQueryItem(u"action"_s, u"rootActionOnControllerWithoutNamespace"_s);
    query.addQueryItem(u"ns"_s, u"any/name/space/will/give/a/match"_s);
    QTest::newRow("getactions-test06")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("rootActionOnControllerWithoutNamespace;");

    query.clear();
    query.addQueryItem(u"action"_s, u"actionName"_s);
    query.addQueryItem(u"ns"_s, u"any/name/space/will/NOT/give/a/match"_s);
    QTest::newRow("getactions-test06")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("__NOT_FOUND__");

    query.clear();
    query.addQueryItem(u"action"_s, u"ns"_s);
    query.addQueryItem(u"ns"_s, u"context/test_ns/with/this/extra/invalid/namespace/will/match"_s);
    QTest::newRow("getactions-test00")
        << u"/context/test_ns/getActions?"_s + query.toString(QUrl::FullyEncoded)
        << QByteArrayLiteral("context/test_ns/ns;");
}

QTEST_MAIN(TestContext)

#include "testcontext.moc"

#endif
