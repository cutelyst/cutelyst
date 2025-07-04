#ifndef TESTSERVER_H
#define TESTSERVER_H

#include "coverageobject.h"

#include <Cutelyst/Server/server.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

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
    void testSetServerConfigFromFile();

private:
    QTemporaryDir m_tmpDir;
    QVariantMap m_expectedIniConfig;
    QVariantMap m_expectedJsonConfig;
    QVariantMap m_expectedServerConfig;

    void writeIniFile(const QString &fileName, const QMap<QString, QVariantMap> &data);
    void writeJsonFile(const QString &fileName, const QJsonObject &data);
};

void TestServer::writeIniFile(const QString &fileName, const QMap<QString, QVariantMap> &data)
{
    QSettings ini{fileName, QSettings::IniFormat};
    QCOMPARE(ini.status(), QSettings::NoError);
    QVERIFY(ini.isWritable());

    for (const auto &[key, map] : data.asKeyValueRange()) {
        ini.beginGroup(key);
        for (const auto &[mapKey, mapValue] : map.asKeyValueRange()) {
            ini.setValue(mapKey, mapValue);
        }
        ini.endGroup();
    }

    ini.sync();
    QCOMPARE(ini.status(), QSettings::NoError);
}

void TestServer::writeJsonFile(const QString &fileName, const QJsonObject &data)
{
    QFile f{fileName};
    QVERIFY(f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text));

    const QJsonDocument json{data};

    QVERIFY(f.write(json.toJson(QJsonDocument::Indented)) > 0);
}

