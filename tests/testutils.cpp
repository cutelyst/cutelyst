#ifndef UTILSTEST_H
#define UTILSTEST_H

#include <Cutelyst/utils.h>
#include <chrono>

#include <QTest>

using namespace std::chrono_literals;

class UtilsTest : public QObject
{
    Q_OBJECT
public:
    explicit UtilsTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void testDurationFromString();
    void testDurationFromString_data();
};

void UtilsTest::testDurationFromString()
{
    QFETCH(QString, str);
    QFETCH(std::chrono::microseconds, us);
    QFETCH(bool, ok);

    bool _ok = false;
    QCOMPARE(us, Cutelyst::Utils::durationFromString(str, &_ok));
    QCOMPARE(ok, _ok);
}

void UtilsTest::testDurationFromString_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<std::chrono::microseconds>("us");
    QTest::addColumn<bool>("ok");

    QTest::newRow("valid-us-01") << u"123456us"_qs << 123456us << true;
    QTest::newRow("valid-us-02") << u"123456  usec"_qs << 123456us << true;

    QTest::newRow("valid-ms-01") << u"123ms"_qs << 123000us << true;
    QTest::newRow("valid-ms-02") << u"123456  msec"_qs << 123456000us << true;

    QTest::newRow("valid-sec-01") << u"12seconds"_qs << 12000000us << true;
    QTest::newRow("valid-sec-02") << u"12 second"_qs << 12000000us << true;
    QTest::newRow("valid-sec-03") << u"12    sec"_qs << 12000000us << true;
    QTest::newRow("valid-sec-04") << u"  12    s"_qs << 12000000us << true;
    QTest::newRow("valid-sec-05") << u"    12   "_qs << 12000000us << true;

    QTest::newRow("valid-min-01") << u"12minutes"_qs << 720000000us << true;
    QTest::newRow("valid-min-02") << u"12 minute"_qs << 720000000us << true;
    QTest::newRow("valid-min-03") << u"12    min"_qs << 720000000us << true;
    QTest::newRow("valid-min-04") << u"   12  m "_qs << 720000000us << true;

    QTest::newRow("valid-hr-01") << u"1hours"_qs << 3600000000us << true;
    QTest::newRow("valid-hr-02") << u"1 hour"_qs << 3600000000us << true;
    QTest::newRow("valid-hr-03") << u"1   hr"_qs << 3600000000us << true;
    QTest::newRow("valid-hr-04") << u" 1  h "_qs << 3600000000us << true;

    QTest::newRow("valid-day-01") << u"1days"_qs << 86400000000us << true;
    QTest::newRow("valid-day-02") << u"1 day"_qs << 86400000000us << true;
    QTest::newRow("valid-day-03") << u" 1 d "_qs << 86400000000us << true;

    QTest::newRow("valid-week-01") << u"1weeks"_qs << 604800000000us << true;
    QTest::newRow("valid-week-02") << u"1 week"_qs << 604800000000us << true;
    QTest::newRow("valid-week-03") << u" 1  w "_qs << 604800000000us << true;

    QTest::newRow("valid-month-01") << u"1months"_qs << 2629746000000us << true;
    QTest::newRow("valid-month-02") << u"1 month"_qs << 2629746000000us << true;
    QTest::newRow("valid-month-03") << u"  1  M "_qs << 2629746000000us << true;

    QTest::newRow("valid-year-01") << u"1years"_qs << 31556952000000us << true;
    QTest::newRow("valid-year-02") << u"1 year"_qs << 31556952000000us << true;
    QTest::newRow("valid-year-03") << u" 1  y "_qs << 31556952000000us << true;

    QTest::newRow("combined-valid-01") << u"1hour 12 minutes"_qs << 4320000000us << true;
    QTest::newRow("combined-valid-02") << u" 12 min   1hr  "_qs << 4320000000us << true;
    QTest::newRow("combined-valid-03") << u"1 sec 1"_qs << 2000000us << true;

    QTest::newRow("invalid-digit-too-long")
        << u"184467440737095516152934759234234"_qs << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-no-digit")
        << u"seconds"_qs << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-unit") << u"1 Stunde"_qs << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-empty") << u"  "_qs << std::chrono::microseconds::zero() << false;
}

QTEST_MAIN(UtilsTest)

#include "testutils.moc"

#endif
