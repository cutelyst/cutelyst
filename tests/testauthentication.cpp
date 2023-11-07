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
    TestEngine *m_engine;

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
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_realm, :Local :AutoArgs)
    void authenticate_realm(Context *c, const QString &realm)
    {
        if (Authentication::authenticate(c, realm)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user, :Local :AutoArgs)
    void authenticate_user(Context *c)
    {
        if (Authentication::authenticate(c, c->request()->queryParameters())) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_realm, :Local :AutoArgs)
    void authenticate_user_realm(Context *c, const QString &realm)
    {
        if (Authentication::authenticate(c, c->request()->queryParameters(), realm)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_exists, :Local :AutoArgs)
    void authenticate_user_exists(Context *c, const QString &realm)
    {
        if (!Authentication::userExists(c) &&
            Authentication::authenticate(c, c->request()->queryParameters(), realm) &&
            Authentication::userExists(c)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_obj, :Local :AutoArgs)
    void authenticate_user_obj(Context *c, const QString &realm)
    {
        if (Authentication::user(c).isNull() &&
            Authentication::authenticate(c, c->request()->queryParameters(), realm) &&
            !Authentication::user(c).id().isNull()) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
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
                c->response()->setBody(QStringLiteral("ok"));
                return;
            }
        }
        c->response()->setBody(QStringLiteral("fail"));
    }

    C_ATTR(authenticate_user_cookie, :Local :AutoArgs)
    void authenticate_user_cookie(Context *c, const QString &realm)
    {
        std::ignore       = Authentication::authenticate(c, c->request()->queryParameters(), realm);
        const auto cookie = c->response()
                                ->cookie(QByteArrayLiteral("testauthentication_exec_session"))
                                .value<QNetworkCookie>();
        if (cookie.isHttpOnly() && cookie.path().compare(u"/") == 0) {
            c->response()->setBody(QStringLiteral("ok"));
            return;
        } else {
            c->response()->setBody(QStringLiteral("fail"));
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

    auto clearStore = std::make_shared<StoreMinimal>(QStringLiteral("id"));
    {
        AuthenticationUser fooUser(QStringLiteral("foo"));
        fooUser.insert(QStringLiteral("password"), QStringLiteral("123"));
        clearStore->addUser(fooUser);

        AuthenticationUser barUser(QStringLiteral("bar"));
        barUser.insert(QStringLiteral("password"), QStringLiteral("321"));
        clearStore->addUser(barUser);
    }
    auto clearPassword = std::make_shared<CredentialPassword>();
    clearPassword->setPasswordField(QStringLiteral("password"));
    clearPassword->setPasswordType(CredentialPassword::Clear);
    auth->addRealm(std::make_shared<AuthenticationRealm>(clearStore, clearPassword));

    const auto preSalt  = u"preSalt"_qs;
    const auto postSalt = u"postSalt"_qs;

    auto hashedStore = std::make_shared<StoreMinimal>(QStringLiteral("id"));
    {
        AuthenticationUser fooUser(QStringLiteral("foo"));
        fooUser.insert(QStringLiteral("password"),
                       CredentialPassword::createPassword(preSalt + u"123" + postSalt));
        hashedStore->addUser(fooUser);

        AuthenticationUser barUser(QStringLiteral("bar"));
        barUser.insert(QStringLiteral("password"),
                       CredentialPassword::createPassword(preSalt + u"321" + postSalt));
        hashedStore->addUser(barUser);
    }

    auto hashedPassword = std::make_shared<CredentialPassword>();
    hashedPassword->setPasswordField(QStringLiteral("password"));
    hashedPassword->setPasswordType(CredentialPassword::Hashed);
    hashedPassword->setPasswordPreSalt(preSalt);
    hashedPassword->setPasswordPostSalt(postSalt);

    auth->addRealm(std::make_shared<AuthenticationRealm>(
        hashedStore, hashedPassword, QStringLiteral("hashed")));

    auto nonePassword = std::make_shared<CredentialPassword>();
    nonePassword->setPasswordField(QStringLiteral("password"));
    nonePassword->setPasswordType(CredentialPassword::None);
    auth->addRealm(
        std::make_shared<AuthenticationRealm>(clearStore, nonePassword, QStringLiteral("none")));

    auto clearHttpCredential = std::make_shared<CredentialHttp>();
    clearHttpCredential->setPasswordType(CredentialHttp::Clear);
    clearHttpCredential->setUsernameField(QStringLiteral("id"));
    auth->addRealm(std::make_shared<AuthenticationRealm>(
        clearStore, clearHttpCredential, QStringLiteral("httpClear")));

    auto hashedHttpCredential = std::make_shared<CredentialHttp>();
    hashedHttpCredential->setPasswordType(CredentialHttp::Hashed);
    hashedHttpCredential->setUsernameField(QStringLiteral("id"));
    hashedHttpCredential->setPasswordPreSalt(preSalt);
    hashedHttpCredential->setPasswordPostSalt(postSalt);
    auth->addRealm(hashedStore, hashedHttpCredential, QStringLiteral("httpHashed"));

    auto noneHttpCredential = std::make_shared<CredentialHttp>();
    noneHttpCredential->setPasswordType(CredentialHttp::None);
    noneHttpCredential->setUsernameField(QStringLiteral("id"));
    auth->addRealm(clearStore, noneHttpCredential, QStringLiteral("httpNone"));

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
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-test00") << QStringLiteral("/authentication/test/authenticate?") +
                                        query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-test01") << QStringLiteral("/authentication/test/authenticate?") +
                                        query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-test00") << QStringLiteral("/authentication/test/authenticate_user?") +
                                             query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-test01") << QStringLiteral("/authentication/test/authenticate_user?") +
                                             query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-test02") << QStringLiteral("/authentication/test/authenticate_user?") +
                                             query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-test03") << QStringLiteral("/authentication/test/authenticate_user?") +
                                             query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test00")
        << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-realm-test01")
        << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");
    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-realm-test02")
        << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test03")
        << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test04")
        << QStringLiteral("/authentication/test/authenticate_user_realm/none?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-user-realm-test05")
        << QStringLiteral("/authentication/test/authenticate_user_realm/none?") +
               query.toString(QUrl::FullyEncoded)
        << headers << 200 << QByteArrayLiteral("ok");

    // HTTP auth
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test00")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed") << headers
        << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-http-user-realm-test01")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed") << headers
        << 401 << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("321"));
    QTest::newRow("auth-http-user-realm-test02")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed") << headers
        << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test03")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed") << headers
        << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test04")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpNone") << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-http-user-realm-test05")
        << QStringLiteral("/authentication/test/authenticate_user_realm/httpNone") << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test00")
        << QStringLiteral("/authentication/test/authenticate_realm/httpHashed") << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-http-realm-test01")
        << QStringLiteral("/authentication/test/authenticate_realm/httpHashed") << headers << 401
        << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("321"));
    QTest::newRow("auth-http-realm-test02")
        << QStringLiteral("/authentication/test/authenticate_realm/httpHashed") << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test03")
        << QStringLiteral("/authentication/test/authenticate_realm/httpHashed") << headers << 401
        << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test04")
        << QStringLiteral("/authentication/test/authenticate_realm/httpNone") << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-http-realm-test05")
        << QStringLiteral("/authentication/test/authenticate_realm/httpNone") << headers << 200
        << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-exists-test00")
        << QStringLiteral("/authentication/test/authenticate_user_exists/httpHashed") << headers
        << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-user-test01")
        << QStringLiteral("/authentication/test/authenticate_user_exists/httpHashed") << headers
        << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-obj-test00")
        << QStringLiteral("/authentication/test/authenticate_user_obj/httpHashed") << headers << 200
        << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-obj-test01")
        << QStringLiteral("/authentication/test/authenticate_user_obj/httpHashed") << headers << 401
        << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-logout-test00")
        << QStringLiteral("/authentication/test/authenticate_user_logout/httpHashed") << headers
        << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-logout-test01")
        << QStringLiteral("/authentication/test/authenticate_user_logout/httpHashed") << headers
        << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-cookie-test00")
        << QStringLiteral("/authentication/test/authenticate_user_cookie/httpHashed") << headers
        << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-cookie-test01")
        << QStringLiteral("/authentication/test/authenticate_user_cookie/httpHashed") << headers
        << 401 << QByteArrayLiteral("fail");
}

QTEST_MAIN(TestAuthentication)

#include "testauthentication.moc"

#endif