void TestServer::initTestCase()
{
    QVERIFY(m_tmpDir.isValid());

    // load this from the server/ini value in file1.ini
    const QString file3Ini = m_tmpDir.filePath(u"file3.ini"_s);
    writeIniFile(file3Ini,
                 {{u"Testsection1"_s, {{u"john"_s, u"doe"_s}}},
                  {u"Testsection3"_s, {{u"hello"_s, u"world"_s}}}});

    const QString file1Ini = m_tmpDir.filePath(u"file1.ini"_s);
    writeIniFile(file1Ini,
                 {{u"Cutelyst"_s, {{u"hello"_s, u"world1"_s}, {u"onlyin1"_s, u"hello"_s}}},
                  {u"Testsection1"_s, {{u"foo"_s, u"bar"_s}, {u"hello"_s, u"world"_s}}},
                  {u"server"_s, {{u"ini"_s, file3Ini}}}});

    const QString file2Ini = m_tmpDir.filePath(u"file2.ini"_s);
    writeIniFile(file2Ini,
                 {{u"Cutelyst"_s, {{u"hello"_s, u"world2"_s}, {u"onlyin2"_s, u"world"_s}}},
                  {u"Testsection2"_s, {{u"fu"_s, u"baz"_s}, {u"hello"_s, u"world"_s}}}});

    m_expectedIniConfig = {
        {u"Cutelyst"_s,
         QVariantMap{
             {u"hello"_s, u"world2"_s}, {u"onlyin1"_s, u"hello"_s}, {u"onlyin2"_s, u"world"_s}}},
        {u"Testsection1"_s,
         QVariantMap{{u"foo"_s, u"bar"_s}, {u"hello"_s, u"world"_s}, {u"john"_s, u"doe"_s}}},
        {u"Testsection2"_s, QVariantMap{{u"fu"_s, u"baz"_s}, {u"hello"_s, u"world"_s}}},
        {u"Testsection3"_s, QVariantMap{{u"hello"_s, u"world"_s}}},
        {u"server"_s, QVariantMap{{u"ini"_s, file3Ini}}}};

    // load this from the server/json value in file1.json
    const QString file3Json = m_tmpDir.filePath(u"file3.json"_s);
    writeJsonFile(file3Json,
                  QJsonObject{{u"Testsection1"_s, QJsonObject{{u"john"_s, u"doe"_s}}},
                              {u"Testsection3"_s, QJsonObject{{u"hello"_s, u"world"_s}}}});

    const QString file1Json = m_tmpDir.filePath(u"file1.json"_s);
    writeJsonFile(
        file1Json,
        QJsonObject{
            {{u"Cutelyst"_s, QJsonObject{{u"hello"_s, u"world1"_s}, {u"onlyin1"_s, u"hello"_s}}},
             {u"Testsection1"_s, QJsonObject{{u"foo"_s, u"bar"_s}, {u"hello"_s, u"world"_s}}},
             {u"server"_s, QJsonObject{{u"json"_s, file3Json}}}}});

    const QString file2Json = m_tmpDir.filePath(u"file2.json"_s);
    writeJsonFile(
        file2Json,
        QJsonObject{
            {{u"Cutelyst"_s, QJsonObject{{u"hello"_s, u"world2"_s}, {u"onlyin2"_s, u"world"_s}}},
             {u"Testsection2"_s, QJsonObject{{u"fu"_s, u"baz"_s}, {u"hello"_s, u"world"_s}}}}});

    m_expectedJsonConfig = {
        {u"Cutelyst"_s,
         QVariantMap{
             {u"hello"_s, u"world2"_s}, {u"onlyin1"_s, u"hello"_s}, {u"onlyin2"_s, u"world"_s}}},
        {u"Testsection1"_s,
         QVariantMap{{u"foo"_s, u"bar"_s}, {u"hello"_s, u"world"_s}, {u"john"_s, u"doe"_s}}},
        {u"Testsection2"_s, QVariantMap{{u"fu"_s, u"baz"_s}, {u"hello"_s, u"world"_s}}},
        {u"Testsection3"_s, QVariantMap{{u"hello"_s, u"world"_s}}},
        {u"server"_s, QVariantMap{{u"json"_s, file3Json}}}};

    const QString serverConfig1Ini = m_tmpDir.filePath(u"serverConfig1.ini"_s);
    writeIniFile(serverConfig1Ini,
                 {{u"Testsection1"_s, {{u"hello"_s, u"world"_s}}},
                  {u"server"_s,
                   {{u"threads"_s, 2},
                    {u"processes"_s, 3},
                    {u"chdir"_s, u"/path/to/chdir"_s},
                    {u"http_socket"_s, u"localhost:3000"_s},
                    {u"http2_socket"_s, u"localhost:3001"_s},
                    {u"http2_header_table_size"_s, 123},
                    {u"upgrade_h2c"_s, true},
                    {u"https_h2"_s, true},
                    {u"https_socket"_s, u"localhost:3002"_s},
                    {u"fastcgi_socket"_s, u"/path/to/socket"_s},
                    {u"socket_access"_s, u"ug"_s},
                    {u"socket_timeout"_s, 4321},
                    {u"chdir2"_s, u"/path/to/chdir2"_s},
                    {u"listen"_s, 111},
                    {u"socket_sndbuf"_s, 123},
                    {u"socket_rcvbuf"_s, 456}}}});

    const QString serverConfig2Ini = m_tmpDir.filePath(u"serverConfig2.ini"_s);
    writeIniFile(serverConfig2Ini,
                 {{u"Testsection2"_s, {{u"foo"_s, u"bar"_s}}},
                  {u"server"_s,
                   {{u"ini"_s, serverConfig1Ini},
                    {u"static_map"_s, u"/mountpoint1=/path/to/static1"_s},
                    {u"static_map2"_s, u"/mountpoint2=/path/to/static2"_s},
                    {u"master"_s, true},
                    {u"auto_reload"_s, true},
                    {u"touch_reload"_s, u"/path/to/file"_s},
                    {u"buffer_size"_s, 5432},
                    {u"post_buffering"_s, 456},
                    {u"post_buffering_bufsize"_s, 5000},
                    {u"tcp_nodelay"_s, true},
                    {u"so_keepalive"_s, true},
                    {u"websocket_max_size"_s, 2048}}}});

    const QString serverConfig3Ini = m_tmpDir.filePath(u"serverConfig3.ini"_s);
    writeIniFile(serverConfig3Ini,
                 {{u"Testsection3"_s, {{u"fu"_s, u"baz"_s}}},
                  {u"server"_s,
                   {{u"pidfile"_s, u"/path/to/pidfile1"_s},
                    {u"pidfile2"_s, u"/path/to/pidfile2"_s},
                    {u"uid"_s, u"user"_s},
                    {u"gid"_s, u"group"_s},
                    {u"no_initgroups"_s, true},
                    {u"chown_socket"_s, u"user:group"_s},
                    {u"umask"_s, u"0077"_s},
                    {u"cpu_affinity"_s, 1},
                    {u"reuse_port"_s, true},
                    {u"lazy"_s, true},
                    {u"using_frontend_proxy"_s, true}}}});

    m_expectedServerConfig = {{u"Testsection1"_s, QVariantMap{{u"hello"_s, u"world"_s}}},
                              {u"Testsection2"_s, QVariantMap{{u"foo"_s, u"bar"_s}}},
                              {u"Testsection3"_s, QVariantMap{{u"fu"_s, u"baz"_s}}},
                              {u"server"_s,
                               QVariantMap{{u"threads"_s, 2},
                                           {u"processes"_s, 3},
                                           {u"chdir"_s, u"/path/to/chdir"_s},
                                           {u"http_socket"_s, u"localhost:3000"_s},
                                           {u"http2_socket"_s, u"localhost:3001"_s},
                                           {u"http2_header_table_size"_s, 123},
                                           {u"upgrade_h2c"_s, true},
                                           {u"https_h2"_s, true},
                                           {u"https_socket"_s, u"localhost:3002"_s},
                                           {u"fastcgi_socket"_s, u"/path/to/socket"_s},
                                           {u"socket_access"_s, u"ug"_s},
                                           {u"socket_timeout"_s, 4321},
                                           {u"chdir2"_s, u"/path/to/chdir2"_s},
                                           {u"ini"_s, serverConfig1Ini},
                                           {u"static_map"_s, u"/mountpoint1=/path/to/static1"_s},
                                           {u"static_map2"_s, u"/mountpoint2=/path/to/static2"_s},
                                           {u"master"_s, true},
                                           {u"auto_reload"_s, true},
                                           {u"touch_reload"_s, u"/path/to/file"_s},
                                           {u"listen"_s, 111},
                                           {u"buffer_size"_s, 5432},
                                           {u"post_buffering"_s, 456},
                                           {u"post_buffering_bufsize"_s, 5000},
                                           {u"tcp_nodelay"_s, true},
                                           {u"so_keepalive"_s, true},
                                           {u"socket_sndbuf"_s, 123},
                                           {u"socket_rcvbuf"_s, 456},
                                           {u"websocket_max_size"_s, 2048},
                                           {u"pidfile"_s, u"/path/to/pidfile1"_s},
                                           {u"pidfile2"_s, u"/path/to/pidfile2"_s},
                                           {u"uid"_s, u"user"_s},
                                           {u"gid"_s, u"group"_s},
                                           {u"no_initgroups"_s, true},
                                           {u"chown_socket"_s, u"user:group"_s},
                                           {u"umask"_s, u"0077"_s},
                                           {u"cpu_affinity"_s, 1},
                                           {u"reuse_port"_s, true},
                                           {u"lazy"_s, true},
                                           {u"using_frontend_proxy"_s, true}}}};
}

