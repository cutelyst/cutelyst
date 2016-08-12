#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>
#include <QHostInfo>
#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>

using namespace Cutelyst;

class TestRequest : public CoverageObject
{
    Q_OBJECT
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

};

class RequestTest : public Controller
{
    Q_OBJECT
public:
    RequestTest(QObject *parent) : Controller(parent) {}

    C_ATTR(address, :Local :AutoArgs)
    void address(Context *c) {
        c->response()->setBody(c->request()->address().toString());
    }

    C_ATTR(hostname, :Local :AutoArgs)
    void hostname(Context *c) {
        c->response()->setBody(c->request()->hostname());
    }

    C_ATTR(port, :Local :AutoArgs)
    void port(Context *c) {
        c->response()->setBody(QByteArray::number(c->request()->port()));
    }

    C_ATTR(uri, :Local :AutoArgs)
    void uri(Context *c) {
        c->response()->setBody(c->request()->uri().toString());
    }

    C_ATTR(base, :Local :AutoArgs)
    void base(Context *c) {
        c->response()->setBody(c->request()->base());
    }

    C_ATTR(path, :Local :AutoArgs)
    void path(Context *c) {
        c->response()->setBody(c->request()->path());
    }

    C_ATTR(match, :Local :AutoArgs)
    void match(Context *c) {
        c->response()->setBody(c->request()->match());
    }

    C_ATTR(method, :Local :AutoArgs)
    void method(Context *c) {
        c->response()->setBody(c->request()->method());
    }

    C_ATTR(isPost, :Local :AutoArgs)
    void isPost(Context *c) {
        c->response()->setBody(QVariant(c->request()->isPost()).toString());
    }

    C_ATTR(isGet, :Local :AutoArgs)
    void isGet(Context *c) {
        c->response()->setBody(QVariant(c->request()->isGet()).toString());
    }

    C_ATTR(protocol, :Local :AutoArgs)
    void protocol(Context *c) {
        c->response()->setBody(c->request()->protocol());
    }

    C_ATTR(remoteUser, :Local :AutoArgs)
    void remoteUser(Context *c) {
        c->response()->setBody(c->request()->remoteUser());
    }

