#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationrealm.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Authentication/credentialhttp.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/minimal.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QNetworkCookie>
#include <QObject>
#include <QTest>
#include <QUrlQuery>

using namespace Cutelyst;

class TestAuthentication : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestAuthentication(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void cleanupTestCase();

private:
    TestEngine *m_engine = nullptr;

    TestEngine *getEngine();

    void doTest();
};

class AuthenticationTest : public Controller
{
    Q_OBJECT
public:
    explicit AuthenticationTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(authenticate, :Local :AutoArgs)
    void authenticate(Context *c)
    {
        if (Authentication::authenticate(c)) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_realm, :Local :AutoArgs)
    void authenticate_realm(Context *c, const QString &realm)
    {
        if (Authentication::authenticate(c, realm)) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_user, :Local :AutoArgs)
    void authenticate_user(Context *c)
    {
        if (Authentication::authenticate(c, c->request()->queryParameters())) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_user_realm, :Local :AutoArgs)
    void authenticate_user_realm(Context *c, const QString &realm)
    {
        if (Authentication::authenticate(c, c->request()->queryParameters(), realm)) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_user_exists, :Local :AutoArgs)
    void authenticate_user_exists(Context *c, const QString &realm)
    {
        if (!Authentication::userExists(c) &&
            Authentication::authenticate(c, c->request()->queryParameters(), realm) &&
            Authentication::userExists(c)) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_user_obj, :Local :AutoArgs)
    void authenticate_user_obj(Context *c, const QString &realm)
    {
        if (Authentication::user(c).isNull() &&
            Authentication::authenticate(c, c->request()->queryParameters(), realm) &&
            !Authentication::user(c).id().isNull()) {
            c->response()->setBody(u"ok"_s);
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }

    C_ATTR(authenticate_user_logout, :Local :AutoArgs)
    void authenticate_user_logout(Context *c, const QString &realm)
    {
        if (!Authentication::userExists(c) &&
            Authentication::authenticate(c, c->request()->queryParameters(), realm) &&
            Authentication::userExists(c)) {
            Authentication::logout(c);
            if (!Authentication::userExists(c)) {
                c->response()->setBody(u"ok"_s);
                return;
            }
        }
        c->response()->setBody(u"fail"_s);
    }

    C_ATTR(authenticate_user_cookie, :Local :AutoArgs)
    void authenticate_user_cookie(Context *c, const QString &realm)
    {
        std::ignore       = Authentication::authenticate(c, c->request()->queryParameters(), realm);
        const auto cookie = c->response()
                                ->cookie(QByteArrayLiteral("testauthentication_exec_session"))
                                .value<QNetworkCookie>();
        if (cookie.isHttpOnly() && cookie.path().compare(u"/") == 0) {
            c->response()->setBody(u"ok"_s);
            return;
        } else {
            c->response()->setBody(u"fail"_s);
        }
    }
};

void TestAuthentication::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestAuthentication::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());

    auto auth = new Authentication(app);

    auto clearStore = std::make_shared<StoreMinimal>(u"id"_s);
    {
        AuthenticationUser fooUser(u"foo"_s);
        fooUser.insert(u"password"_s, u"123"_s);
        clearStore->addUser(fooUser);

        AuthenticationUser barUser(u"bar"_s);
        barUser.insert(u"password"_s, u"321"_s);
        clearStore->addUser(barUser);
    }
    auto clearPassword = std::make_shared<CredentialPassword>();
    clearPassword->setPasswordField(u"password"_s);
    clearPassword->setPasswordType(CredentialPassword::Clear);
    auth->addRealm(std::make_shared<AuthenticationRealm>(clearStore, clearPassword));

    const auto preSalt  = QStringLiteral(u"preSalt");
    const auto postSalt = QStringLiteral(u"postSalt");

    auto hashedStore = std::make_shared<StoreMinimal>(u"id"_s);
    {
        AuthenticationUser fooUser(u"foo"_s);
        fooUser.insert(u"password"_s,
                       CredentialPassword::createPassword(preSalt + u"123" + postSalt));
        hashedStore->addUser(fooUser);

        AuthenticationUser barUser(u"bar"_s);
        barUser.insert(u"password"_s,
                       CredentialPassword::createPassword(preSalt + u"321" + postSalt));
        hashedStore->addUser(barUser);
    }

    auto hashedPassword = std::make_shared<CredentialPassword>();
    hashedPassword->setPasswordField(u"password"_s);
    hashedPassword->setPasswordType(CredentialPassword::Hashed);
    hashedPassword->setPasswordPreSalt(preSalt);
    hashedPassword->setPasswordPostSalt(postSalt);

    auth->addRealm(std::make_shared<AuthenticationRealm>(hashedStore, hashedPassword, u"hashed"_s));

    auto nonePassword = std::make_shared<CredentialPassword>();
    nonePassword->setPasswordField(u"password"_s);
    nonePassword->setPasswordType(CredentialPassword::None);
    auth->addRealm(std::make_shared<AuthenticationRealm>(clearStore, nonePassword, u"none"_s));

    auto clearHttpCredential = std::make_shared<CredentialHttp>();
    clearHttpCredential->setPasswordType(CredentialHttp::Clear);
    clearHttpCredential->setUsernameField(u"id"_s);
    auth->addRealm(
        std::make_shared<AuthenticationRealm>(clearStore, clearHttpCredential, u"httpClear"_s));

    auto hashedHttpCredential = std::make_shared<CredentialHttp>();
    hashedHttpCredential->setPasswordType(CredentialHttp::Hashed);
    hashedHttpCredential->setUsernameField(u"id"_s);
    hashedHttpCredential->setPasswordPreSalt(preSalt);
    hashedHttpCredential->setPasswordPostSalt(postSalt);
    auth->addRealm(hashedStore, hashedHttpCredential, u"httpHashed"_s);

    auto noneHttpCredential = std::make_shared<CredentialHttp>();
    noneHttpCredential->setPasswordType(CredentialHttp::None);
    noneHttpCredential->setUsernameField(u"id"_s);
    auth->addRealm(clearStore, noneHttpCredential, u"httpNone"_s);

    new Session(app);

    new AuthenticationTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestAuthentication::cleanupTestCase()
{
    delete m_engine;
}

