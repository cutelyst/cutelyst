#ifndef TESTLANGSELECTMANUAL_H
#define TESTLANGSELECTMANUAL_H

#include <QTest>
#include <QObject>
#include <QLocale>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/Utils/LangSelect/LangSelect>

using namespace Cutelyst;

class TestLangselectManual : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestLangselectManual(QObject *parent = nullptr) : CoverageObject(parent) {}

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

    QTemporaryDir staticDir;
    QFileInfo staticFile;
};

class LangselectManualTest : public Controller
{
    Q_OBJECT
public:
    explicit LangselectManualTest(QObject *parent) : Controller(parent) {}

    C_ATTR(testSession, :Local :AutoArgs)
    void testSession(Context *c)
    {
        LangSelect::fromSession(c, QStringLiteral("lang"));
        c->res()->setBody(c->locale().bcp47Name());
    }

    C_ATTR(pathBase, :Chained("/") :PathPart("langselect/manual/test") :CaptureArgs(1))
    void pathBase(Context *c, const QString &locale)
    {
        LangSelect::fromPath(c, locale);
    }

    C_ATTR(testPath, :Chained("pathBase") :PathPart("testPath") :Args(0))
    void testPath(Context *c)
    {
        c->res()->setBody(c->locale().bcp47Name());
    }

    C_ATTR(testUrlQuery, :Local :AutoArgs)
    void testUrlQuery(Context *c)
    {
        if (LangSelect::fromUrlQuery(c)) {
            c->res()->setBody(c->locale().bcp47Name());
        }
    }
};

void TestLangselectManual::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestLangselectManual::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new Session(app);
    auto plugin = new LangSelect(app);
    plugin->setSupportedLocales({
                                    QLocale(QLocale::German),
                                    QLocale(QLocale::Portuguese)
                                });
    plugin->setFallbackLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    plugin->setQueryKey(QStringLiteral("locale"));

    new LangselectManualTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestLangselectManual::cleanupTestCase()
{
    delete m_engine;
}

void TestLangselectManual::doTest()
{
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(int, status);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    const QVariantMap result = m_engine->createRequest(QStringLiteral("GET"),
                                                       urlAux.path(),
                                                       urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                       headers,
                                                       nullptr);


    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), status);
    if (status == 200) {
        QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
    } else if (status == 307) {
        QCOMPARE(result.value(QStringLiteral("headers")).value<Headers>().header(QStringLiteral("Location")).toUtf8(), output);
    }
}

void TestLangselectManual::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    Headers headers;

    QTest::newRow("session-00") << QStringLiteral("/langselect/manual/test/testSession") << headers << 200 << QByteArrayLiteral("en-GB");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-AT,de;q=0.8,en-GB;q=0.6,en-US;q=0.4,en;q=0.2"));
    QTest::newRow("session-01") << QStringLiteral("/langselect/manual/test/testSession") << headers << 200 << QByteArrayLiteral("de");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("ru"));
    QTest::newRow("session-02") << QStringLiteral("/langselect/manual/test/testSession") << headers << 200 << QByteArrayLiteral("en-GB");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-AT"));
    QTest::newRow("session-02") << QStringLiteral("/langselect/manual/test/testSession") << headers << 200 << QByteArrayLiteral("de");

    headers.removeHeader(QStringLiteral("Accept-Language"));
    QTest::newRow("path-00") << QStringLiteral("/langselect/manual/test/de/testPath") << headers << 200 << QByteArrayLiteral("de");
    QTest::newRow("path-01") << QStringLiteral("/langselect/manual/test/ru/testPath?foo=bar") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/en-gb/testPath?foo=bar");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-AT,de;q=0.8,en-GB;q=0.6,en-US;q=0.4,en;q=0.2"));
    QTest::newRow("path-02") << QStringLiteral("/langselect/manual/test/ru/testPath?foo=bar") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/de/testPath?foo=bar");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-CH"));
    QTest::newRow("path-02") << QStringLiteral("/langselect/manual/test/ru/testPath?foo=bar") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/de/testPath?foo=bar");
    QTest::newRow("path-00") << QStringLiteral("/langselect/manual/test/en-gb/testPath") << headers << 200 << QByteArrayLiteral("en-GB");

    headers.removeHeader(QStringLiteral("Accept-Language"));
    QTest::newRow("query-00") << QStringLiteral("/langselect/manual/test/testUrlQuery") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/testUrlQuery?locale=en-gb");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-AT,de;q=0.8,en-GB;q=0.6,en-US;q=0.4,en;q=0.2"));
    QTest::newRow("query-01") << QStringLiteral("/langselect/manual/test/testUrlQuery") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/testUrlQuery?locale=de");
    QTest::newRow("query-02") << QStringLiteral("/langselect/manual/test/testUrlQuery?foo=bar") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/testUrlQuery?foo=bar&locale=de");
    QTest::newRow("query-03") << QStringLiteral("/langselect/manual/test/testUrlQuery?locale=ru") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/testUrlQuery?locale=de");
    headers.removeHeader(QStringLiteral("Accept-Language"));
    QTest::newRow("query-04") << QStringLiteral("/langselect/manual/test/testUrlQuery?locale=ru") << headers << 307 << QByteArrayLiteral("http://127.0.0.1/langselect/manual/test/testUrlQuery?locale=en-gb");
    QTest::newRow("query-05") << QStringLiteral("/langselect/manual/test/testUrlQuery?locale=pt") << headers << 200 << QByteArrayLiteral("pt");

}

QTEST_MAIN(TestLangselectManual)

#include "testlangselectmanual.moc"

#endif // TESTLANGSELECT_H