void TestServer::testSetIni()
{
    Server server;
    server.setIni({m_tmpDir.filePath(u"file1.ini"_s), m_tmpDir.filePath(u"file2.ini"_s)});
    QCOMPARE(server.config(), m_expectedIniConfig);
}

void TestServer::testSetJson()
{
    Server server;
    server.setJson({m_tmpDir.filePath(u"file1.json"_s), m_tmpDir.filePath(u"file2.json"_s)});
    QCOMPARE(server.config(), m_expectedJsonConfig);
}

void TestServer::testSetServerConfigFromFile()
{
    Server server;
    server.setIni(
        {m_tmpDir.filePath(u"serverConfig2.ini"_s), m_tmpDir.filePath(u"serverConfig3.ini"_s)});
    QCOMPARE(server.config(), m_expectedServerConfig);
    QCOMPARE(server.threads(), u"2"_s);
#ifdef Q_OS_UNIX
    QCOMPARE(server.processes(), u"3"_s);
#endif
    QCOMPARE(server.chdir(), u"/path/to/chdir"_s);
    QCOMPARE(server.httpSocket(), QStringList(u"localhost:3000"_s));
    QCOMPARE(server.http2Socket(), QStringList(u"localhost:3001"_s));
    QCOMPARE(server.http2HeaderTableSize(), 123);
    QCOMPARE(server.upgradeH2c(), true);
    QCOMPARE(server.httpsH2(), true);
    QCOMPARE(server.httpsSocket(), QStringList(u"localhost:3002"_s));
    QCOMPARE(server.fastcgiSocket(), QStringList(u"/path/to/socket"_s));
    QCOMPARE(server.socketAccess(), u"ug"_s);
    QCOMPARE(server.socketTimeout(), 4321);
    QCOMPARE(server.chdir2(), u"/path/to/chdir2"_s);
    QCOMPARE(server.staticMap(), QStringList(u"/mountpoint1=/path/to/static1"_s));
    QCOMPARE(server.staticMap2(), QStringList(u"/mountpoint2=/path/to/static2"_s));
    QCOMPARE(server.master(), true);
    QCOMPARE(server.autoReload(), true);
    QCOMPARE(server.touchReload(), QStringList(u"/path/to/file"_s));
    QCOMPARE(server.listenQueue(), 111);
    QCOMPARE(server.bufferSize(), 5432);
    QCOMPARE(server.postBuffering(), 456);
    QCOMPARE(server.postBufferingBufsize(), 5000);
    QCOMPARE(server.tcpNodelay(), true);
    QCOMPARE(server.soKeepalive(), true);
    QCOMPARE(server.socketSndbuf(), 123);
    QCOMPARE(server.socketRcvbuf(), 456);
    QCOMPARE(server.websocketMaxSize(), 2048);
    QCOMPARE(server.pidfile(), u"/path/to/pidfile1"_s);
    QCOMPARE(server.pidfile2(), u"/path/to/pidfile2"_s);
#ifdef Q_OS_UNIX
    QCOMPARE(server.uid(), u"user"_s);
    QCOMPARE(server.gid(), u"group"_s);
    QCOMPARE(server.noInitgroups(), true);
    QCOMPARE(server.chownSocket(), u"user:group"_s);
    QCOMPARE(server.umask(), u"0077"_s);
    QCOMPARE(server.cpuAffinity(), 1);
#endif
#ifdef Q_OS_LINUX
    QCOMPARE(server.reusePort(), true);
#endif
    QCOMPARE(server.lazy(), true);
    QCOMPARE(server.usingFrontendProxy(), true);
}

QTEST_MAIN(TestServer)

#include "testserver.moc"

#endif // TESTSERVER_H
