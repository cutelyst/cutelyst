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

class TestController : public Controller
{
    Q_OBJECT
public:
    TestController(QObject *parent) : Controller(parent) {}

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(hello, :Local :AutoArgs)
    void hello(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(global, :Global :AutoArgs)
    void global(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(many, :Local :AutoArgs)
    void many(Context *c, const QStringList &args) {
        Q_UNUSED(args)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(one, :Local :AutoArgs)
    void one(Context *c, const QString &one) {
        Q_UNUSED(one)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(two, :Local :AutoArgs)
    void two(Context *c, const QString &one, const QString &two) {
        Q_UNUSED(one)
        Q_UNUSED(two)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOld, :Local :Args)
    void manyOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOldWithNoArgs, :Local)
    void manyOldWithNoArgs(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(oneOld, :Local :Args(1))
    void oneOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(twoOld, :Local :Args(2))
    void twoOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }


    C_ATTR(root, :Chained("/"))
    void root(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/root"));
    }

    C_ATTR(rootItem, :Chained("root") :PathPart("item"))
    void rootItem(Context *c) {
        // Since root has no capture part this is never called
        c->response()->body().append(QByteArrayLiteral("/root/item"));
    }


    C_ATTR(chain, :Chained("/") :PathPart("chain") :CaptureArgs(0))
    void chain(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/chain"));
    }

    C_ATTR(item, :Chained("chain"))
    void item(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/item[MANY]/"));
        c->response()->body().append(c->request()->args().join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(itemOne, :Chained("chain") :PathPart("item") :AutoArgs)
    void itemOne(Context *c, const QString &arg) {
        c->response()->body().append(QByteArrayLiteral("/item[ONE]/"));
        c->response()->body().append(arg.toLatin1());
    }

    C_ATTR(midle, :Chained("chain") :AutoCaptureArgs)
    void midle(Context *c, const QString &first, const QString &second) {
        c->response()->body().append(QByteArrayLiteral("/midle/"));
        c->response()->body().append(first.toLatin1());
        c->response()->body().append(QByteArrayLiteral("/"));
        c->response()->body().append(second.toLatin1());
    }

    C_ATTR(midleEnd, :Chained("midle") :PathPart("end") :Args(0))
    void midleEnd(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/end"));
    }

    C_ATTR(midleEndMany, :Chained("midle") :PathPart("end") :Args)
    void midleEndMany(Context *c, const QStringList &args) {
        c->response()->body().append(QByteArrayLiteral("/end/"));
        c->response()->body().append(args.join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(uriFor, :Global :AutoArgs)
    void uriFor(Context *c, const QStringList &args) {
        auto query = c->request()->queryParameters();
        QString path = query.take(QStringLiteral("path"));
        qDebug() << query;
        c->response()->setBody(c->uriFor(path, args, query).toString());
    }

    C_ATTR(uriForAction, :Global :AutoArgs)
    void uriForAction(Context *c, const QStringList &args) {
        QStringList arguments = args;
        auto query = c->request()->queryParameters();
        QStringList captures = query.take(QStringLiteral("captures")).split(QLatin1Char('/'), QString::SkipEmptyParts);
        QString action = query.take(QStringLiteral("action"));
        QUrl uri = c->uriForAction(action, captures, arguments, query);
        if (uri.isEmpty()) {
            c->response()->setBody(QByteArray("uriForAction not found"));
        } else {
            c->response()->setBody(uri.toString());
        }
    }
};

class TestApplication : public Application
{
    Q_OBJECT
public:
    TestApplication(QObject *parent = nullptr) : Application(parent) {}
    virtual bool init() {
        new TestController(this);

        return true;
    }
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

    // Path dispatcher
    QTest::newRow("path-test00") << QStringLiteral("/test/unknown_resource") << QByteArrayLiteral("Unknown resource \"test/unknown_resource\".");
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

    // Chained dispatcher
    QTest::newRow("chained-test00") << QStringLiteral("/root") << QByteArrayLiteral("/root");
    QTest::newRow("chained-test01") << QStringLiteral("/root/") << QByteArrayLiteral("/root");
    QTest::newRow("chained-test02") << QStringLiteral("/root////") << QByteArrayLiteral("/root");
    QTest::newRow("chained-test03") << QStringLiteral("/root/item") << QByteArrayLiteral("/root");
    QTest::newRow("chained-test04") << QStringLiteral("/root/item/") << QByteArrayLiteral("/root");

    QTest::newRow("chained-test05") << QStringLiteral("/chain/item") << QByteArrayLiteral("/chain/item[MANY]/");
    QTest::newRow("chained-test06") << QStringLiteral("/chain/item/") << QByteArrayLiteral("/chain/item[MANY]/");
    QTest::newRow("chained-test07") << QStringLiteral("/chain/item/foo") << QByteArrayLiteral("/chain/item[ONE]/foo");
    QTest::newRow("chained-test08") << QStringLiteral("/chain/item/foo/bar") << QByteArrayLiteral("/chain/item[MANY]/foo/bar");

    QTest::newRow("chained-test09") << QStringLiteral("/chain/midle/one/two/end") << QByteArrayLiteral("/chain/midle/one/two/end");
    QTest::newRow("chained-test10") << QStringLiteral("/chain/midle/TWO/ONE/end") << QByteArrayLiteral("/chain/midle/TWO/ONE/end");
    QTest::newRow("chained-test11") << QStringLiteral("/chain/midle/one/two/end/") << QByteArrayLiteral("/chain/midle/one/two/end");
    QTest::newRow("chained-test12") << QStringLiteral("/chain/midle/TWO/ONE/end/1/2/3/4/5") << QByteArrayLiteral("/chain/midle/TWO/ONE/end/1/2/3/4/5");

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

#include "testdispatcher.moc"

#endif
