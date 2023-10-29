#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>

#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>
#include <QUrlQuery>
#include <QUuid>

using namespace Cutelyst;

class TestRequest : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestRequest(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void testUploads_data();
    void testUploads() { doTest(); }

    void cleanupTestCase();

private:
    TestEngine *m_engine;

    TestEngine *getEngine();

    void doTest();
};

class RequestTest : public Controller
{
    Q_OBJECT
public:
    explicit RequestTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(address, :Local :AutoArgs)
    void address(Context *c) { c->response()->setBody(c->request()->address().toString()); }

    C_ATTR(args, :Local :AutoArgs)
    void args(Context *c, const QStringList &args)
    {
        if (c->request()->args() == args) {
            c->response()->setBody(args.join(QLatin1Char('/')));
        } else {
            c->response()->setBody(QByteArrayLiteral("error"));
        }
    }

    C_ATTR(hostname, :Local :AutoArgs)
    void hostname(Context *c) { c->response()->setBody(c->request()->hostname()); }

    C_ATTR(port, :Local :AutoArgs)
    void port(Context *c) { c->response()->setBody(QByteArray::number(c->request()->port())); }

    C_ATTR(uri, :Local :AutoArgs)
    void uri(Context *c) { c->response()->setBody(c->request()->uri().toString()); }

    C_ATTR(base, :Local :AutoArgs)
    void base(Context *c) { c->response()->setBody(c->request()->base()); }

    C_ATTR(path, :Local :AutoArgs)
    void path(Context *c) { c->response()->setBody(c->request()->path()); }

    C_ATTR(match, :Local :AutoArgs)
    void match(Context *c) { c->response()->setBody(c->request()->match()); }

    C_ATTR(method, :Local :AutoArgs)
    void method(Context *c) { c->response()->setBody(c->request()->method()); }

    C_ATTR(isPost, :Local :AutoArgs)
    void isPost(Context *c) { c->response()->setBody(QVariant(c->request()->isPost()).toString()); }

    C_ATTR(isGet, :Local :AutoArgs)
    void isGet(Context *c) { c->response()->setBody(QVariant(c->request()->isGet()).toString()); }

    C_ATTR(protocol, :Local :AutoArgs)
    void protocol(Context *c) { c->response()->setBody(c->request()->protocol()); }

    C_ATTR(remoteUser, :Local :AutoArgs)
    void remoteUser(Context *c) { c->response()->setBody(c->request()->remoteUser()); }

