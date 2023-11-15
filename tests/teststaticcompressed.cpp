#ifndef TESTSTATICCOMPRESSED_H
#define TESTSTATICCOMPRESSED_H

#include "coverageobject.h"

#include <Cutelyst/Plugins/StaticCompressed/staticcompressed.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

using namespace Cutelyst;
// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
class TestStaticCompressed : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestStaticCompressed(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testFileNotFound();
    void testOnTheFlyCompression();
    void testOnTheFlyCompression_data();
    void testPreCompressed();
    void testPreCompressed_data();
    void testLastModifiedSince();

private:
    TestEngine *m_engine{nullptr};
    QTemporaryDir m_dataDir;
    static const QStringList types;
    static const QByteArrayList encoding;

    TestEngine *getEngine();

    TestEngine::TestResponse getFile(const QString &path, const Headers &headers = {});
    bool writeTestFile(const QString &name);
};

const QStringList TestStaticCompressed::types{u"js"_qs,
                                              u"css"_qs,
                                              u"min.js"_qs,
                                              u"min.css"_qs,
                                              u"js.map"_qs,
                                              u"css.map"_qs,
                                              u"min.js.map"_qs,
                                              u"min.css.map"_qs};

const QByteArrayList TestStaticCompressed::encoding{
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    "br",
#endif
    "deflate",
    "gzip"};

void TestStaticCompressed::initTestCase()
{
    QVERIFY(m_dataDir.isValid());
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestStaticCompressed::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, {});

    auto plug = new StaticCompressed(app);
    plug->setIncludePaths({m_dataDir.path()});

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

TestEngine::TestResponse TestStaticCompressed::getFile(const QString &path, const Headers &headers)
{
    const Headers hdrs =
        headers.data().empty() ? Headers({{"Accept-Encoding", "gzip, defalte, br"}}) : headers;
    return m_engine->createRequest("GET", path, {}, headers, nullptr);
}

bool TestStaticCompressed::writeTestFile(const QString &name)
{
    QFile f(m_dataDir.filePath(name));
    if (!f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text)) {
        qCritical() << "Failed to open test file for writing:" << f.errorString();
        return false;
    }

    {
        QTextStream out(&f);
        out << "Nisi et et fugiat debitis impedit. Sint officiis optio quas beatae facilis "
               "laudantium accusantium voluptatem. Fugit et asperiores quia accusantium est. "
               "Rerum sunt est temporibus. Sit esse dolore quaerat vero et. Dicta quia "
               "assumenda ad beatae ut.\n\n";
        out << "Ab est rerum ut et qui aut. Voluptates voluptatem laborum qui earum maxime ea "
               "consequuntur pariatur. Ipsam reprehenderit aut ut sapiente qui qui.\n\n";
        out << "Nemo sequi voluptatem non vero eum cumque aliquid at. Aut alias et qui adipisci. "
               "Sit consectetur dolorum error et aut aut soluta veniam. Sit ex cum voluptatem eos "
               "ullam cumque. At ut et est ab. Id ut iste voluptatibus est aliquam deleniti "
               "quia.\n";
    }

    f.close();
    return true;
}

void TestStaticCompressed::cleanupTestCase()
{
    delete m_engine;
    m_engine = nullptr;
}

void TestStaticCompressed::testFileNotFound()
{
    const auto resp = getFile(u"/filenotavailable.js"_qs);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

void TestStaticCompressed::testOnTheFlyCompression()
{
    QFETCH(QString, fileName);
    QFETCH(QByteArray, encoding);

    QVERIFY(writeTestFile(fileName));

    const auto resp = getFile(u'/' + fileName, {{"Accept-Encoding", encoding}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), encoding);
}

void TestStaticCompressed::testOnTheFlyCompression_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QByteArray>("encoding");

    for (const QString &type : TestStaticCompressed::types) {
        for (const QByteArray &enc : TestStaticCompressed::encoding) {
            const QByteArray testName = type.toLatin1() + "-" + enc;
            const QString fileName    = u"onTheFly-"_qs + QString::fromLatin1(enc) + u"."_qs + type;
            QTest::newRow(testName.data()) << fileName << enc;
        }
    }
}

void TestStaticCompressed::testPreCompressed()
{
    QFETCH(QString, fileName);
    QFETCH(QByteArray, encoding);

    QVERIFY(writeTestFile(fileName));
    QString encodedFileName = fileName;
    if (encoding == "br") {
        encodedFileName += u".br"_qs;
    } else if (encoding == "gzip") {
        encodedFileName += u".gz"_qs;
    } else if (encoding == "deflate") {
        encodedFileName += u".deflate"_qs;
    } else {
        QVERIFY2(false, "invalid encoding");
    }
    QVERIFY(writeTestFile(encodedFileName));

    const auto resp = getFile(u'/' + fileName, {{"Accept-Encoding", encoding}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), encoding);
}

void TestStaticCompressed::testPreCompressed_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QByteArray>("encoding");

    for (const QString &type : TestStaticCompressed::types) {
        for (const QByteArray &enc : TestStaticCompressed::encoding) {
            const QByteArray testName = type.toLatin1() + "-" + enc;
            const QString fileName =
                u"preCompressed-"_qs + QString::fromLatin1(enc) + u"."_qs + type;
            QTest::newRow(testName.data()) << fileName << enc;
        }
    }
}

void TestStaticCompressed::testLastModifiedSince()
{
    QVERIFY(writeTestFile(u"lastmodified.js"_qs));
    QFileInfo fi(m_dataDir.filePath(u"lastmodified.js"_qs));
    Headers hdrs{
        {"Accept-Encoding", "br"},
        {"If-Modified-Since",
         fi.lastModified().toUTC().toString(u"ddd, dd MMM yyyy hh:mm:ss 'GMT'").toLatin1()}};
    const auto resp = getFile(u"/lastmodified.js"_qs, hdrs);

    QCOMPARE(resp.statusCode, Response::NotModified);
}

QTEST_MAIN(TestStaticCompressed)

// NOLINTEND(cppcoreguidelines-avoid-do-while)

#include "teststaticcompressed.moc"

#endif // TESTSTATICCOMPRESSED_H