void TestAuthentication::doTest()
{
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(int, status);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), headers, nullptr);

    QCOMPARE(result.statusCode, status);
    QCOMPARE(result.body, output);
}

void TestAuthentication::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    // Auth based on query data
    Headers headers;
    QUrlQuery query;
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-test00") << u"/authentication/test/authenticate?"_s +
                                        query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"321"_s);
    QTest::newRow("auth-test01") << u"/authentication/test/authenticate?"_s +
                                        query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-user-test00")
        << u"/authentication/test/authenticate_user?"_s + query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"321"_s);
    QTest::newRow("auth-user-test01")
        << u"/authentication/test/authenticate_user?"_s + query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(u"id"_s, u"bar"_s);
    query.addQueryItem(u"password"_s, u"321"_s);
    QTest::newRow("auth-user-test02")
        << u"/authentication/test/authenticate_user?"_s + query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(u"id"_s, u"bar"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-user-test03")
        << u"/authentication/test/authenticate_user?"_s + query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-user-realm-test00")
        << u"/authentication/test/authenticate_user_realm/hashed?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"321"_s);
    QTest::newRow("auth-user-realm-test01")
        << u"/authentication/test/authenticate_user_realm/hashed?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");
    query.clear();
    query.addQueryItem(u"id"_s, u"bar"_s);
    query.addQueryItem(u"password"_s, u"321"_s);
    QTest::newRow("auth-user-realm-test02")
        << u"/authentication/test/authenticate_user_realm/hashed?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(u"id"_s, u"bar"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-user-realm-test03")
        << u"/authentication/test/authenticate_user_realm/hashed?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"123"_s);
    QTest::newRow("auth-user-realm-test04")
        << u"/authentication/test/authenticate_user_realm/none?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(u"id"_s, u"foo"_s);
    query.addQueryItem(u"password"_s, u"3212134324234324"_s);
    QTest::newRow("auth-user-realm-test05")
        << u"/authentication/test/authenticate_user_realm/none?"_s +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    // HTTP auth
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-http-user-realm-test00")
        << u"/authentication/test/authenticate_user_realm/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-http-user-realm-test01")
        << u"/authentication/test/authenticate_user_realm/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(u"bar"_s, u"321"_s);
    QTest::newRow("auth-http-user-realm-test02")
        << u"/authentication/test/authenticate_user_realm/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(u"bar"_s, u"123"_s);
    QTest::newRow("auth-http-user-realm-test03")
        << u"/authentication/test/authenticate_user_realm/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-http-user-realm-test04")
        << u"/authentication/test/authenticate_user_realm/httpNone"_s << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"3212134324234324"_s);
    QTest::newRow("auth-http-user-realm-test05")
        << u"/authentication/test/authenticate_user_realm/httpNone"_s << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-http-realm-test00")
        << u"/authentication/test/authenticate_realm/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-http-realm-test01")
        << u"/authentication/test/authenticate_realm/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(u"bar"_s, u"321"_s);
    QTest::newRow("auth-http-realm-test02")
        << u"/authentication/test/authenticate_realm/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"bar"_s, u"123"_s);
    QTest::newRow("auth-http-realm-test03")
        << u"/authentication/test/authenticate_realm/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-http-realm-test04") << u"/authentication/test/authenticate_realm/httpNone"_s
                                            << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"3212134324234324"_s);
    QTest::newRow("auth-http-realm-test05") << u"/authentication/test/authenticate_realm/httpNone"_s
                                            << headers << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-user-exists-test00")
        << u"/authentication/test/authenticate_user_exists/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-user-user-test01")
        << u"/authentication/test/authenticate_user_exists/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-user-obj-test00")
        << u"/authentication/test/authenticate_user_obj/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-user-obj-test01")
        << u"/authentication/test/authenticate_user_obj/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-user-logout-test00")
        << u"/authentication/test/authenticate_user_logout/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-user-logout-test01")
        << u"/authentication/test/authenticate_user_logout/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"123"_s);
    QTest::newRow("auth-user-cookie-test00")
        << u"/authentication/test/authenticate_user_cookie/httpHashed"_s << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(u"foo"_s, u"321"_s);
    QTest::newRow("auth-user-cookie-test01")
        << u"/authentication/test/authenticate_user_cookie/httpHashed"_s << headers << 401
        << QByteArrayLiteral("fail");
}

QTEST_MAIN(TestAuthentication)

#include "testauthentication.moc"

#endif