    C_ATTR(headers, :Local :AutoArgs)
    void headers(Context *c)
    {
        QUrlQuery ret;
        auto headers = c->request()->headers().data();
        std::sort(
            headers.begin(),
            headers.end(),
            [](Headers::HeaderKeyValue &a, Headers::HeaderKeyValue &b) { return a.key < b.key; });

        for (const auto &header : headers) {
            ret.addQueryItem(QString::fromLatin1(header.key), QString::fromLatin1(header.value));
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(userAgent, :Local :AutoArgs)
    void userAgent(Context *c) { c->response()->setBody(c->request()->userAgent()); }

    C_ATTR(referer, :Local :AutoArgs)
    void referer(Context *c) { c->response()->setBody(c->request()->referer()); }

    C_ATTR(contentEncoding, :Local :AutoArgs)
    void contentEncoding(Context *c) { c->response()->setBody(c->request()->contentEncoding()); }

    C_ATTR(contentType, :Local :AutoArgs)
    void contentType(Context *c) { c->response()->setBody(c->request()->contentType()); }

    C_ATTR(cookie, :Local :AutoArgs)
    void cookie(Context *c, const QString &name)
    {
        QUrlQuery ret;
        const auto cookie = c->request()->cookie(name.toLatin1());
        if (!cookie.isNull()) {
            ret.addQueryItem(name, QString::fromLatin1(cookie));
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(cookies, :Local :AutoArgs)
    void cookies(Context *c)
    {
        QUrlQuery ret;
        auto cookies = c->request()->cookies();
        for (const auto &cookie : cookies) {
            ret.addQueryItem(QString::fromLatin1(cookie.name), QString::fromLatin1(cookie.value));
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(cookies_list, :Path('cookies') :AutoArgs)
    void cookies_list(Context *c, const QString &name)
    {
        QUrlQuery ret;
        const auto values = c->request()->cookies(name.toLatin1());
        for (const auto &cookie : values) {
            ret.addQueryItem(name, QString::fromLatin1(cookie));
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryKeywords, :Local :AutoArgs)
    void queryKeywords(Context *c) { c->response()->setBody(c->request()->queryKeywords()); }

    C_ATTR(uriWith, :Local :AutoArgs)
    void uriWith(Context *c, const QString &append)
    {
        c->response()->setBody(
            c->request()
                ->uriWith(ParamsMultiMap{{QStringLiteral("foo"), QStringLiteral("baz")},
                                         {QStringLiteral("fooz"), QStringLiteral("bar")}},
                          QVariant(append).toBool())
                .toString(QUrl::FullyEncoded));
    }

    C_ATTR(mangleParams, :Local :AutoArgs)
    void mangleParams(Context *c, const QString &append)
    {
        QUrlQuery ret;
        auto params = c->request()->mangleParams(
            ParamsMultiMap{
                {QStringLiteral("foo"), QStringLiteral("baz")},
            },
            QVariant(append).toBool());
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(body, :Local :AutoArgs)
    void body(Context *c)
    {
        if (c->request()->body()) {
            c->response()->setBody(c->request()->body()->readAll());
        }
    }

    C_ATTR(queryParameters, :Local :AutoArgs)
    void queryParameters(Context *c)
    {
        QUrlQuery ret;
        auto params = c->request()->queryParameters();
        auto it     = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParams, :Local :AutoArgs)
    void queryParams(Context *c)
    {
        QUrlQuery ret;
        auto params = c->request()->queryParams();
        auto it     = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParametersVariant, :Local :AutoArgs)
    void queryParametersVariant(Context *c)
    {
        QUrlQuery ret;
        auto params = c->request()->queryParametersVariant();
        auto it     = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value().toString());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParameter, :Local :AutoArgs)
    void queryParameter(Context *c, const QString &param, const QString &defaultValue)
    {
        c->response()->setBody(c->request()->queryParameter(param, defaultValue));
    }

    C_ATTR(queryParametersList, :Local :AutoArgs)
    void queryParametersList(Context *c, const QString &param)
    {
        const QStringList params = c->request()->queryParams(param);
        c->response()->setBody(params.join(QLatin1Char('&')));
    }

    C_ATTR(queryParam, :Local :AutoArgs)
    void queryParam(Context *c, const QString &param, const QString &defaultValue)
    {
        c->response()->setBody(c->request()->queryParam(param, defaultValue));
    }

    C_ATTR(bodyParameters, :Local :AutoArgs)
    void bodyParameters(Context *c)
    {
        QUrlQuery ret;
        auto params = c->request()->bodyParameters();
        auto it     = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(bodyParams, :Local :AutoArgs)
    void bodyParams(Context *c)
    {
        QUrlQuery ret;
        auto params = c->request()->bodyParams();
        auto it     = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(bodyParametersList, :Local :AutoArgs)
    void bodyParametersList(Context *c)
    {
        const QStringList params =
            c->request()->bodyParameters(c->request()->queryParam(QStringLiteral("param")));
        c->response()->setBody(params.join(QLatin1Char('&')));
    }

    C_ATTR(bodyParameter, :Local :AutoArgs)
    void bodyParameter(Context *c)
    {
        c->response()->setBody(
            c->request()->bodyParameter(c->request()->queryParam(QStringLiteral("param")),
                                        c->request()->queryParam(QStringLiteral("defaultValue"))));
    }

    C_ATTR(bodyParam, :Local :AutoArgs)
    void bodyParam(Context *c)
    {
        c->response()->setBody(
            c->request()->bodyParam(c->request()->queryParam(QStringLiteral("param")),
                                    c->request()->queryParam(QStringLiteral("defaultValue"))));
    }

    C_ATTR(bodyData, :Local :AutoArgs)
    void bodyData(Context *c)
    {
        c->response()->setBody(QByteArray(c->request()->bodyData().typeName()));
    }

    C_ATTR(bodyDataJson, :Local :AutoArgs)
    void bodyDataJson(Context *c)
    {
        c->response()->setBody(
            c->request()->bodyData().toJsonDocument().toJson(QJsonDocument::Compact));
    }

    C_ATTR(uploads, :Local :AutoArgs)
    void uploads(Context *c)
    {
        QUrlQuery ret;
        const auto uploads = c->request()->uploadsMap();
        auto it            = uploads.constBegin();
        while (it != uploads.constEnd()) {
            Upload *upload = it.value();
            ret.addQueryItem(it.key().toString(), upload->name());
            ret.addQueryItem(it.key().toString(), upload->filename());
            ret.addQueryItem(it.key().toString(), QString::fromLatin1(upload->contentType()));
            ret.addQueryItem(it.key().toString(), QString::number(upload->size()));
            ret.addQueryItem(it.key().toString(),
                             QString::fromLatin1(upload->readAll().toBase64()));
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(uploadsName, :Local :AutoArgs)
    void uploadsName(Context *c, const QString &name)
    {
        QUrlQuery ret;
        Uploads uploads = c->request()->uploads(name);
        auto it         = uploads.constBegin();
        while (it != uploads.constEnd()) {
            Upload *upload = *it;
            ret.addQueryItem(upload->name(), upload->filename());
            ret.addQueryItem(upload->name(), QString::fromLatin1(upload->contentType()));
            ret.addQueryItem(upload->name(), QString::number(upload->size()));
            ret.addQueryItem(upload->name(), QString::fromLatin1(upload->readAll().toBase64()));
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(upload, :Local :AutoArgs)
    void upload(Context *c, const QString &name)
    {
        Upload *upload = c->request()->upload(name);
        if (upload) {
            QUrlQuery ret;
            ret.addQueryItem(upload->name(), upload->filename());
            ret.addQueryItem(upload->name(), QString::fromLatin1(upload->contentType()));
            ret.addQueryItem(upload->name(), QString::number(upload->size()));
            ret.addQueryItem(upload->name(), QString::fromLatin1(upload->readAll().toBase64()));
            c->response()->setBody(ret.toString(QUrl::FullyEncoded));
        }
    }
};

void TestRequest::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestRequest::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new RequestTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestRequest::cleanupTestCase()
{
    delete m_engine;
}

void TestRequest::doTest()
{
    QFETCH(QByteArray, method);
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(method,
                                          urlAux.path(QUrl::FullyEncoded),
                                          urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                          headers,
                                          &body);

    //    qDebug() << result.body;
    //    qDebug() << output;
    QCOMPARE(result.body, output);
}

void TestRequest::testController_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    const auto get  = "GET"_qba;
    const auto GeT  = "GeT"_qba;
    const auto post = "POST"_qba;
    const auto head = "HEAD"_qba;
    const auto PoSt = "PoSt"_qba;

    QUrlQuery query;
    Headers headers;
    QByteArray body;

    QTest::newRow("address-test00") << get << QStringLiteral("/request/test/address") << headers
                                    << QByteArray() << QByteArrayLiteral("127.0.0.1");

    QTest::newRow("args-test00") << get << QStringLiteral("/request/test/args/a%C3%A9%C3%A3u")
                                 << headers << QByteArray()
                                 << QByteArrayLiteral("a\xC3\xA9\xC3\xA3u");

    QTest::newRow("hostname-test00")
        << get << QStringLiteral("/request/test/hostname") << headers << QByteArray()
        << QHostInfo::fromName(QStringLiteral("127.0.0.1")).hostName().toLatin1();
    QTest::newRow("port-test00") << get << QStringLiteral("/request/test/port") << headers
                                 << QByteArray() << QByteArrayLiteral("3000");
    QTest::newRow("uri-test00") << get << QStringLiteral("/request/test/uri") << headers
                                << QByteArray()
                                << QByteArrayLiteral("http://127.0.0.1/request/test/uri");
    QTest::newRow("base-test00") << get << QStringLiteral("/request/test/base") << headers
                                 << QByteArray() << QByteArrayLiteral("http://127.0.0.1");
    QTest::newRow("path-test00") << get << QStringLiteral("/request/test/path") << headers
                                 << QByteArray() << QByteArrayLiteral("/request/test/path");
    QTest::newRow("match-test00") << get << QStringLiteral("/request/test/match") << headers
                                  << QByteArray() << QByteArrayLiteral("/request/test/match");

    QTest::newRow("method-test00") << get << QStringLiteral("/request/test/method") << headers
                                   << QByteArray() << QByteArrayLiteral("GET");
    QTest::newRow("method-test01") << post << QStringLiteral("/request/test/method") << headers
                                   << QByteArray() << QByteArrayLiteral("POST");
    QTest::newRow("method-test02") << head << QStringLiteral("/request/test/method") << headers
                                   << QByteArray() << QByteArrayLiteral("HEAD");

    QTest::newRow("isPost-test00") << get << QStringLiteral("/request/test/isPost") << headers
                                   << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isPost-test01") << PoSt << QStringLiteral("/request/test/isPost") << headers
                                   << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isPost-test02") << post << QStringLiteral("/request/test/isPost") << headers
                                   << QByteArray() << QByteArrayLiteral("true");

    QTest::newRow("isGet-test00") << post << QStringLiteral("/request/test/isGet") << headers
                                  << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isGet-test01") << GeT << QStringLiteral("/request/test/isGet") << headers
                                  << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isGet-test02") << get << QStringLiteral("/request/test/isGet") << headers
                                  << QByteArray() << QByteArrayLiteral("true");

    QTest::newRow("protocol-test00") << get << QStringLiteral("/request/test/protocol") << headers
                                     << QByteArray() << QByteArrayLiteral("HTTP/1.1");
    QTest::newRow("remoteUser-test00") << get << QStringLiteral("/request/test/remoteUser")
                                       << headers << QByteArray() << QByteArrayLiteral("");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("bar"));
    headers.setReferer("http://www.cutelyst.org");
    QTest::newRow("headers-test00")
        << get << QStringLiteral("/request/test/headers") << headers << QByteArray()
        << QByteArrayLiteral(
               "Authorization=Basic%20Zm9vOmJhcg%3D%3D&Referer=http://www.cutelyst.org");

    headers.clear();
    headers.setHeader("User-Agent",
                      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, "
                      "like Gecko) Chrome/51.0.2704.79 Safari/537.36");
    QTest::newRow("userAgent-test00")
        << get << QStringLiteral("/request/test/userAgent") << headers << QByteArray()
        << QByteArrayLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like "
                             "Gecko) Chrome/51.0.2704.79 Safari/537.36");

    headers.clear();
    headers.setHeader("Referer", "http://www.cutelyst.org");
    QTest::newRow("referer-test00") << get << QStringLiteral("/request/test/referer") << headers
                                    << QByteArray() << QByteArrayLiteral("http://www.cutelyst.org");

    headers.clear();
    headers.setHeader("Content-Encoding", "gzip");
    QTest::newRow("contentEncoding-test00")
        << get << QStringLiteral("/request/test/contentEncoding") << headers << QByteArray()
        << QByteArrayLiteral("gzip");

    headers.clear();
    headers.setHeader("Content-Type", "text/html; charset=UTF-8");
    QTest::newRow("contentType-test00")
        << get << QStringLiteral("/request/test/contentType") << headers << QByteArray()
        << QByteArrayLiteral("text/html");

    query.clear();
    headers.setHeader("Cookie", QByteArray{});
    QTest::newRow("cookies-test00") << get << QStringLiteral("/request/test/cookies") << headers
                                    << QByteArray() << QByteArrayLiteral("");
    query.clear();
    headers.setHeader("Cookie", QByteArray(""));
    QTest::newRow("cookies-test01") << get << QStringLiteral("/request/test/cookies") << headers
                                    << QByteArray() << QByteArrayLiteral("");

    query.clear();
    headers.setHeader("Cookie",
                      "FIRST=10186486272; SECOND=AF6bahuOZFc_P7-oCw; "
                      "S=foo=TGp743-6uvY:first=MnBbT3wcrA-uy=MnwcrA:bla=0L7g; "
                      "S=something=qjgNs_EA:more=n1Ki8xVsQ:andmore=FSg:andmore=nQMU_"
                      "0VRlTJAbs_4fw:gmail=j4yGWsKuoZg");
    QTest::newRow("cookies-test02")
        << get << QStringLiteral("/request/test/cookies") << headers << QByteArray()
        << QByteArrayLiteral(
               "FIRST=10186486272&S=something%3DqjgNs_EA:more%3Dn1Ki8xVsQ:andmore%3DFSg:andmore%"
               "3DnQMU_0VRlTJAbs_4fw:gmail%3Dj4yGWsKuoZg&S=foo%3DTGp743-6uvY:first%3DMnBbT3wcrA-uy%"
               "3DMnwcrA:bla%3D0L7g&SECOND=AF6bahuOZFc_P7-oCw");

    query.clear();
    headers.setHeader("Cookie", "b=1; a=1; a=2; a=3; b=0");
    QTest::newRow("cookies-test03") << get << QStringLiteral("/request/test/cookies/a") << headers
                                    << QByteArray() << QByteArrayLiteral("a=1&a=2&a=3");

    query.clear();
    headers.setHeader("Cookie", QByteArray{});
    QTest::newRow("cookie-test00") << get << QStringLiteral("/request/test/cookie/foo") << headers
                                   << QByteArray() << QByteArrayLiteral("");

    query.clear();
    headers.setHeader("Cookie",
                      "FIRST=10186486272; SECOND=AF6bahuOZFc_P7-oCw; "
                      "S=foo=TGp743-6uvY:first=MnBbT3wcrA-uy=MnwcrA:bla=0L7g; "
                      "S=something=qjgNs_EA:more=n1Ki8xVsQ:andmore=FSg:andmore=nQMU_"
                      "0VRlTJAbs_4fw:gmail=j4yGWsKuoZg");
    QTest::newRow("cookie-test01")
        << get << QStringLiteral("/request/test/cookie/S") << headers << QByteArray()
        << QByteArrayLiteral("S=something%3DqjgNs_EA:more%3Dn1Ki8xVsQ:andmore%3DFSg:andmore%3DnQMU_"
                             "0VRlTJAbs_4fw:gmail%3Dj4yGWsKuoZg");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    QTest::newRow("queryKeywords-test00")
        << get
        << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("some text to ask");
    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QStringLiteral("not"));
    QTest::newRow("queryKeywords-test01")
        << get
        << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryKeywords-test02")
        << get
        << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral("some text to ask&another keyword&and yet another is fine");

    query.clear();
    query.addQueryItem(QStringLiteral("some+text+to+ask"), QString());
    QTest::newRow("queryKeywords-test03")
        << get
        << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("some text to ask");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriWith-test00")
        << get
        << QStringLiteral("/request/test/uriWith/false?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral("http://127.0.0.1/request/test/uriWith/false?fooz=bar&foo=baz");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriWith-test01")
        << get << QStringLiteral("/request/test/uriWith/true?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral("http://127.0.0.1/request/test/uriWith/true?fooz=bar&foo=bar&foo=baz");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baz"));
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("mangleParams-test00")
        << post << QStringLiteral("/request/test/mangleParams/false?foo=bar&x=y") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("foo=baz&x=y");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baz"));
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("mangleParams-test01")
        << post << QStringLiteral("/request/test/mangleParams/true?foo=bar&x=y") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1()
        << QByteArrayLiteral("foo=baz&foo=bar&x=y");

    query.clear();
    QTest::newRow("body-test00") << get << QStringLiteral("/request/test/body") << headers
                                 << QByteArray() << QByteArray();

    query.clear();
    body = QUuid::createUuid().toByteArray();
    QTest::newRow("body-test00") << get << QStringLiteral("/request/test/body") << headers << body
                                 << body;

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParameters-test00")
        << get
        << QStringLiteral("/request/test/queryParameters?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParameters-test01")
        << get
        << QStringLiteral("/request/test/queryParameters?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral(
               "and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParams-test00")
        << get << QStringLiteral("/request/test/queryParams?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParams-test01")
        << get << QStringLiteral("/request/test/queryParams?") + query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral(
               "and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    QTest::newRow("queryParams-test02")
        << get << QStringLiteral("/request/test/queryParams?&=&=&foo&&&=ooo&=bar") << headers
        << QByteArray() << QByteArrayLiteral("=bar&=ooo&foo");

    QTest::newRow("queryParams-test03")
        << get << QStringLiteral("/request/test/queryParams?&foo=bar&=&something&&&=ooo&=bar")
        << headers << QByteArray() << QByteArrayLiteral("=bar&=ooo&foo=bar&something");

    QTest::newRow("queryParams-test04")
        << get << QStringLiteral("/request/test/queryParams?foo=bar&=&something&&&=ooo&=bar&")
        << headers << QByteArray() << QByteArrayLiteral("=bar&=ooo&foo=bar&something");

    QTest::newRow("queryParams-test05")
        << get << QStringLiteral("/request/test/queryParams?&foo=bar&=&something&&&=ooo&=bar&")
        << headers << QByteArray() << QByteArrayLiteral("=bar&=ooo&foo=bar&something");

    QTest::newRow("queryParams-test06")
        << get << QStringLiteral("/request/test/queryParams?a=1&a=2&b=0&a=0&a=1&a=3&a=2") << headers
        << QByteArray() << QByteArrayLiteral("a=2&a=3&a=1&a=0&a=2&a=1&b=0");
    QTest::newRow("queryParams-test07")
        << get << QStringLiteral("/request/test/queryParams?foo=bar&baz=") << headers
        << QByteArray() << QByteArrayLiteral("baz&foo=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParametersVariant-test00")
        << get
        << QStringLiteral("/request/test/queryParametersVariant?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParametersVariant-test01")
        << get
        << QStringLiteral("/request/test/queryParametersVariant?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray()
        << QByteArrayLiteral(
               "and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParameter-test00")
        << get
        << QStringLiteral("/request/test/queryParameter/foo/gotDefault?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParameter-test01")
        << get
        << QStringLiteral("/request/test/queryParameter/x/gotDefault?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParameter-test02")
        << get
        << QStringLiteral("/request/test/queryParameter/y/gotDefault?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("gotDefault");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral(""));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParametersList-test00")
        << get
        << QStringLiteral("/request/test/queryParametersList/foo?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("&bar&baz");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParam-test00")
        << get
        << QStringLiteral("/request/test/queryParam/foo/gotDefault?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParam-test01") << get
                                       << QStringLiteral("/request/test/queryParam/x/gotDefault?") +
                                              query.toString(QUrl::FullyEncoded)
                                       << headers << QByteArray() << QByteArrayLiteral("");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParam-test02")
        << get
        << QStringLiteral("/request/test/queryParam/y/gotDefault?") +
               query.toString(QUrl::FullyEncoded)
        << headers << QByteArray() << QByteArrayLiteral("gotDefault");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParameters-test00")
        << get << QStringLiteral("/request/test/bodyParameters") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1()
        << QByteArrayLiteral(
               "and%20yet%20another%20is%20fine&another%20keyword&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParams-test00")
        << get << QStringLiteral("/request/test/bodyParams") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1()
        << QByteArrayLiteral(
               "and%20yet%20another%20is%20fine&another%20keyword&some%20text%20to%20ask");

    query.clear();
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParams-test01")
        << get << QStringLiteral("/request/test/bodyParams") << headers
        << QByteArrayLiteral("foo=bar&baz=") << QByteArrayLiteral("baz&foo=bar");

    query.clear();
    headers.clear();
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("bodyParams-test02")
        << get << QStringLiteral("/request/test/bodyParams") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntextooooo\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"wifi\"\r\nContent-Type: "
               "application/"
               "octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArrayLiteral("path=textooooo");

    query.clear();
    headers.clear();
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("bodyParams-test03")
        << get << QStringLiteral("/request/test/bodyParams") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntexto1\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-"
               "Disposition: form-data; "
               "name=\"path\"\r\n\r\ntexto2\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-"
               "Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArrayLiteral("path=texto2&path=texto1");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral(""));
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baa"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParametersList-test00")
        << get << QStringLiteral("/request/test/bodyParametersList?param=foo") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("bar&&baa");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test00")
        << get << QStringLiteral("/request/test/bodyParam?param=foo") << headers
        << query.toString(QUrl::FullyEncoded).toLatin1() << body;

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test01")
        << get << QStringLiteral("/request/test/bodyParam?param=y&defaultValue=SomeDefaultValue")
        << headers << query.toString(QUrl::FullyEncoded).toLatin1()
        << QByteArrayLiteral("SomeDefaultValue");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString{});
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test02")
        << get << QStringLiteral("/request/test/bodyParam?param=x&defaultValue=SomeDefaultValue")
        << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("x"), QStringLiteral("foo bar"));
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test02")
        << get << QStringLiteral("/request/test/bodyParam?param=x&defaultValue=SomeDefaultValue")
        << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("foo bar");

    query.clear();
    query.addQueryItem(QStringLiteral("x+y"), QStringLiteral("foo+bar"));
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test03")
        << get << QStringLiteral("/request/test/bodyParam?param=x+y&defaultValue=SomeDefaultValue")
        << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("foo bar");

    query.clear();
    query.addQueryItem(QStringLiteral("x%2By"), QStringLiteral("foo%2Bbar"));
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyParam-test04")
        << get
        << QStringLiteral("/request/test/bodyParam?param=x%2By&defaultValue=SomeDefaultValue")
        << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("foo+bar");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyData-test01") << post << QStringLiteral("/request/test/bodyData") << headers
                                     << query.toString(QUrl::FullyEncoded).toLatin1()
                                     << QByteArrayLiteral("QMultiMap<QString,QString>");

    query.clear();
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyData-test02")
        << post << QStringLiteral("/request/test/bodyData") << headers << QByteArray()
        << QByteArrayLiteral("QMultiMap<QString,QString>");

    query.clear();
    headers.setContentType("application/x-www-form-urlencoded");
    QTest::newRow("bodyData-test03")
        << post << QStringLiteral("/request/test/bodyData") << headers << QByteArray()
        << QByteArrayLiteral("QMultiMap<QString,QString>");

    query.clear();
    headers.setContentType("application/json");
    QTest::newRow("bodyData-test04") << post << QStringLiteral("/request/test/bodyData") << headers
                                     << QByteArray() << QByteArrayLiteral("QJsonDocument");

    query.clear();
    headers.setContentType("multipart/form-data");
    QTest::newRow("bodyData-test05") << post << QStringLiteral("/request/test/bodyData") << headers
                                     << QByteArray() << QByteArrayLiteral("");

    query.clear();
    QJsonObject obj;
    obj.insert(QStringLiteral("foo"), QStringLiteral("bar"));
    QJsonArray array;
    array.append(obj);
    headers.setContentType("application/json");
    QTest::newRow("bodyDataJson-test00")
        << post << QStringLiteral("/request/test/bodyDataJson") << headers
        << QJsonDocument(array).toJson(QJsonDocument::Compact)
        << QByteArrayLiteral("[{\"foo\":\"bar\"}]");

    query.clear();
    obj   = QJsonObject();
    array = QJsonArray();
    obj.insert(QStringLiteral("foo"), QStringLiteral("bar"));
    array.append(obj);
    headers.setHeader("sequential", "true");
    headers.setContentType("application/json");
    QTest::newRow("bodyDataJson-test01")
        << post << QStringLiteral("/request/test/bodyDataJson") << headers
        << QJsonDocument(array).toJson(QJsonDocument::Compact)
        << QByteArrayLiteral("[{\"foo\":\"bar\"}]");

    query.clear();
    headers.clear();
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploads-test00")
        << get << QStringLiteral("/request/test/uploads") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntextooooo\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"wifi\"\r\nContent-Type: "
               "application/"
               "octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArrayLiteral("file1=file1&file1=example.txt&file1=application/"
                             "octet-stream&file1=27&file1=aHR0cHM6Ly9leGFtcGxlLmNvbS9hZG1pbgoK&"
                             "file1=file1&file1=wifi&file1=application/"
                             "octet-stream&file1=23&file1=TU9UT0NNCk1PVE9DTQowMDAwMDAwMAo%3D&path="
                             "path&path&path&path=9&path=dGV4dG9vb29v");

    query.clear();
    headers.clear();
    headers.setHeader("sequential", "true");
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploads-test01")
        << post << QStringLiteral("/request/test/uploads") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntextooooo\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"wifi\"\r\nContent-Type: "
               "application/"
               "octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArray();

    query.clear();
    headers.clear();
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploadsName-test00")
        << get << QStringLiteral("/request/test/uploadsName/file1") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntextooooo\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"wifi\"\r\nContent-Type: "
               "application/"
               "octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArrayLiteral(
               "file1=example.txt&file1=application/"
               "octet-stream&file1=27&file1=aHR0cHM6Ly9leGFtcGxlLmNvbS9hZG1pbgoK&file1=wifi&file1="
               "application/octet-stream&file1=23&file1=TU9UT0NNCk1PVE9DTQowMDAwMDAwMAo%3D");

    query.clear();
    headers.clear();
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("upload-test00")
        << post << QStringLiteral("/request/test/upload/file1") << headers
        << QByteArrayLiteral(
               "------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"path\"\r\n\r\ntextooooo\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"wifi\"\r\nContent-Type: "
               "application/"
               "octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------"
               "WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; "
               "name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: "
               "application/octet-stream\r\n\r\nhttps://example.com/"
               "admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
        << QByteArrayLiteral("file1=example.txt&file1=application/"
                             "octet-stream&file1=27&file1=aHR0cHM6Ly9leGFtcGxlLmNvbS9hZG1pbgoK");
}

QByteArray createBody(QByteArray &result, int count)
{
    QByteArray body;
    QMap<QString, QByteArray> uploads;

    for (int i = 0; i < count; ++i) {
        QString name = QLatin1String("file-") + QString::number(i) + QLatin1String(".bin");
        body.append("------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\n");
        body.append("Content-Disposition: form-data; name=\"" + name.toLatin1() +
                    "\"; filename=\"file.bin\"\r\nContent-Type: application/octet-stream\r\n\r\n");
        QByteArray data = QUuid::createUuid().toByteArray().toBase64();
        body.append(data);
        body.append("\r\n");
        uploads.insert(name, data);
    }
    body.append("------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n");

    QUrlQuery ret;
    auto it = uploads.constBegin();
    while (it != uploads.constEnd()) {
        const QByteArray upload = it.value();
        ret.addQueryItem(it.key(), it.key());
        ret.addQueryItem(it.key(), QStringLiteral("file.bin"));
        ret.addQueryItem(it.key(), QStringLiteral("application/octet-stream"));
        ret.addQueryItem(it.key(), QString::number(upload.size()));
        ret.addQueryItem(it.key(), QString::fromLatin1(upload.toBase64()));
        ++it;
    }
    result = ret.toString(QUrl::FullyEncoded).toUtf8();

    return body;
}

void TestRequest::testUploads_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    const auto post = "POST"_qba;

    QUrlQuery query;
    Headers headers;
    QByteArray body;
    QByteArray result;

    query.clear();
    headers.clear();
    body = createBody(result, 1);
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploads-1") << post << QStringLiteral("/request/test/uploads") << headers << body
                               << result;

    query.clear();
    headers.clear();
    body = createBody(result, 10);
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploads-10") << post << QStringLiteral("/request/test/uploads") << headers
                                << body << result;

    query.clear();
    headers.clear();
    body = createBody(result, 100);
    headers.setContentType("multipart/form-data; boundary=----WebKitFormBoundaryoPPQLwBBssFnOTVH");
    QTest::newRow("uploads-100") << post << QStringLiteral("/request/test/uploads") << headers
                                 << body << result;
}
QTEST_MAIN(TestRequest)

#include "testrequest.moc"

#endif
