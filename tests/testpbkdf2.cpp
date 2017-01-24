#ifndef PBKDF2TEST_H
#define PBKDF2TEST_H

#include <QtTest/QTest>
#include <QtCore/QObject>

#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include "coverageobject.h"

// Based on https://tools.ietf.org/html/rfc6070

using namespace Cutelyst;

class TestPbkdf2 : public CoverageObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPbkdf2_data();
    void testPbkdf2() {
        doTest();
    }

private:
    void doTest();
};

void TestPbkdf2::doTest()
{
    QFETCH(QByteArray, password);
    QFETCH(QByteArray, salt);
    QFETCH(int, count);
    QFETCH(int, keyLength);
    QFETCH(QByteArray, result);

    const QByteArray ret = CredentialPassword::pbkdf2(QCryptographicHash::Sha1, password, salt, count, keyLength);
    QCOMPARE(ret.toHex(), result);
}

void TestPbkdf2::testPbkdf2_data()
{
    QTest::addColumn<QByteArray>("password");
    QTest::addColumn<QByteArray>("salt");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("keyLength");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("pbkdf2-test00") << QByteArrayLiteral("password") << QByteArrayLiteral("salt") << 1 << 20
                                   << QByteArrayLiteral("0c60c80f961f0e71"
                                                        "f3a9b524af601206"
                                                        "2fe037a6");

    QTest::newRow("pbkdf2-test01") << QByteArrayLiteral("password") << QByteArrayLiteral("salt") << 2 << 20
                                   << QByteArrayLiteral("ea6c014dc72d6f8c"
                                                        "cd1ed92ace1d41f0"
                                                        "d8de8957");

    QTest::newRow("pbkdf1-test02") << QByteArrayLiteral("password") << QByteArrayLiteral("salt") << 4096 << 20
                                   << QByteArrayLiteral("4b007901b765489a"
                                                        "bead49d926f721d0"
                                                        "65a429c1");

//    QTest::newRow("pbkdf2-test03") << QByteArrayLiteral("password") << QByteArrayLiteral("salt") << 16777216 << 20
//                                   << QByteArrayLiteral("eefe3d61cd4da4e4"
//                                                        "e9945b3d6ba2158c"
//                                                        "2634e984");

    QTest::newRow("pbkdf2-test04") << QByteArrayLiteral("passwordPASSWORDpassword") << QByteArrayLiteral("saltSALTsaltSALTsaltSALTsaltSALTsalt") << 4096 << 25
                                   << QByteArrayLiteral("3d2eec4fe41c849b"
                                                        "80c8d83662c0e44a"
                                                        "8b291a964cf2f070"
                                                        "38");

    QTest::newRow("pbkdf2-test05") << QByteArrayLiteral("pass\0word") << QByteArrayLiteral("sa\0lt") << 4096 << 16
                                   << QByteArrayLiteral("56fa6aa75548099d"
                                                        "cc37d7f03425e0c3");

}

QTEST_MAIN(TestPbkdf2)

#include "testpbkdf2.moc"

#endif
