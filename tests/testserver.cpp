#ifndef TESTSERVER_H
#define TESTSERVER_H

#include "coverageobject.h"

#include <server/server.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace Cutelyst;

class TestServer : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestServer(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void testSetIni();
    void testSetJson();

private:
    QTemporaryDir m_tmpDir;
    QVariantMap m_expectedConfig;

    void writeIniFile(const QString &fileName, const QMap<QString, QVariantMap> &data);
    void writeJsonFile(const QString &fileName, const QJsonObject &data);
};

void TestServer::writeIniFile(const QString &fileName, const QMap<QString, QVariantMap> &data)
{
    QSettings ini{m_tmpDir.filePath(fileName), QSettings::IniFormat};
    QCOMPARE(ini.status(), QSettings::NoError);
    QVERIFY(ini.isWritable());

    for (auto i = data.cbegin(), iend = data.cend(); i != iend; ++i) {
        ini.beginGroup(i.key());
        const QVariantMap map = i.value();
        for (auto j = map.cbegin(), jend = map.cend(); j != jend; ++j) {
            ini.setValue(j.key(), j.value());
        }
        ini.endGroup();
    }

    ini.sync();
    QCOMPARE(ini.status(), QSettings::NoError);
}

void TestServer::writeJsonFile(const QString &fileName, const QJsonObject &data)
{
    QFile f{m_tmpDir.filePath(fileName)};
    QVERIFY(f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text));

    const QJsonDocument json{data};

    QVERIFY(f.write(json.toJson(QJsonDocument::Indented)) > 0);
}

void TestServer::initTestCase()
{
    QVERIFY(m_tmpDir.isValid());

    writeIniFile(u"file1.ini"_qs,
                 {{u"Cutelyst"_qs, {{u"hello"_qs, u"world1"_qs}, {u"onlyin1"_qs, u"hello"_qs}}},
                  {u"Testsection1"_qs, {{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}}}});

    writeIniFile(u"file2.ini"_qs,
                 {{u"Cutelyst"_qs, {{u"hello"_qs, u"world2"_qs}, {u"onlyin2"_qs, u"world"_qs}}},
                  {u"Testsection2"_qs, {{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}}});

    writeJsonFile(
        u"file1.json"_qs,
        QJsonObject{{{u"Cutelyst"_qs,
                      QJsonObject{{u"hello"_qs, u"world1"_qs}, {u"onlyin1"_qs, u"hello"_qs}}},
                     {u"Testsection1"_qs,
                      QJsonObject{{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}}}}});

    writeJsonFile(
        u"file2.json"_qs,
        QJsonObject{{{u"Cutelyst"_qs,
                      QJsonObject{{u"hello"_qs, u"world2"_qs}, {u"onlyin2"_qs, u"world"_qs}}},
                     {u"Testsection2"_qs,
                      QJsonObject{{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}}}});

    m_expectedConfig = {
        {u"Cutelyst"_qs,
         QVariantMap{{u"hello"_qs, u"world2"_qs},
                     {u"onlyin1"_qs, u"hello"_qs},
                     {u"onlyin2"_qs, u"world"_qs}}},
        {u"Testsection1"_qs, QVariantMap{{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}}},
        {u"Testsection2"_qs, QVariantMap{{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}}};
}

void TestServer::testSetIni()
{
    Server server;
    server.setIni({m_tmpDir.filePath(u"file1.ini"_qs), m_tmpDir.filePath(u"file2.ini"_qs)});
    QCOMPARE(server.config(), m_expectedConfig);
}

void TestServer::testSetJson()
{
    Server server;
    server.setJson({m_tmpDir.filePath(u"file1.json"_qs), m_tmpDir.filePath(u"file2.json"_qs)});
    QCOMPARE(server.config(), m_expectedConfig);
}

QTEST_MAIN(TestServer)

#include "testserver.moc"

#endif // TESTSERVER_H