    C_ATTR(headers, :Local :AutoArgs)
    void headers(Context *c) {
        QUrlQuery ret;
        auto headers = c->request()->headers().map();
        auto it = headers.constBegin();
        while (it != headers.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(userAgent, :Local :AutoArgs)
    void userAgent(Context *c) {
        c->response()->setBody(c->request()->userAgent());
    }

    C_ATTR(referer, :Local :AutoArgs)
    void referer(Context *c) {
        c->response()->setBody(c->request()->referer());
    }

    C_ATTR(contentEncoding, :Local :AutoArgs)
    void contentEncoding(Context *c) {
        c->response()->setBody(c->request()->contentEncoding());
    }

    C_ATTR(contentType, :Local :AutoArgs)
    void contentType(Context *c) {
        c->response()->setBody(c->request()->contentType());
    }

    C_ATTR(cookie, :Local :AutoArgs)
    void cookie(Context *c, const QString &name) {
        QUrlQuery ret;
        const QString cookie = c->request()->cookie(name);
        if (!cookie.isNull()) {
            ret.addQueryItem(name, cookie);
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(cookies, :Local :AutoArgs)
    void cookies(Context *c) {
        QUrlQuery ret;
        auto cookies = c->request()->cookies();
        auto it = cookies.constBegin();
        while (it != cookies.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryKeywords, :Local :AutoArgs)
    void queryKeywords(Context *c) {
        c->response()->setBody(c->request()->queryKeywords());
    }

    C_ATTR(uriWith, :Local :AutoArgs)
    void uriWith(Context *c, const QString &append) {
        c->response()->setBody(c->request()->uriWith(ParamsMultiMap{
                                                         {QStringLiteral("foo"),QStringLiteral("baz")},
                                                         {QStringLiteral("fooz"),QStringLiteral("bar")}
                                                     }, QVariant(append).toBool()).toString(QUrl::FullyEncoded));
    }

    C_ATTR(mangleParams, :Local :AutoArgs)
    void mangleParams(Context *c, const QString &append) {
        QUrlQuery ret;
        auto params = c->request()->mangleParams(c->request()->bodyParameters(), QVariant(append).toBool());
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(body, :Local :AutoArgs)
    void body(Context *c) {
        c->response()->setBody(c->request()->body()->readAll());
    }

    C_ATTR(queryParameters, :Local :AutoArgs)
    void queryParameters(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->queryParameters();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParams, :Local :AutoArgs)
    void queryParams(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->queryParams();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParametersVariant, :Local :AutoArgs)
    void queryParametersVariant(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->queryParametersVariant();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value().toString());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(queryParameter, :Local :AutoArgs)
    void queryParameter(Context *c, const QString &param, const QString &defaultValue) {
        c->response()->setBody(c->request()->queryParameter(param, defaultValue));
    }

    C_ATTR(queryParam, :Local :AutoArgs)
    void queryParam(Context *c, const QString &param, const QString &defaultValue) {
        c->response()->setBody(c->request()->queryParam(param, defaultValue));
    }

    C_ATTR(bodyParameters, :Local :AutoArgs)
    void bodyParameters(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->bodyParameters();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(bodyParams, :Local :AutoArgs)
    void bodyParams(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->bodyParams();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(bodyParameter, :Local :AutoArgs)
    void bodyParameter(Context *c) {
        c->response()->setBody(c->request()->bodyParameter(c->request()->queryParam(QStringLiteral("param")),
                                                           c->request()->queryParam(QStringLiteral("defaultValue"))));
    }

    C_ATTR(bodyParam, :Local :AutoArgs)
    void bodyParam(Context *c) {
        c->response()->setBody(c->request()->bodyParam(c->request()->queryParam(QStringLiteral("param")),
                                                       c->request()->queryParam(QStringLiteral("defaultValue"))));
    }

    C_ATTR(parameters, :Local :AutoArgs)
    void parameters(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->parameters();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(paramsKey, :Local :AutoArgs)
    void paramsKey(Context *c, const QString &param) {
        c->response()->setBody(c->request()->params(param).join(QLatin1Char('/')));
    }

    C_ATTR(params, :Local :AutoArgs)
    void params(Context *c) {
        QUrlQuery ret;
        auto params = c->request()->params();
        auto it = params.constBegin();
        while (it != params.constEnd()) {
            ret.addQueryItem(it.key(), it.value());
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(paramsKeyDefaultValue, :Local :AutoArgs)
    void paramsKeyDefaultValue(Context *c, const QString &param, const QString &defaultValue) {
        c->response()->setBody(c->request()->param(param, defaultValue));
    }

    C_ATTR(bodyData, :Local :AutoArgs)
    void bodyData(Context *c) {
        c->response()->setBody(QByteArray(c->request()->bodyData().typeName()));
    }

    C_ATTR(bodyDataJson, :Local :AutoArgs)
    void bodyDataJson(Context *c) {
        c->response()->setBody(c->request()->bodyData().toJsonDocument().toJson(QJsonDocument::Compact));
    }

    C_ATTR(uploads, :Local :AutoArgs)
    void uploads(Context *c) {
        QUrlQuery ret;
        QMap<QString, Upload *> uploads = c->request()->uploads();
        auto it = uploads.constBegin();
        while (it != uploads.constEnd()) {
            Upload *upload = it.value();
            ret.addQueryItem(it.key(), upload->name());
            ret.addQueryItem(it.key(), upload->filename());
            ret.addQueryItem(it.key(), upload->contentType());
            ret.addQueryItem(it.key(), QString::number(upload->size()));
            ret.addQueryItem(it.key(), QString::fromLatin1(QCryptographicHash::hash(upload->readAll(), QCryptographicHash::Sha256).toBase64()));
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(uploadsName, :Local :AutoArgs)
    void uploadsName(Context *c, const QString &name) {
        QUrlQuery ret;
        Uploads uploads = c->request()->uploads(name);
        auto it = uploads.constBegin();
        while (it != uploads.constEnd()) {
            Upload *upload = *it;
            ret.addQueryItem(upload->name(), upload->filename());
            ret.addQueryItem(upload->name(), upload->contentType());
            ret.addQueryItem(upload->name(), QString::number(upload->size()));
            ret.addQueryItem(upload->name(), QString::fromLatin1(QCryptographicHash::hash(upload->readAll(), QCryptographicHash::Sha256).toBase64()));
            ++it;
        }
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }

    C_ATTR(upload, :Local :AutoArgs)
    void upload(Context *c, const QString &name) {
        QUrlQuery ret;
        Upload *upload = c->request()->upload(name);
        ret.addQueryItem(upload->name(), upload->filename());
        ret.addQueryItem(upload->name(), upload->contentType());
        ret.addQueryItem(upload->name(), QString::number(upload->size()));
        ret.addQueryItem(upload->name(), QString::fromLatin1(QCryptographicHash::hash(upload->readAll(), QCryptographicHash::Sha256).toBase64()));
        c->response()->setBody(ret.toString(QUrl::FullyEncoded));
    }


};

void TestRequest::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestRequest::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new RequestTest(app);
    if (!engine->initEngine()) {
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
    QFETCH(QString, method);
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(method,
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 headers,
                                                 &body);

    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestRequest::testController_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    QString get = QStringLiteral("GET");
    QString post = QStringLiteral("POST");

    QUrlQuery query;
    Headers headers;
    QByteArray body;

    QTest::newRow("address-test00") << get << QStringLiteral("/request/test/address") << headers << QByteArray() << QByteArrayLiteral("127.0.0.1");
    QTest::newRow("hostname-test00") << get << QStringLiteral("/request/test/hostname") << headers << QByteArray()
                                     << QHostInfo::fromName(QStringLiteral("127.0.0.1")).hostName().toLatin1();
    QTest::newRow("port-test00") << get << QStringLiteral("/request/test/port") << headers << QByteArray() << QByteArrayLiteral("3000");
    QTest::newRow("uri-test00") << get << QStringLiteral("/request/test/uri") << headers << QByteArray() << QByteArrayLiteral("http://127.0.0.1/request/test/uri");
    QTest::newRow("base-test00") << get << QStringLiteral("/request/test/base") << headers << QByteArray() << QByteArrayLiteral("http://127.0.0.1/");
    QTest::newRow("path-test00") << get << QStringLiteral("/request/test/path") << headers << QByteArray() << QByteArrayLiteral("request/test/path");
    QTest::newRow("match-test00") << get << QStringLiteral("/request/test/match") << headers << QByteArray() << QByteArrayLiteral("request/test/match");

    QTest::newRow("method-test00") << get << QStringLiteral("/request/test/method") << headers << QByteArray() << QByteArrayLiteral("GET");
    QTest::newRow("method-test01") << post << QStringLiteral("/request/test/method") << headers << QByteArray() << QByteArrayLiteral("POST");
    QTest::newRow("method-test02") << QStringLiteral("HEAD") << QStringLiteral("/request/test/method") << headers << QByteArray() << QByteArrayLiteral("HEAD");

    QTest::newRow("isPost-test00") << get << QStringLiteral("/request/test/isPost") << headers << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isPost-test01") << QStringLiteral("PoSt") << QStringLiteral("/request/test/isPost") << headers << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isPost-test02") << post << QStringLiteral("/request/test/isPost") << headers << QByteArray() << QByteArrayLiteral("true");

    QTest::newRow("isGet-test00") << post << QStringLiteral("/request/test/isGet") << headers << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isGet-test01") << QStringLiteral("GeT") << QStringLiteral("/request/test/isGet") << headers << QByteArray() << QByteArrayLiteral("false");
    QTest::newRow("isGet-test02") << get << QStringLiteral("/request/test/isGet") << headers << QByteArray() << QByteArrayLiteral("true");

    QTest::newRow("protocol-test00") << get << QStringLiteral("/request/test/protocol") << headers << QByteArray() << QByteArrayLiteral("HTTP/1.1");
    QTest::newRow("remoteUser-test00") << get << QStringLiteral("/request/test/remoteUser") << headers << QByteArray() << QByteArrayLiteral("");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("bar"));
    headers.setReferer(QStringLiteral("http://www.cutelyst.org"));
    QTest::newRow("headers-test00") << get << QStringLiteral("/request/test/headers") << headers << QByteArray()
                                    << QByteArrayLiteral("authorization=Basic%20Zm9vOmJhcg%3D%3D&referer=http://www.cutelyst.org");

    headers.clear();
    headers.setHeader(QStringLiteral("User-Agent"), QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.79 Safari/537.36"));
    QTest::newRow("userAgent-test00") << get << QStringLiteral("/request/test/userAgent") << headers << QByteArray()
                                      << QByteArrayLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.79 Safari/537.36");

    headers.clear();
    headers.setHeader(QStringLiteral("Referer"), QStringLiteral("http://www.cutelyst.org"));
    QTest::newRow("referer-test00") << get << QStringLiteral("/request/test/referer") << headers << QByteArray()
                                    << QByteArrayLiteral("http://www.cutelyst.org");

    headers.clear();
    headers.setHeader(QStringLiteral("Content-Encoding"), QStringLiteral("gzip"));
    QTest::newRow("contentEncoding-test00") << get << QStringLiteral("/request/test/contentEncoding") << headers << QByteArray()
                                            << QByteArrayLiteral("gzip");

    headers.clear();
    headers.setHeader(QStringLiteral("Content-Type"), QStringLiteral("text/html; charset=UTF-8"));
    QTest::newRow("contentType-test00") << get << QStringLiteral("/request/test/contentType") << headers << QByteArray()
                                        << QByteArrayLiteral("text/html");


    query.clear();
    headers.setHeader(QStringLiteral("Cookie"), QString());
    QTest::newRow("cookies-test00") << get << QStringLiteral("/request/test/cookies")
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("");
    query.clear();
    headers.setHeader(QStringLiteral("Cookie"), QStringLiteral(""));
    QTest::newRow("cookies-test01") << get << QStringLiteral("/request/test/cookies")
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("");

    query.clear();
    headers.setHeader(QStringLiteral("Cookie"), QStringLiteral("FIRST=10186486272; SECOND=AF6bahuOZFc_P7-oCw; S=foo=TGp743-6uvY:first=MnBbT3wcrA-uy=MnwcrA:bla=0L7g; S=something=qjgNs_EA:more=n1Ki8xVsQ:andmore=FSg:andmore=nQMU_0VRlTJAbs_4fw:gmail=j4yGWsKuoZg"));
    QTest::newRow("cookies-test02") << get << QStringLiteral("/request/test/cookies")
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("FIRST=10186486272&S=foo%3DTGp743-6uvY:first%3DMnBbT3wcrA-uy%3DMnwcrA:bla%3D0L7g&S=something%3DqjgNs_EA:more%3Dn1Ki8xVsQ:andmore%3DFSg:andmore%3DnQMU_0VRlTJAbs_4fw:gmail%3Dj4yGWsKuoZg&SECOND=AF6bahuOZFc_P7-oCw");

    query.clear();
    headers.setHeader(QStringLiteral("Cookie"), QString());
    QTest::newRow("cookie-test00") << get << QStringLiteral("/request/test/cookie/foo")
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("");

    query.clear();
    headers.setHeader(QStringLiteral("Cookie"), QStringLiteral("FIRST=10186486272; SECOND=AF6bahuOZFc_P7-oCw; S=foo=TGp743-6uvY:first=MnBbT3wcrA-uy=MnwcrA:bla=0L7g; S=something=qjgNs_EA:more=n1Ki8xVsQ:andmore=FSg:andmore=nQMU_0VRlTJAbs_4fw:gmail=j4yGWsKuoZg"));
    QTest::newRow("cookie-test01") << get << QStringLiteral("/request/test/cookie/S")
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("S=foo%3DTGp743-6uvY:first%3DMnBbT3wcrA-uy%3DMnwcrA:bla%3D0L7g");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    QTest::newRow("queryKeywords-test00") << get << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
                                          << headers << QByteArray()
                                          << QByteArrayLiteral("some text to ask");
    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QStringLiteral("not"));
    QTest::newRow("queryKeywords-test01") << get << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
                                          << headers << QByteArray()
                                          << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryKeywords-test02") << get << QStringLiteral("/request/test/queryKeywords?") + query.toString(QUrl::FullyEncoded)
                                          << headers << QByteArray()
                                          << QByteArrayLiteral("some text to ask&another keyword&and yet another is fine");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriWith-test00") << get << QStringLiteral("/request/test/uriWith/false?") + query.toString(QUrl::FullyEncoded)
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("http://127.0.0.1/request/test/uriWith/false?foo=baz&fooz=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("bar"));
    QTest::newRow("uriWith-test01") << get << QStringLiteral("/request/test/uriWith/true?") + query.toString(QUrl::FullyEncoded)
                                    << headers << QByteArray()
                                    << QByteArrayLiteral("http://127.0.0.1/request/test/uriWith/true?foo=bar&foo=baz&fooz=bar");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baz"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("mangleParams-test00") << get << QStringLiteral("/request/test/mangleParams/false?foo=bar&x=y")
                                         << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                         << QByteArrayLiteral("foo=baz&x=y");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("baz"));
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("mangleParams-test01") << get << QStringLiteral("/request/test/mangleParams/true?foo=bar&x=y")
                                         << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                         << QByteArrayLiteral("foo=bar&foo=baz&x=y");

    query.clear();
    QTest::newRow("body-test00") << get << QStringLiteral("/request/test/body")
                                 << headers << QByteArray() << QByteArray();

    query.clear();
    body = QUuid::createUuid().toByteArray();
    QTest::newRow("body-test00") << get << QStringLiteral("/request/test/body")
                                 << headers << body << body;

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParameters-test00") << get << QStringLiteral("/request/test/queryParameters?") + query.toString(QUrl::FullyEncoded)
                                            << headers << QByteArray()
                                            << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParameters-test01") << get << QStringLiteral("/request/test/queryParameters?") + query.toString(QUrl::FullyEncoded)
                                            << headers << QByteArray()
                                            << QByteArrayLiteral("and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParams-test00") << get << QStringLiteral("/request/test/queryParams?") + query.toString(QUrl::FullyEncoded)
                                        << headers << QByteArray()
                                        << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParams-test01") << get << QStringLiteral("/request/test/queryParams?") + query.toString(QUrl::FullyEncoded)
                                        << headers << QByteArray()
                                        << QByteArrayLiteral("and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParametersVariant-test00") << get << QStringLiteral("/request/test/queryParametersVariant?") + query.toString(QUrl::FullyEncoded)
                                                   << headers << QByteArray()
                                                   << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    QTest::newRow("queryParametersVariant-test01") << get << QStringLiteral("/request/test/queryParametersVariant?") + query.toString(QUrl::FullyEncoded)
                                                   << headers << QByteArray()
                                                   << QByteArrayLiteral("and%20yet%20another%20is%20fine&another%20keyword&bar=baz&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParameter-test00") << get << QStringLiteral("/request/test/queryParameter/foo/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                           << headers << QByteArray()
                                           << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParameter-test01") << get << QStringLiteral("/request/test/queryParameter/x/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                           << headers << QByteArray()
                                           << QByteArrayLiteral("");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParameter-test02") << get << QStringLiteral("/request/test/queryParameter/y/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                           << headers << QByteArray()
                                           << QByteArrayLiteral("gotDefault");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    QTest::newRow("queryParam-test00") << get << QStringLiteral("/request/test/queryParam/foo/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                       << headers << QByteArray()
                                       << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParam-test01") << get << QStringLiteral("/request/test/queryParam/x/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                       << headers << QByteArray()
                                       << QByteArrayLiteral("");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    QTest::newRow("queryParam-test02") << get << QStringLiteral("/request/test/queryParam/y/gotDefault?") + query.toString(QUrl::FullyEncoded)
                                       << headers << QByteArray()
                                       << QByteArrayLiteral("gotDefault");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyParameters-test00") << get << QStringLiteral("/request/test/bodyParameters")
                                           << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                           << QByteArrayLiteral("and%20yet%20another%20is%20fine&another%20keyword&some%20text%20to%20ask");

    query.clear();
    query.addQueryItem(QStringLiteral("some text to ask"), QString());
    query.addQueryItem(QStringLiteral("another keyword"), QString());
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyParams-test00") << get << QStringLiteral("/request/test/bodyParams")
                                       << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                       << QByteArrayLiteral("and%20yet%20another%20is%20fine&another%20keyword&some%20text%20to%20ask");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("and yet another is fine"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyParam-test00") << get << QStringLiteral("/request/test/bodyParam?param=foo")
                                      << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                      << body;

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyParam-test01") << get << QStringLiteral("/request/test/bodyParam?param=y&defaultValue=SomeDefaultValue")
                                      << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                      << QByteArrayLiteral("SomeDefaultValue");

    query.clear();
    body = QUuid::createUuid().toByteArray();
    query.addQueryItem(QStringLiteral("foo"), QString::fromLatin1(body));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyParam-test02") << get << QStringLiteral("/request/test/bodyParam?param=x&defaultValue=SomeDefaultValue")
                                      << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                      << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("parameters-test00") << get << QStringLiteral("/request/test/parameters?param=y&defaultValue=SomeDefaultValue")
                                       << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                       << QByteArrayLiteral("bar=baz&defaultValue=SomeDefaultValue&foo=Cutelyst&param=y&x");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("params-test00") << get << QStringLiteral("/request/test/params?param=y&defaultValue=SomeDefaultValue")
                                   << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                   << QByteArrayLiteral("bar=baz&defaultValue=SomeDefaultValue&foo=Cutelyst&param=y&x");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("paramsKey-test00") << get << QStringLiteral("/request/test/paramsKey/foo?param=y&defaultValue=SomeDefaultValue")
                                      << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                      << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("paramsKey-test00") << get << QStringLiteral("/request/test/paramsKey/x?param=y&defaultValue=SomeDefaultValue")
                                      << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                      << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("paramsKeyDefaultValue-test00") << get << QStringLiteral("/request/test/paramsKeyDefaultValue/foo/bar?param=y&defaultValue=SomeDefaultValue")
                                                  << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                                  << QByteArrayLiteral("Cutelyst");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("paramsKeyDefaultValue-test01") << get << QStringLiteral("/request/test/paramsKeyDefaultValue/x/gotDefault?param=y&defaultValue=SomeDefaultValue")
                                                  << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                                  << QByteArrayLiteral("");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("paramsKeyDefaultValue-test01") << get << QStringLiteral("/request/test/paramsKeyDefaultValue/z/gotDefault?param=y&defaultValue=SomeDefaultValue")
                                                  << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                                  << QByteArrayLiteral("gotDefault");

    query.clear();
    query.addQueryItem(QStringLiteral("foo"), QStringLiteral("Cutelyst"));
    query.addQueryItem(QStringLiteral("bar"), QStringLiteral("baz"));
    query.addQueryItem(QStringLiteral("x"), QString());
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyData-test01") << get << QStringLiteral("/request/test/bodyData")
                                     << headers << query.toString(QUrl::FullyEncoded).toLatin1()
                                     << QByteArrayLiteral("Cutelyst::ParamsMultiMap");

    query.clear();
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyData-test02") << get << QStringLiteral("/request/test/bodyData")
                                     << headers << QByteArray()
                                     << QByteArrayLiteral("Cutelyst::ParamsMultiMap");

    query.clear();
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QTest::newRow("bodyData-test03") << get << QStringLiteral("/request/test/bodyData")
                                     << headers << QByteArray()
                                     << QByteArrayLiteral("Cutelyst::ParamsMultiMap");

    query.clear();
    headers.setContentType(QStringLiteral("application/json"));
    QTest::newRow("bodyData-test04") << get << QStringLiteral("/request/test/bodyData")
                                     << headers << QByteArray()
                                     << QByteArrayLiteral("QJsonDocument");

    query.clear();
    headers.setContentType(QStringLiteral("multipart/form-data"));
    QTest::newRow("bodyData-test05") << get << QStringLiteral("/request/test/bodyData")
                                     << headers << QByteArray()
                                     << QByteArrayLiteral("QMap<QString,Cutelyst::Upload*>");

    query.clear();
    QJsonObject obj;
    obj.insert(QStringLiteral("foo"), QStringLiteral("bar"));
    QJsonArray array;
    array.append(obj);
    headers.setContentType(QStringLiteral("application/json"));
    QTest::newRow("bodyDataJson-test00") << get << QStringLiteral("/request/test/bodyDataJson")
                                         << headers << QJsonDocument(array).toJson(QJsonDocument::Compact)
                                         << QByteArrayLiteral("[{\"foo\":\"bar\"}]");

    query.clear();
    headers.setContentType(QStringLiteral("multipart/form-data; boundary=WebKitFormBoundaryoPPQLwBBssFnOTVH"));
    QTest::newRow("uploads-test00") << get << QStringLiteral("/request/test/uploads")
                                    << headers << QByteArrayLiteral("------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"path\"\r\n\r\ntextooooo\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"wifi\"\r\nContent-Type: application/octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: application/octet-stream\r\n\r\nhttps://example.com/admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
                                    << QByteArrayLiteral("file1=file1&file1=wifi&file1=application/octet-stream&file1=27&file1=WJuOfAGaYV4qdMH4goQ3/DvHCjoJVLeQv52++NESsfo%3D&file1=file1&file1=example.txt&file1=application/octet-stream&file1=31&file1=3LPlbWl4PsPXNDXvOfTNkTewkm6vhtNrMGjXz3H433Q%3D&path=path&path&path&path=13&path=JhOwXPadgbn3jGlWnq/lmbEZ1HiI4WjarTZ1YKoeXfI%3D");

    query.clear();
    headers.setContentType(QStringLiteral("multipart/form-data; boundary=WebKitFormBoundaryoPPQLwBBssFnOTVH"));
    QTest::newRow("uploadsName-test00") << get << QStringLiteral("/request/test/uploadsName/file1")
                                        << headers << QByteArrayLiteral("------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"path\"\r\n\r\ntextooooo\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"wifi\"\r\nContent-Type: application/octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: application/octet-stream\r\n\r\nhttps://example.com/admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
                                        << QByteArrayLiteral("file1=wifi&file1=application/octet-stream&file1=27&file1=WJuOfAGaYV4qdMH4goQ3/DvHCjoJVLeQv52++NESsfo%3D&file1=example.txt&file1=application/octet-stream&file1=31&file1=3LPlbWl4PsPXNDXvOfTNkTewkm6vhtNrMGjXz3H433Q%3D");

    query.clear();
    headers.setContentType(QStringLiteral("multipart/form-data; boundary=WebKitFormBoundaryoPPQLwBBssFnOTVH"));
    QTest::newRow("upload-test00") << get << QStringLiteral("/request/test/upload/file1")
                                   << headers << QByteArrayLiteral("------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"path\"\r\n\r\ntextooooo\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"wifi\"\r\nContent-Type: application/octet-stream\r\n\r\nMOTOCM\nMOTOCM\n00000000\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\nContent-Type: application/octet-stream\r\n\r\nhttps://example.com/admin\n\n\r\n------WebKitFormBoundaryoPPQLwBBssFnOTVH--\r\n")
                                   << QByteArrayLiteral("file1=wifi&file1=application/octet-stream&file1=27&file1=WJuOfAGaYV4qdMH4goQ3/DvHCjoJVLeQv52++NESsfo%3D");
}

QTEST_MAIN(TestRequest)

#include "testrequest.moc"

#endif
