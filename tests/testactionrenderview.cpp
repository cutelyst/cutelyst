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

class TestView : public View
{
    Q_OBJECT
public:
    TestView(QObject *parent, const QString &name = QString())
        : View(parent, name)
    {
    }

    virtual QByteArray render(Context *c) const override
    {
        return name().toLatin1() + c->stash(QStringLiteral("data")).toByteArray();
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
    void test0(Context *c) { c->setStash(QStringLiteral("data"), QByteArrayLiteral("test0")); }

    C_ATTR(test1, :Local :ActionClass(RenderView) :View(view1))
    void test1(Context *c) { c->setStash(QStringLiteral("data"), QByteArrayLiteral("test1")); }

    C_ATTR(test2, :Local :ActionClass(RenderView))
    void test2(Context *c)
    {
        c->setCustomView(QStringLiteral("view2"));
        c->setStash(QStringLiteral("data"), QByteArrayLiteral("test2"));
    }

    C_ATTR(test3, :Local :ActionClass(RenderView))
    void test3(Context *c)
    {
        c->response()->setContentType("plain/text"_qba);
        c->setStash(QStringLiteral("data"), QByteArrayLiteral("test3"));
    }

    C_ATTR(test4, :Local :ActionClass(RenderView) :View(not_defined_view))
    void test4(Context *c) { c->setStash(QStringLiteral("data"), QByteArrayLiteral("test4")); }

    C_ATTR(test6, :Local :ActionClass(RenderView))
    void test6(Context *c)
    {
        c->response()->setBody(QByteArrayLiteral("test6 body"));
        c->setStash(QStringLiteral("data"), QByteArrayLiteral("test6"));
    }

    C_ATTR(test7, :Local :ActionClass(RenderView))
    bool test7(Context *c)
    {
        c->setStash(QStringLiteral("data"), QByteArrayLiteral("test7"));
        return false;
    }

    C_ATTR(testStatus, :Local :ActionClass(RenderView) :AutoArgs)
    void testStatus(Context *c, const QString &status)
    {
        c->response()->setStatus(status.toUInt());
        c->setStash(QStringLiteral("data"), QByteArrayLiteral("rendered"));
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
    TestEngine *m_engine;

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
    buildDir.cd(QStringLiteral(".."));

    QDir current        = buildDir;
    QString pluginPaths = current.absolutePath();

    current.cd(QStringLiteral("Cutelyst/Actions/RenderView"));
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current = buildDir;
    current.cd(QStringLiteral("Release"));
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current = buildDir;
    current.cd(QStringLiteral("Release/Cutelyst/Actions/RenderView"));
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current = buildDir;
    current.cd(QStringLiteral("Debug"));
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    current = buildDir;
    current.cd(QStringLiteral("Debug/Cutelyst/Actions/RenderView"));
    pluginPaths += QLatin1Char(';') + current.absolutePath();

    qDebug() << "setting CUTELYST_PLUGINS_DIR to" << pluginPaths;
    qputenv("CUTELYST_PLUGINS_DIR", pluginPaths.toLocal8Bit());

    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ActionRenderView(app);
    new TestView(app);
    new TestView(app, QStringLiteral("view1"));
    new TestView(app, QStringLiteral("view2"));

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

    const auto get  = "GET"_qba;
    const auto head = "HEAD"_qba;

    QTest::newRow("renderview-test-00")
        << get << QStringLiteral("/action/render/view/test0") << 200 << QByteArrayLiteral("test0")
        << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-test-01")
        << get << QStringLiteral("/action/render/view/test1") << 200
        << QByteArrayLiteral("view1test1") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-test-02")
        << get << QStringLiteral("/action/render/view/test2") << 200
        << QByteArrayLiteral("view2test2") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-test-03")
        << get << QStringLiteral("/action/render/view/test3") << 200 << QByteArrayLiteral("test3")
        << QStringLiteral("plain/text");
    QTest::newRow("renderview-test-04")
        << get << QStringLiteral("/action/render/view/test4") << 500 << QByteArrayLiteral("")
        << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-test-05")
        << head << QStringLiteral("/action/render/view/test3") << 200 << QByteArrayLiteral("")
        << QStringLiteral("plain/text");
    QTest::newRow("renderview-test-06")
        << get << QStringLiteral("/action/render/view/test6") << 200
        << QByteArrayLiteral("test6 body") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-test-07") << get << QStringLiteral("/action/render/view/test7") << 200
                                        << QByteArrayLiteral("") << QStringLiteral("");

    QTest::newRow("renderview-testStatus-00")
        << get << QStringLiteral("/action/render/view/testStatus/204") << 204
        << QByteArrayLiteral("") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-testStatus-01")
        << get << QStringLiteral("/action/render/view/testStatus/300") << 300
        << QByteArrayLiteral("") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-testStatus-02")
        << get << QStringLiteral("/action/render/view/testStatus/399") << 399
        << QByteArrayLiteral("") << QStringLiteral("text/html; charset=utf-8");
    QTest::newRow("renderview-testStatus-03")
        << get << QStringLiteral("/action/render/view/testStatus/200") << 200
        << QByteArrayLiteral("rendered") << QStringLiteral("text/html; charset=utf-8");
}

QTEST_MAIN(TestActionRenderView)

#include "testactionrenderview.moc"

#endif
