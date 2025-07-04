#ifndef RENDERVIEWTEST_H
#define RENDERVIEWTEST_H

#include "coverageobject.h"

#include <Cutelyst/View>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>

#include <QDir>
#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class TestView : public View
{
    Q_OBJECT
public:
    explicit TestView(QObject *parent, const QString &name = {})
        : View(parent, name)
    {
    }

    virtual QByteArray render(Context *c) const override
    {
        return name().toLatin1() + c->stash(u"data"_s).toByteArray();
    }
};

class ActionRenderView : public Controller
{
    Q_OBJECT
public:
    explicit ActionRenderView(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(test0, :Local :ActionClass(RenderView))
    void test0(Context *c) { c->setStash(u"data"_s, QByteArrayLiteral("test0")); }

    C_ATTR(test1, :Local :ActionClass(RenderView) :View(view1))
    void test1(Context *c) { c->setStash(u"data"_s, QByteArrayLiteral("test1")); }

    C_ATTR(test2, :Local :ActionClass(RenderView))
    void test2(Context *c)
    {
        c->setCustomView(u"view2"_s);
        c->setStash(u"data"_s, QByteArrayLiteral("test2"));
    }

    C_ATTR(test3, :Local :ActionClass(RenderView))
    void test3(Context *c)
    {
        c->response()->setContentType("plain/text"_ba);
        c->setStash(u"data"_s, QByteArrayLiteral("test3"));
    }

    C_ATTR(test4, :Local :ActionClass(RenderView) :View(not_defined_view))
    void test4(Context *c) { c->setStash(u"data"_s, QByteArrayLiteral("test4")); }

    C_ATTR(test6, :Local :ActionClass(RenderView))
    void test6(Context *c)
    {
        c->response()->setBody(QByteArrayLiteral("test6 body"));
        c->setStash(u"data"_s, QByteArrayLiteral("test6"));
    }

    C_ATTR(test7, :Local :ActionClass(RenderView))
    bool test7(Context *c)
    {
        c->setStash(u"data"_s, QByteArrayLiteral("test7"));
        return false;
    }

    C_ATTR(testStatus, :Local :ActionClass(RenderView) :AutoArgs)
    void testStatus(Context *c, const QString &status)
    {
        c->response()->setStatus(status.toUInt());
        c->setStash(u"data"_s, QByteArrayLiteral("rendered"));
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
    qputenv("RECURSION", QByteArrayLiteral("10"));

    QDir buildDir = QDir::current();
    std::ignore   = buildDir.cd(u".."_s);

    QDir current        = buildDir;
    QString pluginPaths = current.absolutePath();

    std::ignore = current.cd(u"Cutelyst/Actions/RenderView"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release/Cutelyst/Actions/RenderView"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug/Cutelyst/Actions/RenderView"_s);
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    qDebug() << "setting CUTELYST_PLUGINS_DIR to" << pluginPaths;
    qputenv("CUTELYST_PLUGINS_DIR", pluginPaths.toLocal8Bit());

    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ActionRenderView(app);
    new TestView(app);
    new TestView(app, u"view1"_s);
    new TestView(app, u"view2"_s);

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
    QFETCH(int, statusCode);
    QFETCH(QByteArray, output);
    QFETCH(QString, contentType);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        method, urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.statusCode, statusCode);
    QCOMPARE(result.body, output);
    QCOMPARE(result.headers.header("Content-Type"), contentType.toLatin1());
}

void TestActionRenderView::testController_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<QString>("contentType");

    const auto get  = "GET"_ba;
    const auto head = "HEAD"_ba;

    QTest::newRow("renderview-test-00")
        << get << u"/action/render/view/test0"_s << 200 << QByteArrayLiteral("test0")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-test-01")
        << get << u"/action/render/view/test1"_s << 200 << QByteArrayLiteral("view1test1")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-test-02")
        << get << u"/action/render/view/test2"_s << 200 << QByteArrayLiteral("view2test2")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-test-03") << get << u"/action/render/view/test3"_s << 200
                                        << QByteArrayLiteral("test3") << u"plain/text"_s;
    QTest::newRow("renderview-test-04") << get << u"/action/render/view/test4"_s << 500
                                        << QByteArrayLiteral("") << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-test-05") << head << u"/action/render/view/test3"_s << 200
                                        << QByteArrayLiteral("") << u"plain/text"_s;
    QTest::newRow("renderview-test-06")
        << get << u"/action/render/view/test6"_s << 200 << QByteArrayLiteral("test6 body")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-test-07")
        << get << u"/action/render/view/test7"_s << 200 << QByteArrayLiteral("") << u""_s;

    QTest::newRow("renderview-testStatus-00")
        << get << u"/action/render/view/testStatus/204"_s << 204 << QByteArrayLiteral("")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-testStatus-01")
        << get << u"/action/render/view/testStatus/300"_s << 300 << QByteArrayLiteral("")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-testStatus-02")
        << get << u"/action/render/view/testStatus/399"_s << 399 << QByteArrayLiteral("")
        << u"text/html; charset=utf-8"_s;
    QTest::newRow("renderview-testStatus-03")
        << get << u"/action/render/view/testStatus/200"_s << 200 << QByteArrayLiteral("rendered")
        << u"text/html; charset=utf-8"_s;
}

QTEST_MAIN(TestActionRenderView)

#include "testactionrenderview.moc"

#endif
