#ifndef TESTLANGSELECT_H
#define TESTLANGSELECT_H

#include <QTest>
#include <QObject>
#include <QUrlQuery>
#include <QLocale>
#include <QNetworkCookie>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>

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
    LangselectTest(QObject *parent) : Controller(parent) {}

    C_ATTR(base, :Chained("/") :PathPart("langselect") :CaptureArgs(1))
    void base(Context *c, const QString &lang) {
        Q_UNUSED(lang);
    }

    C_ATTR(testLang, :Chained("base") :PathPart("testLang") :AutoArgs)
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
    auto plugin = new LangSelect(app);
    plugin->setPathIndex(1);
    plugin->setCookieName(QStringLiteral("lang"));
    plugin->setQueryKey(QStringLiteral("lang"));
    plugin->setSupportedLocales({QLocale(QLocale::German), QLocale(QLocale::Portuguese, QLocale::Brazil), QLocale(QLocale::English, QLocale::UnitedKingdom)});
    plugin->setFallbackLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
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
    QByteArray body;

    QUrl urlAux(url.mid(1));

    const QVariantMap result = m_engine->createRequest(QStringLiteral("GET"),
                                                       urlAux.path(),
                                                       urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                       headers,
                                                       &body);

    const auto statusCode = result.value(QStringLiteral("statusCode")).value<quint16>();

    if (statusCode == 307) {
        QByteArray expectedRedirect;
        if (!output.isEmpty()) {
            expectedRedirect = QByteArrayLiteral("http://127.0.0.1/langselect/") + output + QByteArrayLiteral("/testLang");
            if (urlAux.hasQuery()) {
                expectedRedirect += QByteArrayLiteral("?") + urlAux.query(QUrl::FullyEncoded).toLatin1();
            }
        }
        QCOMPARE(result.value(QStringLiteral("headers")).value<Headers>().header(QStringLiteral("Location")).toLatin1(), expectedRedirect);
    } else {
        QCOMPARE(statusCode, static_cast<quint16>(status));
    }
}

void TestLangselect::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    Headers headers;

    QUrlQuery query;

    QTest::newRow("set-by-query-01") << QStringLiteral("/langselect/testLang?lang=de") << headers << 307 << QByteArrayLiteral("de");
    QTest::newRow("set-by-query-02") << QStringLiteral("/langselect/testLang?lang=pt-br") << headers << 307 << QByteArrayLiteral("pt");
    QTest::newRow("set-by-query-03") << QStringLiteral("/langselect/testLang?lang=pt") << headers << 307 << QByteArrayLiteral("pt");
    QTest::newRow("set-by-query-04") << QStringLiteral("/langselect/testLang?lang=pt-PT") << headers << 307 << QByteArrayLiteral("en-GB");

    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-DE,de;q=0.8,en-GB;q=0.6,en-US;q=0.4,en;q=0.2"));
    QTest::newRow("set-by-query-05") << QStringLiteral("/langselect/testLang?lang=ru") << headers << 307 << QByteArrayLiteral("de");

    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(QNetworkCookie(QByteArrayLiteral("lang"), QByteArrayLiteral("de")).toRawForm()));
    QTest::newRow("set-by-cookie-01") << QStringLiteral("/langselect/testLang?lang=pt") << headers << 307 << QByteArrayLiteral("de");

    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(QNetworkCookie(QByteArrayLiteral("lang"), QByteArrayLiteral("ru")).toRawForm()));
    QTest::newRow("set-by-cookie-02") << QStringLiteral("/langselect/testLang?lang=pt") << headers << 307 << QByteArrayLiteral("pt");

    headers.removeHeader(QStringLiteral("Accept-Language"));
    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(QNetworkCookie(QByteArrayLiteral("lang"), QByteArrayLiteral("ru")).toRawForm()));
    QTest::newRow("set-by-cookie-03") << QStringLiteral("/langselect/testLang") << headers << 307 << QByteArrayLiteral("en-GB");

    headers.removeHeader(QStringLiteral("Cookie"));
    QTest::newRow("set-by-path-01") << QStringLiteral("/langselect/de/testLang") << headers << 200 << QByteArray();
    QTest::newRow("set-by-path-02") << QStringLiteral("/langselect/ru/testLang") << headers << 307 << QByteArray("en-GB");
    headers.setHeader(QStringLiteral("Accept-Language"), QStringLiteral("de-DE,de;q=0.8,en-GB;q=0.6,en-US;q=0.4,en;q=0.2"));
    QTest::newRow("set-by-path-03") << QStringLiteral("/langselect/ru/testLang") << headers << 307 << QByteArray("de");

    QTest::newRow("test-static-file") << QStringLiteral("/") + staticFile.fileName() << headers << 200 << QByteArray();

}

QTEST_MAIN(TestLangselect)

#include "testlangselect.moc"

#endif // TESTLANGSELECT_H
