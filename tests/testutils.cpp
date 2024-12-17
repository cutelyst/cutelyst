#ifndef UTILSTEST_H
#define UTILSTEST_H

#include <Cutelyst/utils.h>
#include <chrono>

#include <QTest>

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;

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

    QTest::newRow("valid-us-01") << u"123456us"_s << 123456us << true;
    QTest::newRow("valid-us-02") << u"123456  usec"_s << 123456us << true;

    QTest::newRow("valid-ms-01") << u"123ms"_s << 123000us << true;
    QTest::newRow("valid-ms-02") << u"123456  msec"_s << 123456000us << true;

    QTest::newRow("valid-sec-01") << u"12seconds"_s << 12000000us << true;
    QTest::newRow("valid-sec-02") << u"12 second"_s << 12000000us << true;
    QTest::newRow("valid-sec-03") << u"12    sec"_s << 12000000us << true;
    QTest::newRow("valid-sec-04") << u"  12    s"_s << 12000000us << true;
    QTest::newRow("valid-sec-05") << u"    12   "_s << 12000000us << true;

    QTest::newRow("valid-min-01") << u"12minutes"_s << 720000000us << true;
    QTest::newRow("valid-min-02") << u"12 minute"_s << 720000000us << true;
    QTest::newRow("valid-min-03") << u"12    min"_s << 720000000us << true;
    QTest::newRow("valid-min-04") << u"   12  m "_s << 720000000us << true;

    QTest::newRow("valid-hr-01") << u"1hours"_s << 3600000000us << true;
    QTest::newRow("valid-hr-02") << u"1 hour"_s << 3600000000us << true;
    QTest::newRow("valid-hr-03") << u"1   hr"_s << 3600000000us << true;
    QTest::newRow("valid-hr-04") << u" 1  h "_s << 3600000000us << true;

    QTest::newRow("valid-day-01") << u"1days"_s << 86400000000us << true;
    QTest::newRow("valid-day-02") << u"1 day"_s << 86400000000us << true;
    QTest::newRow("valid-day-03") << u" 1 d "_s << 86400000000us << true;

    QTest::newRow("valid-week-01") << u"1weeks"_s << 604800000000us << true;
    QTest::newRow("valid-week-02") << u"1 week"_s << 604800000000us << true;
    QTest::newRow("valid-week-03") << u" 1  w "_s << 604800000000us << true;

    QTest::newRow("valid-month-01") << u"1months"_s << 2629746000000us << true;
    QTest::newRow("valid-month-02") << u"1 month"_s << 2629746000000us << true;
    QTest::newRow("valid-month-03") << u"  1  M "_s << 2629746000000us << true;

    QTest::newRow("valid-year-01") << u"1years"_s << 31556952000000us << true;
    QTest::newRow("valid-year-02") << u"1 year"_s << 31556952000000us << true;
    QTest::newRow("valid-year-03") << u" 1  y "_s << 31556952000000us << true;

    QTest::newRow("combined-valid-01") << u"1hour 12 minutes"_s << 4320000000us << true;
    QTest::newRow("combined-valid-02") << u" 12 min   1hr  "_s << 4320000000us << true;
    QTest::newRow("combined-valid-03") << u"1 sec 1"_s << 2000000us << true;

    QTest::newRow("invalid-digit-too-long")
        << u"184467440737095516152934759234234"_s << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-no-digit") << u"seconds"_s << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-unit") << u"1 Stunde"_s << std::chrono::microseconds::zero() << false;
    QTest::newRow("invalid-empty") << u"  "_s << std::chrono::microseconds::zero() << false;
}

QTEST_MAIN(UtilsTest)

#include "testutils.moc"

#endif
