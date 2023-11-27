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
    QFile f{fileName};
    QVERIFY(f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text));

    const QJsonDocument json{data};

    QVERIFY(f.write(json.toJson(QJsonDocument::Indented)) > 0);
}

void TestServer::initTestCase()
{
    QVERIFY(m_tmpDir.isValid());

    // load this from the server/ini value in file1.ini
    const QString file3Ini = m_tmpDir.filePath(u"file3.ini"_qs);
    writeIniFile(file3Ini,
                 {{u"Testsection1"_qs, {{u"john"_qs, u"doe"_qs}}},
                  {u"Testsection3"_qs, {{u"hello"_qs, u"world"_qs}}}});

    const QString file1Ini = m_tmpDir.filePath(u"file1.ini"_qs);
    writeIniFile(file1Ini,
                 {{u"Cutelyst"_qs, {{u"hello"_qs, u"world1"_qs}, {u"onlyin1"_qs, u"hello"_qs}}},
                  {u"Testsection1"_qs, {{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}}},
                  {u"server"_qs, {{u"ini"_qs, file3Ini}}}});

    const QString file2Ini = m_tmpDir.filePath(u"file2.ini"_qs);
    writeIniFile(file2Ini,
                 {{u"Cutelyst"_qs, {{u"hello"_qs, u"world2"_qs}, {u"onlyin2"_qs, u"world"_qs}}},
                  {u"Testsection2"_qs, {{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}}});

    m_expectedIniConfig = {
        {u"Cutelyst"_qs,
         QVariantMap{{u"hello"_qs, u"world2"_qs},
                     {u"onlyin1"_qs, u"hello"_qs},
                     {u"onlyin2"_qs, u"world"_qs}}},
        {u"Testsection1"_qs,
         QVariantMap{{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}, {u"john"_qs, u"doe"_qs}}},
        {u"Testsection2"_qs, QVariantMap{{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}},
        {u"Testsection3"_qs, QVariantMap{{u"hello"_qs, u"world"_qs}}},
        {u"server"_qs, QVariantMap{{u"ini"_qs, file3Ini}}}};

    // load this from the server/json value in file1.json
    const QString file3Json = m_tmpDir.filePath(u"file3.json"_qs);
    writeJsonFile(file3Json,
                  QJsonObject{{u"Testsection1"_qs, QJsonObject{{u"john"_qs, u"doe"_qs}}},
                              {u"Testsection3"_qs, QJsonObject{{u"hello"_qs, u"world"_qs}}}});

    const QString file1Json = m_tmpDir.filePath(u"file1.json"_qs);
    writeJsonFile(
        file1Json,
        QJsonObject{
            {{u"Cutelyst"_qs,
              QJsonObject{{u"hello"_qs, u"world1"_qs}, {u"onlyin1"_qs, u"hello"_qs}}},
             {u"Testsection1"_qs, QJsonObject{{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}}},
             {u"server"_qs, QJsonObject{{u"json"_qs, file3Json}}}}});

    const QString file2Json = m_tmpDir.filePath(u"file2.json"_qs);
    writeJsonFile(
        file2Json,
        QJsonObject{{{u"Cutelyst"_qs,
                      QJsonObject{{u"hello"_qs, u"world2"_qs}, {u"onlyin2"_qs, u"world"_qs}}},
                     {u"Testsection2"_qs,
                      QJsonObject{{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}}}});

    m_expectedJsonConfig = {
        {u"Cutelyst"_qs,
         QVariantMap{{u"hello"_qs, u"world2"_qs},
                     {u"onlyin1"_qs, u"hello"_qs},
                     {u"onlyin2"_qs, u"world"_qs}}},
        {u"Testsection1"_qs,
         QVariantMap{{u"foo"_qs, u"bar"_qs}, {u"hello"_qs, u"world"_qs}, {u"john"_qs, u"doe"_qs}}},
        {u"Testsection2"_qs, QVariantMap{{u"fu"_qs, u"baz"_qs}, {u"hello"_qs, u"world"_qs}}},
        {u"Testsection3"_qs, QVariantMap{{u"hello"_qs, u"world"_qs}}},
        {u"server"_qs, QVariantMap{{u"json"_qs, file3Json}}}};

    const QString serverConfig1Ini = m_tmpDir.filePath(u"serverConfig1.ini"_qs);
    writeIniFile(serverConfig1Ini,
                 {{u"Testsection1"_qs, {{u"hello"_qs, u"world"_qs}}},
                  {u"server"_qs,
                   {{u"threads"_qs, 2},
                    {u"processes"_qs, 3},
                    {u"chdir"_qs, u"/path/to/chdir"_qs},
                    {u"http_socket"_qs, u"localhost:3000"_qs},
                    {u"http2_socket"_qs, u"localhost:3001"_qs},
                    {u"http2_header_table_size"_qs, 123},
                    {u"upgrade_h2c"_qs, true},
                    {u"https_h2"_qs, true},
                    {u"https_socket"_qs, u"localhost:3002"_qs},
                    {u"fastcgi_socket"_qs, u"/path/to/socket"_qs},
                    {u"socket_access"_qs, u"ug"_qs},
                    {u"socket_timeout"_qs, 4321},
                    {u"chdir2"_qs, u"/path/to/chdir2"_qs},
                    {u"listen"_qs, 111},
                    {u"socket_sndbuf"_qs, 123},
                    {u"socket_rcvbuf"_qs, 456}}}});

    const QString serverConfig2Ini = m_tmpDir.filePath(u"serverConfig2.ini"_qs);
    writeIniFile(serverConfig2Ini,
                 {{u"Testsection2"_qs, {{u"foo"_qs, u"bar"_qs}}},
                  {u"server"_qs,
                   {{u"ini"_qs, serverConfig1Ini},
                    {u"static_map"_qs, u"/mountpoint1=/path/to/static1"_qs},
                    {u"static_map2"_qs, u"/mountpoint2=/path/to/static2"_qs},
                    {u"master"_qs, true},
                    {u"auto_reload"_qs, true},
                    {u"touch_reload"_qs, u"/path/to/file"_qs},
                    {u"buffer_size"_qs, 5432},
                    {u"post_buffering"_qs, 456},
                    {u"post_buffering_bufsize"_qs, 5000},
                    {u"tcp_nodelay"_qs, true},
                    {u"so_keepalive"_qs, true},
                    {u"websocket_max_size"_qs, 2048}}}});

    const QString serverConfig3Ini = m_tmpDir.filePath(u"serverConfig3.ini"_qs);
    writeIniFile(serverConfig3Ini,
                 {{u"Testsection3"_qs, {{u"fu"_qs, u"baz"_qs}}},
                  {u"server"_qs,
                   {{u"pidfile"_qs, u"/path/to/pidfile1"_qs},
                    {u"pidfile2"_qs, u"/path/to/pidfile2"_qs},
                    {u"uid"_qs, u"user"_qs},
                    {u"gid"_qs, u"group"_qs},
                    {u"no_initgroups"_qs, true},
                    {u"chown_socket"_qs, u"user:group"_qs},
                    {u"umask"_qs, u"0077"_qs},
                    {u"cpu_affinity"_qs, 1},
                    {u"reuse_port"_qs, true},
                    {u"lazy"_qs, true},
                    {u"using_frontend_proxy"_qs, true}}}});

    m_expectedServerConfig = {{u"Testsection1"_qs, QVariantMap{{u"hello"_qs, u"world"_qs}}},
                              {u"Testsection2"_qs, QVariantMap{{u"foo"_qs, u"bar"_qs}}},
                              {u"Testsection3"_qs, QVariantMap{{u"fu"_qs, u"baz"_qs}}},
                              {u"server"_qs,
                               QVariantMap{{u"threads"_qs, 2},
                                           {u"processes"_qs, 3},
                                           {u"chdir"_qs, u"/path/to/chdir"_qs},
                                           {u"http_socket"_qs, u"localhost:3000"_qs},
                                           {u"http2_socket"_qs, u"localhost:3001"_qs},
                                           {u"http2_header_table_size"_qs, 123},
                                           {u"upgrade_h2c"_qs, true},
                                           {u"https_h2"_qs, true},
                                           {u"https_socket"_qs, u"localhost:3002"_qs},
                                           {u"fastcgi_socket"_qs, u"/path/to/socket"_qs},
                                           {u"socket_access"_qs, u"ug"_qs},
                                           {u"socket_timeout"_qs, 4321},
                                           {u"chdir2"_qs, u"/path/to/chdir2"_qs},
                                           {u"ini"_qs, serverConfig1Ini},
                                           {u"static_map"_qs, u"/mountpoint1=/path/to/static1"_qs},
                                           {u"static_map2"_qs, u"/mountpoint2=/path/to/static2"_qs},
                                           {u"master"_qs, true},
                                           {u"auto_reload"_qs, true},
                                           {u"touch_reload"_qs, u"/path/to/file"_qs},
                                           {u"listen"_qs, 111},
                                           {u"buffer_size"_qs, 5432},
                                           {u"post_buffering"_qs, 456},
                                           {u"post_buffering_bufsize"_qs, 5000},
                                           {u"tcp_nodelay"_qs, true},
                                           {u"so_keepalive"_qs, true},
                                           {u"socket_sndbuf"_qs, 123},
                                           {u"socket_rcvbuf"_qs, 456},
                                           {u"websocket_max_size"_qs, 2048},
                                           {u"pidfile"_qs, u"/path/to/pidfile1"_qs},
                                           {u"pidfile2"_qs, u"/path/to/pidfile2"_qs},
                                           {u"uid"_qs, u"user"_qs},
                                           {u"gid"_qs, u"group"_qs},
                                           {u"no_initgroups"_qs, true},
                                           {u"chown_socket"_qs, u"user:group"_qs},
                                           {u"umask"_qs, u"0077"_qs},
                                           {u"cpu_affinity"_qs, 1},
                                           {u"reuse_port"_qs, true},
                                           {u"lazy"_qs, true},
                                           {u"using_frontend_proxy"_qs, true}}}};
}

void TestServer::testSetIni()
{
    Server server;
    server.setIni({m_tmpDir.filePath(u"file1.ini"_qs), m_tmpDir.filePath(u"file2.ini"_qs)});
    QCOMPARE(server.config(), m_expectedIniConfig);
}

void TestServer::testSetJson()
{
    Server server;
    server.setJson({m_tmpDir.filePath(u"file1.json"_qs), m_tmpDir.filePath(u"file2.json"_qs)});
    QCOMPARE(server.config(), m_expectedJsonConfig);
}

void TestServer::testSetServerConfigFromFile()
{
    Server server;
    server.setIni(
        {m_tmpDir.filePath(u"serverConfig2.ini"_qs), m_tmpDir.filePath(u"serverConfig3.ini"_qs)});
    QCOMPARE(server.config(), m_expectedServerConfig);
    QCOMPARE(server.threads(), u"2"_qs);
    QCOMPARE(server.processes(), u"3"_qs);
    QCOMPARE(server.chdir(), u"/path/to/chdir"_qs);
    QCOMPARE(server.httpSocket(), QStringList(u"localhost:3000"_qs));
    QCOMPARE(server.http2Socket(), QStringList(u"localhost:3001"_qs));
    QCOMPARE(server.http2HeaderTableSize(), 123);
    QCOMPARE(server.upgradeH2c(), true);
    QCOMPARE(server.httpsH2(), true);
    QCOMPARE(server.httpsSocket(), QStringList(u"localhost:3002"_qs));
    QCOMPARE(server.fastcgiSocket(), QStringList(u"/path/to/socket"_qs));
    QCOMPARE(server.socketAccess(), u"ug"_qs);
    QCOMPARE(server.socketTimeout(), 4321);
    QCOMPARE(server.chdir2(), u"/path/to/chdir2"_qs);
    QCOMPARE(server.staticMap(), QStringList(u"/mountpoint1=/path/to/static1"_qs));
    QCOMPARE(server.staticMap2(), QStringList(u"/mountpoint2=/path/to/static2"_qs));
    QCOMPARE(server.master(), true);
    QCOMPARE(server.autoReload(), true);
    QCOMPARE(server.touchReload(), QStringList(u"/path/to/file"_qs));
    QCOMPARE(server.listenQueue(), 111);
    QCOMPARE(server.bufferSize(), 5432);
    QCOMPARE(server.postBuffering(), 456);
    QCOMPARE(server.postBufferingBufsize(), 5000);
    QCOMPARE(server.tcpNodelay(), true);
    QCOMPARE(server.soKeepalive(), true);
    QCOMPARE(server.socketSndbuf(), 123);
    QCOMPARE(server.socketRcvbuf(), 456);
    QCOMPARE(server.websocketMaxSize(), 2048);
    QCOMPARE(server.pidfile(), u"/path/to/pidfile1"_qs);
    QCOMPARE(server.pidfile2(), u"/path/to/pidfile2"_qs);
#ifdef Q_OS_UNIX
    QCOMPARE(server.uid(), u"user"_qs);
    QCOMPARE(server.gid(), u"group"_qs);
    QCOMPARE(server.noInitgroups(), true);
    QCOMPARE(server.chownSocket(), u"user:group"_qs);
    QCOMPARE(server.umask(), u"0077"_qs);
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
