#ifndef COOKIETEST_H
#define COOKIETEST_H

#include "coverageobject.h"

#include <Cutelyst/cookie.h>

#include <QLocale>
#include <QTest>

using namespace Cutelyst;

class TestCookie : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestCookie(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void testDefaultConstructor();
    void testConstructorWithArgs();
    void testSetSameSite();
    void testCopy();
    void testMove();
    void testCompare();
    void testToRawForm();
    void testToRawForm_data();
};

void TestCookie::testDefaultConstructor()
{
    Cookie c;
    QVERIFY(c.domain().isEmpty());
    QVERIFY(!c.isHttpOnly());
    QVERIFY(!c.isSecure());
    QVERIFY(c.isSessionCookie());
    QVERIFY(c.name().isEmpty());
    QVERIFY(c.path().isEmpty());
    QCOMPARE(c.sameSitePolicy(), Cookie::SameSite::Default);
    QVERIFY(c.value().isEmpty());
}

void TestCookie::testConstructorWithArgs()
{
    Cookie c(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
    QVERIFY(c.domain().isEmpty());
    QVERIFY(!c.isHttpOnly());
    QVERIFY(!c.isSecure());
    QVERIFY(c.isSessionCookie());
    QCOMPARE(c.name(), QByteArrayLiteral("foo"));
    QVERIFY(c.path().isEmpty());
    QCOMPARE(c.sameSitePolicy(), Cookie::SameSite::Default);
    QCOMPARE(c.value(), QByteArrayLiteral("bar"));
}

void TestCookie::testSetSameSite()
{
    Cookie c(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
    c.setSameSitePolicy(Cookie::SameSite::Strict);
    QCOMPARE(c.sameSitePolicy(), Cookie::SameSite::Strict);
}

void TestCookie::testCopy()
{
    // test copy constructor
    {
        Cookie c1(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
        c1.setPath(QStringLiteral("/"));
        c1.setSameSitePolicy(Cookie::SameSite::Strict);
        Cookie c2(c1);

        QCOMPARE(c1.name(), c2.name());
        QCOMPARE(c1.value(), c2.value());
        QCOMPARE(c1.path(), c2.path());
        QCOMPARE(c1.sameSitePolicy(), c2.sameSitePolicy());
    }

    // test copy assignment
    {
        Cookie c1(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
        c1.setPath(QStringLiteral("/"));
        c1.setSameSitePolicy(Cookie::SameSite::Strict);
        Cookie c2;
        c2 = c1;

        QCOMPARE(c1.name(), c2.name());
        QCOMPARE(c1.value(), c2.value());
        QCOMPARE(c1.path(), c2.path());
        QCOMPARE(c1.sameSitePolicy(), c2.sameSitePolicy());
    }
}

void TestCookie::testMove()
{
    // test move assignment
    {
        Cookie c1(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
        c1.setPath(QStringLiteral("/"));
        c1.setSameSitePolicy(Cookie::SameSite::Strict);
        Cookie c2(QByteArrayLiteral("key"), QByteArrayLiteral("val"));
        c2.setPath(QStringLiteral("/path"));
        c2.setSameSitePolicy(Cookie::SameSite::Lax);
        c2 = std::move(c1);

        QCOMPARE(c2.name(), QByteArrayLiteral("foo"));
        QCOMPARE(c2.value(), QByteArrayLiteral("bar"));
        QCOMPARE(c2.path(), QStringLiteral("/"));
        QCOMPARE(c2.sameSitePolicy(), Cookie::SameSite::Strict);
    }
}

void TestCookie::testCompare()
{
    Cookie c1(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
    c1.setPath(QStringLiteral("/"));
    c1.setSameSitePolicy(Cookie::SameSite::Strict);
    Cookie c2(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
    c2.setPath(QStringLiteral("/"));
    c2.setSameSitePolicy(Cookie::SameSite::Strict);
    Cookie c3 = c1;
    Cookie c4(QByteArrayLiteral("key"), QByteArrayLiteral("val"));
    c4.setPath(QStringLiteral("/path"));
    c4.setSameSitePolicy(Cookie::SameSite::Lax);

    QVERIFY(c1 == c2);
    QVERIFY(c1 == c3);
    QVERIFY(c1 != c4);
}

void TestCookie::testToRawForm()
{
    QFETCH(Cookie, cookie);
    QFETCH(QByteArray, result);

    QCOMPARE(cookie.toRawForm(), result);
}

void TestCookie::testToRawForm_data()
{
    QTest::addColumn<Cookie>("cookie");
    QTest::addColumn<QByteArray>("result");

    Cookie c(QByteArrayLiteral("foo"), QByteArrayLiteral("bar"));
    QTest::newRow("1") << c << QByteArrayLiteral("foo=bar");

    c.setSecure(true);
    QTest::newRow("2") << c << QByteArrayLiteral("foo=bar; secure");

    c.setHttpOnly(true);
    QTest::newRow("3") << c << QByteArrayLiteral("foo=bar; secure; HttpOnly");

    c.setSameSitePolicy(Cookie::SameSite::Strict);
    QTest::newRow("4") << c << QByteArrayLiteral("foo=bar; secure; HttpOnly; SameSite=Strict");

    const auto expire = QDateTime::currentDateTimeUtc().addDays(1);
    c.setExpirationDate(expire);
    QByteArray result = QByteArrayLiteral("foo=bar; secure; HttpOnly; SameSite=Strict; expires=");
    result.append(
        QLocale::c().toString(expire, QStringLiteral("ddd, dd-MMM-yyyy hh:mm:ss 'GMT")).toLatin1());
    QTest::newRow("5") << c << result;

    c.setExpirationDate(QDateTime());
    c.setDomain(QStringLiteral("example.net"));
    QTest::newRow("6") << c
                       << QByteArrayLiteral(
                              "foo=bar; secure; HttpOnly; SameSite=Strict; domain=example.net");

    c.setPath(QStringLiteral("/somewhere"));
    QTest::newRow("7")
        << c
        << QByteArrayLiteral(
               "foo=bar; secure; HttpOnly; SameSite=Strict; domain=example.net; path=/somewhere");
}

QTEST_MAIN(TestCookie)

#include "testcookie.moc"

#endif
