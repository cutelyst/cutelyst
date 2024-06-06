#ifndef TESTSTATICCOMPRESSED_H
#define TESTSTATICCOMPRESSED_H

#include "coverageobject.h"

#include <Cutelyst/Plugins/StaticCompressed/staticcompressed.h>

#include <QDir>
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
    void testGetFileFromSubdirs();
    void testFileNotFoundFromForcedDirs();
    void testGetFileFromForcedDirs();
    void testGetFileFromForcedDirsOnly();
    void testFileNotFoundFromForcedDirsOnly();
    void testFileNotInForcedDirsOnly();
    void testControllerPath();

private:
    TestEngine *m_engine{nullptr};
    TestEngine *m_engineDirsOnly{nullptr};
    QTemporaryDir m_dataDir;
    static const QStringList types;
    static const QByteArrayList encoding;

    TestEngine *getEngine(bool serveDirsOnly = false);

    TestEngine::TestResponse getFile(const QString &path, const Headers &headers = {});
    TestEngine::TestResponse getForcedFile(const QString &path, const Headers &headers = {});
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
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
    "zstd",
#endif
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
    m_engineDirsOnly = getEngine(true);
    QVERIFY(m_engineDirsOnly);
}

TestEngine *TestStaticCompressed::getEngine(bool serveDirsOnly)
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, {});

    const QVariantMap defaultConfig{{u"zlib_compression_level"_qs, 1},
                                    {u"brotli_quality_level"_qs, 0},
                                    {u"use_zopfli"_qs, true},
                                    {u"zopfli_iterations"_qs, 5},
                                    {u"zstd_compression_level"_qs, 1}};

    auto plug = new StaticCompressed(app, defaultConfig);
    plug->setIncludePaths({m_dataDir.path()});
    plug->setDirs({u"forced"_qs});
    plug->setServeDirsOnly(serveDirsOnly);

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

TestEngine::TestResponse TestStaticCompressed::getFile(const QString &path, const Headers &headers)
{
    const Headers hdrs = headers.data().empty()
                             ? Headers({{"Accept-Encoding", "gzip, deflate, br, zstd"}})
                             : headers;
    return m_engine->createRequest("GET", path, {}, headers, nullptr);
}

TestEngine::TestResponse TestStaticCompressed::getForcedFile(const QString &path,
                                                             const Headers &headers)
{
    const Headers hdrs = headers.data().empty()
                             ? Headers({{"Accept-Encoding", "gzip, deflate, br, zstd"}})
                             : headers;
    return m_engineDirsOnly->createRequest("GET", path, {}, headers, nullptr);
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
    delete m_engineDirsOnly;
    m_engineDirsOnly = nullptr;
}

/**
 * @internal
 * Test for files that are not available.
 */
void TestStaticCompressed::testFileNotFound()
{
    const auto resp = getFile(u"/filenotavailable.js"_qs);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

/**
 * @internal
 * Perform tests where files are compressed on the fly.
 */
void TestStaticCompressed::testOnTheFlyCompression()
{
    QFETCH(QString, fileName);
    QFETCH(QByteArray, encoding);

    QVERIFY(writeTestFile(fileName));

    const auto resp = getFile(u'/' + fileName, {{"Accept-Encoding", encoding}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), encoding);
}

/**
 * @internal
 * Provide test data for testing on the fly compression. Will test for all
 * supported encodings and by default supported file mime types and extensions.
 */
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

/**
 * @internal
 * Perform tests where pre-compressed files are available.
 */
void TestStaticCompressed::testPreCompressed()
{
    QFETCH(QString, fileName);
    QFETCH(QByteArray, encoding);

    QVERIFY(writeTestFile(fileName));
    QString encodedFileName = fileName;
    if (encoding == "zstd") {
        encodedFileName += u".zst"_qs;
    } else if (encoding == "br") {
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

/**
 * @internal
 * Provide test data for testing pre-compressed file serving. Will test for all
 * supported encodings and by default supported file mime types and extensions.
 */
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

/**
 * @internal
 * Test for a file where the last modified date has bot been changed.
 */
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

/**
 * @internal
 * Test to get a static file from a subdirectory.
 */
void TestStaticCompressed::testGetFileFromSubdirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"static/css"_qs));
    QVERIFY(writeTestFile(u"static/css/mytestfile.css"_qs));
    const auto resp = getFile(u"/static/css/mytestfile.css"_qs, {{"Accept-Encoding", "gzip"}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), "gzip"_qba);
}

/**
 * @internal
 * Test for not available file in a directory that is below the paths set to
 * StaticCompressed::setDirs() and where StaticCompressed::setServeDirsOnly() is set to @c false.
 */
void TestStaticCompressed::testFileNotFoundFromForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_qs));
    const auto resp = getFile(u"/forced/css/notavailable.css"_qs);
    QCOMPARE(resp.statusCode, Response::NotFound);
}

/**
 * @internal
 * Test for a file that is below a path that is set to StaticCompressed::setDirs()
 * and where StaticCompressed::setServeDirsOnly() is set to @c false.
 */
void TestStaticCompressed::testGetFileFromForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_qs));
    QVERIFY(writeTestFile(u"forced/css/mytestfile.css"_qs));
    const auto resp = getFile(u"/forced/css/mytestfile.css"_qs, {{"Accept-Encoding", "gzip"}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), "gzip"_qba);
}

/**
 * @internal
 * Test for a file that is below a path that is set to StaticCompressed::setDirs()
 * and where StaticCompressed::setServeDirsOnly() is set to @c true.
 */
void TestStaticCompressed::testGetFileFromForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_qs));
    QVERIFY(writeTestFile(u"forced/css/myforcedtestfile.css"_qs));
    const auto resp =
        getForcedFile(u"/forced/css/myforcedtestfile.css"_qs, {{"Accept-Encoding", "gzip"}});
    QCOMPARE(resp.statusCode, Response::OK);
    QCOMPARE(resp.headers.header("Content-Encoding"), "gzip"_qba);
}

/**
 * @internal
 * Test for not available file in a directory that is below the paths set to
 * StaticCompressed::setDirs() and where StaticCompressed::setServeDirsOnly() is set to @c true.
 */
void TestStaticCompressed::testFileNotFoundFromForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced"_qs));
    const auto resp = getForcedFile(u"/forced/notavailable.js"_qs);
    QCOMPARE(resp.statusCode, Response::NotFound);
}

/**
 * @internal
 * Test for not available file in a directory that is not below a path set to
 * StaticCompressed::setDirs() and where StaticCompressed::setServeDirsOnly() is set to @c true.
 */
void TestStaticCompressed::testFileNotInForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced"_qs));
    const auto resp = getForcedFile(u"/notinforced.js"_qs);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

/**
 * @internal
 * Test if we still reach normal controllers
 */
void TestStaticCompressed::testControllerPath()
{
    const auto resp = getFile(u"/test/controller/hello"_qs);
    QCOMPARE(resp.statusCode, Response::OK);
}

QTEST_MAIN(TestStaticCompressed)

// NOLINTEND(cppcoreguidelines-avoid-do-while)

#include "teststaticcompressed.moc"

#endif // TESTSTATICCOMPRESSED_H
