#ifndef TESTLANGSELECT_H
#define TESTLANGSELECT_H

#include <QTest>
#include <QObject>
#include <QLocale>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QNetworkCookie>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/StaticSimple/StaticSimple>
#include <Cutelyst/Plugins/Utils/LangSelect/LangSelect>

using namespace Cutelyst;

class TestLangselect : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestLangselect(QObject *parent = nullptr) : CoverageObject(parent) {
        auto file = new QTemporaryFile(staticDir.path() + QLatin1String("/staticfile"), this);
        file->open();
        staticFile.setFile(file->fileName());
    }

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

class LangselectTest : public Controller
{
    Q_OBJECT
public:
    explicit LangselectTest(QObject *parent) : Controller(parent) {}

    C_ATTR(testLang, :Local :AutoArgs)
    void testLang(Context *c) {
        c->res()->setBody(c->locale().bcp47Name());
    }
};

void TestLangselect::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestLangselect::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new Session(app);
    auto statDir = new StaticSimple(app);
    statDir->setIncludePaths({staticDir.path()});
    auto plugin = new LangSelect(app, LangSelect::Cookie);
    plugin->setSupportedLocales({
                                    QLocale(QLocale::German),
                                    QLocale(QLocale::Portuguese)
                                });
    plugin->setFallbackLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    plugin->setCookieName(QStringLiteral("lang"));
    new LangselectTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestLangselect::cleanupTestCase()
{
    delete m_engine;
}

void TestLangselect::doTest()
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
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestLangselect::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    Headers headers;

    QTest::newRow("test-auto-cookie-00") << QStringLiteral("/langselect/test/testLang") << headers << 200 << QByteArrayLiteral("en-GB");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-AT"));
    QTest::newRow("test-auto-cookie-01") << QStringLiteral("/langselect/test/testLang") << headers << 200 << QByteArrayLiteral("de");
    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(QNetworkCookie(QByteArrayLiteral("lang"), QByteArrayLiteral("pt")).toRawForm()));
    QTest::newRow("test-auto-cookie-02") << QStringLiteral("/langselect/test/testLang") << headers << 200 << QByteArrayLiteral("pt");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("ru"));
    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(QNetworkCookie(QByteArrayLiteral("lang"), QByteArrayLiteral("dk")).toRawForm()));
    QTest::newRow("test-auto-cookie-03") << QStringLiteral("/langselect/test/testLang") << headers << 200 << QByteArrayLiteral("en-GB");

    QTest::newRow("test-auto-static-file") << QStringLiteral("/") + staticFile.fileName() << headers << 200 << QByteArray();

}

QTEST_MAIN(TestLangselect)

#include "testlangselect.moc"

#endif // TESTLANGSELECT_H
