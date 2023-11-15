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

private:
    TestEngine *m_engine{nullptr};
    QTemporaryDir m_dataDir;

    TestEngine *getEngine();

    TestEngine::TestResponse getFile(const QString &path, const Headers &headers = {});
    bool writeTestFile(const QString &name);
};

void TestStaticSimple::initTestCase()
{
    QVERIFY(m_dataDir.isValid());
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestStaticSimple::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, {});

    auto plug = new StaticSimple(app);
    plug->setIncludePaths({m_dataDir.path()});
    plug->setDirs({u"forced"_qs});

    if (!engine->init()) {
        return nullptr;
    }

    return engine;
}

TestEngine::TestResponse TestStaticSimple::getFile(const QString &path, const Headers &headers)
{
    return m_engine->createRequest("GET", path, {}, headers, nullptr);
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
    const auto resp = getFile(u"/filenotavailable.js"_qs);
    QVERIFY(resp.statusCode >= Response::BadRequest);
}

void TestStaticSimple::testGetFileFromRoot()
{
    QVERIFY(writeTestFile(u"mytestfile.css"_qs));
    const auto resp = getFile(u"/mytestfile.css"_qs);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testGetFileFromSubdirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"static/css"_qs));
    QVERIFY(writeTestFile(u"static/css/mytestfile.css"_qs));
    const auto resp = getFile(u"/static/css/mytestfile.css"_qs);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testFileNotFoundFomForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_qs));
    const auto resp = getFile(u"/forced/css/notavailable.css"_qs);
    QCOMPARE(resp.statusCode, Response::NotFound);
}

void TestStaticSimple::testGetFileFromForcedDirs()
{
    QDir dataDir(m_dataDir.path());
    QVERIFY(dataDir.mkpath(u"forced/css"_qs));
    QVERIFY(writeTestFile(u"forced/css/mytestfile.css"_qs));
    const auto resp = getFile(u"/forced/css/mytestfile.css"_qs);
    QCOMPARE(resp.statusCode, Response::OK);
}

void TestStaticSimple::testLastModifiedSince()
{
    QVERIFY(writeTestFile(u"lastmodified.js"_qs));
    QFileInfo fi(m_dataDir.filePath(u"lastmodified.js"_qs));
    const auto resp = getFile(
        u"/lastmodified.js"_qs,
        {{"If-Modified-Since",
          fi.lastModified().toUTC().toString(u"ddd, dd MMM yyyy hh:mm:ss 'GMT'").toLatin1()}});
    QCOMPARE(resp.statusCode, Response::NotModified);
}

QTEST_MAIN(TestStaticSimple)

// NOLINTEND(cppcoreguidelines-avoid-do-while)

#include "teststaticsimple.moc"

#endif // TESTSTATICSIMPLE_H
