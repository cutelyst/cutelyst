#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;

class Tst_Dispatcher : public CoverageObject
{
    Q_OBJECT
public:
    explicit Tst_Dispatcher(QObject *parent = nullptr)
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

void Tst_Dispatcher::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *Tst_Dispatcher::getEngine()
{
    auto app                    = new TestApplication;
    app->m_enableRootController = false;

    auto engine = new TestEngine(app, QVariantMap());
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void Tst_Dispatcher::cleanupTestCase()
{
    delete m_engine;
}

void Tst_Dispatcher::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.body, output);
}

void Tst_Dispatcher::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("path-test00") << QStringLiteral("/test/unknown_resource")
                                 << QByteArrayLiteral("Unknown resource '/test/unknown_resource'.");
}

QTEST_MAIN(Tst_Dispatcher)

#include "tst_dispatcher.moc"
