#ifndef TESTSTATICSIMPLE_H
#define TESTSTATICSIMPLE_H

#include "coverageobject.h"

#include <Cutelyst/Plugins/StaticSimple/staticsimple.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;
// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
class TestStaticSimple : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestStaticSimple(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testFileNotFound();
    void testGetFileFromRoot();
    void testGetFileFromSubdirs();
    void testFileNotFoundFomForcedDirs();
    void testGetFileFromForcedDirs();
    void testLastModifiedSince();
    void testGetFileFromForcedDirsOnly();
    void testFileNotFoundFromForcedDirsOnly();
    void testFileNotInForcedDirsOnly();
    void testControllerPath();

private:
    TestEngine *m_engine{nullptr};
    TestEngine *m_engineDirsOnly{nullptr};
    QTemporaryDir m_dataDir;

    TestEngine *getEngine(bool serveDirsOnly = false);

    TestEngine::TestResponse getFile(const QString &path, const Headers &headers = {});
    TestEngine::TestResponse getForcedFile(const QString &path, const Headers &headers = {});
    bool writeTestFile(const QString &name);
};

void TestStaticSimple::initTestCase()
{
    QVERIFY(m_dataDir.isValid());
    m_engine = getEngine();
    QVERIFY(m_engine);
    m_engineDirsOnly = getEngine(true);
    QVERIFY(m_engineDirsOnly);
}

TestEngine *TestStaticSimple::getEngine(bool serveDirsOnly)
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, {});

    auto plug = new StaticSimple(app);
    plug->setIncludePaths({m_dataDir.path()});
    plug->setDirs({u"forced"_s});
    plug->setServeDirsOnly(serveDirsOnly);

    if (!engine->init()) {
        return nullptr;
    }

    return engine;
}

TestEngine::TestResponse TestStaticSimple::getFile(const QString &path, const Headers &headers)
{
    return m_engine->createRequest("GET", path, {}, headers, nullptr);
}

TestEngine::TestResponse TestStaticSimple::getForcedFile(const QString &path,
                                                         const Headers &headers)
{
    return m_engineDirsOnly->createRequest("GET", path, {}, headers, nullptr);
}

bool TestStaticSimple::writeTestFile(const QString &name)
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

    return true;
}

void TestStaticSimple::cleanupTestCase()
{
    delete m_engine;
    m_engine = nullptr;
}

void TestStaticSimple::testFileNotFound()
{
    const auto resp = getFile(u"/filenotavailable.js"_s);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

void TestStaticSimple::testGetFileFromRoot()
{
    QVERIFY(writeTestFile(u"mytestfile.css"_s));
    const auto resp = getFile(u"/mytestfile.css"_s);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testGetFileFromSubdirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"static/css"_s));
    QVERIFY(writeTestFile(u"static/css/mytestfile.css"_s));
    const auto resp = getFile(u"/static/css/mytestfile.css"_s);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testFileNotFoundFomForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_s));
    const auto resp = getFile(u"/forced/css/notavailable.css"_s);
    QCOMPARE(resp.statusCode, Response::NotFound);
}

void TestStaticSimple::testGetFileFromForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_s));
    QVERIFY(writeTestFile(u"forced/css/mytestfile.css"_s));
    const auto resp = getFile(u"/forced/css/mytestfile.css"_s);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testLastModifiedSince()
{
    QVERIFY(writeTestFile(u"lastmodified.js"_s));
    QFileInfo fi(m_dataDir.filePath(u"lastmodified.js"_s));
    const auto resp = getFile(
        u"/lastmodified.js"_s,
        {{"If-Modified-Since",
          fi.lastModified().toUTC().toString(u"ddd, dd MMM yyyy hh:mm:ss 'GMT'").toLatin1()}});
    QCOMPARE(resp.statusCode, Response::NotModified);
}

/**
 * @internal
 * Test for a file that is below a path that is set to TestStaticSimple::setDirs()
 * and where TestStaticSimple::setServeDirsOnly() is set to @c true.
 */
void TestStaticSimple::testGetFileFromForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_s));
    QVERIFY(writeTestFile(u"forced/css/myforcedtestfile.css"_s));
    const auto resp = getForcedFile(u"/forced/css/myforcedtestfile.css"_s);
    QCOMPARE(resp.statusCode, Response::OK);
}

/**
 * @internal
 * Test for not available file in a directory that is below the paths set to
 * TestStaticSimple::setDirs() and where TestStaticSimple::setServeDirsOnly() is set to @c true.
 */
void TestStaticSimple::testFileNotFoundFromForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced"_s));
    const auto resp = getForcedFile(u"/forced/notavailable.js"_s);
    QCOMPARE(resp.statusCode, Response::NotFound);
}

/**
 * @internal
 * Test for not available file in a directory that is not below a path set to
 * TestStaticSimple::setDirs() and where TestStaticSimple::setServeDirsOnly() is set to @c true.
 */
void TestStaticSimple::testFileNotInForcedDirsOnly()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced"_s));
    const auto resp = getForcedFile(u"/notinforced.js"_s);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

/**
 * @internal
 * Test if we still reach normal controllers
 */
void TestStaticSimple::testControllerPath()
{
    const auto resp = getFile(u"/test/controller/hello"_s);
    QCOMPARE(resp.statusCode, Response::OK);
}

QTEST_MAIN(TestStaticSimple)

// NOLINTEND(cppcoreguidelines-avoid-do-while)

#include "teststaticsimple.moc"

#endif // TESTSTATICSIMPLE_H
