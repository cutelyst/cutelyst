#ifndef VALIDATORTEST_H
#define VALIDATORTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/Plugins/Utils/Validator/Validator>
#include <Cutelyst/Plugins/Utils/Validator/Validators>
#include <Cutelyst/Plugins/Utils/Validator/validatorresult.h>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>
#include <limits>

#include <QCryptographicHash>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTest>
#include <QTimeZone>
#include <QUrlQuery>
#include <QUuid>

using namespace Cutelyst;

class TestValidator : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestValidator(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

    ~TestValidator() {}

private Q_SLOTS:
    void initTestCase();

    void testValidator_data();
    void testValidator() { doTest(); }

    void testValidatorAccepted_data();
    void testValidatorAccepted() { doTest(); };

    void testValidatorAfter_data();
    void testValidatorAfter() { doTest(); };

    void testValidatorAlpha_data();
    void testValidatorAlpha() { doTest(); };

    void testValidatorAlphaDash_data();
    void testValidatorAlphaDash() { doTest(); };

    void testValidatorAlphaNum_data();
    void testValidatorAlphaNum() { doTest(); };

    void testValidatorBefore_data();
    void testValidatorBefore() { doTest(); };

    void testValidatorBetween_data();
    void testValidatorBetween() { doTest(); };

    void testValidatorBoolean_data();
    void testValidatorBoolean() { doTest(); };

    void testValidatorCharNotAllowed_data();
    void testValidatorCharNotAllowed() { doTest(); }

    void testValidatorConfirmed_data();
    void testValidatorConfirmed() { doTest(); };

    void testValidatorDate_data();
    void testValidatorDate() { doTest(); };

    void testValidatorDateTime_data();
    void testValidatorDateTime() { doTest(); };

    void testValidatorDifferent_data();
    void testValidatorDifferent() { doTest(); };

    void testValidatorDigits_data();
    void testValidatorDigits() { doTest(); };

    void testValidatorDigitsBetween_data();
    void testValidatorDigitsBetween() { doTest(); };

    void testValidatorDomain_data();
    void testValidatorDomain() { doTest(); };

    void testValidatorEmail_data();
    void testValidatorEmail() { doTest(); };

    void testValidatorFileSize_data();
    void testValidatorFileSize() { doTest(); };

    void testValidatorFilled_data();
    void testValidatorFilled() { doTest(); };

    void testValidatorIn_data();
    void testValidatorIn() { doTest(); };

    void testValidatorInteger_data();
    void testValidatorInteger() { doTest(); };

    void testValidatorIp_data();
    void testValidatorIp() { doTest(); };

    void testValidatorJson_data();
    void testValidatorJson() { doTest(); };

    void testValidatorMax_data();
    void testValidatorMax() { doTest(); };

    void testValidatorMin_data();
    void testValidatorMin() { doTest(); };

    void testValidatorNotIn_data();
    void testValidatorNotIn() { doTest(); };

    void testValidatorNumeric_data();
    void testValidatorNumeric() { doTest(); };

    void testValidatorPresent_data();
    void testValidatorPresent() { doTest(); };

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    void testValidatorPwQuality_data();
    void testValidatorPwQuality() { doTest(); };
#endif

    void testValidatorRegex_data();
    void testValidatorRegex() { doTest(); };

    void testValidatorRequired_data();
    void testValidatorRequired() { doTest(); };

    void testValidatorRequiredIf_data();
    void testValidatorRequiredIf() { doTest(); };

    void testValidatorRequiredIfStash_data();
    void testValidatorRequiredIfStash() { doTest(); };

    void testValidatorRequiredUnless_data();
    void testValidatorRequiredUnless() { doTest(); };

    void testValidatorRequiredUnlessStash_data();
    void testValidatorRequiredUnlessStash() { doTest(); };

    void testValidatorRequiredWith_data();
    void testValidatorRequiredWith() { doTest(); };

    void testValidatorRequiredWithAll_data();
    void testValidatorRequiredWithAll() { doTest(); };

    void testValidatorRequiredWithout_data();
    void testValidatorRequiredWithout() { doTest(); };

    void testValidatorRequiredWithoutAll_data();
    void testValidatorRequiredWithoutAll() { doTest(); };

    void testValidatorSame_data();
    void testValidatorSame() { doTest(); };

    void testValidatorSize_data();
    void testValidatorSize() { doTest(); };

    void testValidatorTime_data();
    void testValidatorTime() { doTest(); };

    void testValidatorUrl_data();
    void testValidatorUrl() { doTest(); };

    void cleanupTestCase();

private:
    TestEngine *m_engine;

    TestEngine *getEngine();

    void doTest();

    const QByteArray valid{"valid"};
    const QByteArray invalid{"invalid"};
    const QByteArray parsingError{"parsingerror"};
    const QByteArray validationDataError{"validationdataerror"};
    const QList<Qt::DateFormat> dateFormats{Qt::ISODate, Qt::RFC2822Date, Qt::TextDate};
};

class ValidatorTest : public Controller
{
    Q_OBJECT
public:
    explicit ValidatorTest(QObject *parent)
        : Controller(parent)
    {
    }

    // ***** Endpoint for checking Validator::BodyParamsOnly
    C_ATTR(bodyParamsOnly, :Local :AutoArgs)
    void bodyParamsOnly(Context *c)
    {
        Validator v({new ValidatorRequired(u"req_field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::BodyParamsOnly));
    }

    // ***** Endpoint for checking Validator::QueryParamsOnly
    C_ATTR(queryParamsOnly, :Local :AutoArgs)
    void queryParamsOnly(Context *c)
    {
        Validator v({new ValidatorRequired(u"req_field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::QueryParamsOnly));
    }

    // ***** Endpoint for ValidatorAccepted ******
    C_ATTR(accepted, :Local :AutoArgs)
    void accepted(Context *c)
    {
        Validator v({new ValidatorAccepted(u"accepted_field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QDate ******
    C_ATTR(afterDate, :Local :AutoArgs)
    void afterDate(Context *c)
    {
        Validator v({new ValidatorAfter(
            u"after_field"_qs, QDate::currentDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QTime ******
    C_ATTR(afterTime, :Local :AutoArgs)
    void afterTime(Context *c)
    {
        Validator v({new ValidatorAfter(
            u"after_field"_qs, QTime(12, 0), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QDateTime ******
    C_ATTR(afterDateTime, :Local :AutoArgs)
    void afterDateTime(Context *c)
    {
        Validator v({new ValidatorAfter(u"after_field"_qs,
                                        QDateTime::currentDateTime(),
                                        QString(),
                                        nullptr,
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with custom format ******
    C_ATTR(afterFormat, :Local :AutoArgs)
    void afterFormat(Context *c)
    {
        Validator v({new ValidatorAfter(u"after_field"_qs,
                                        QDateTime::currentDateTime(),
                                        QString(),
                                        "yyyy d MM HH:mm",
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data ******
    C_ATTR(afterInvalidValidationData, :Local :AutoArgs)
    void afterInvalidValidationData(Context *c)
    {
        Validator v({new ValidatorAfter(
            u"after_field"_qs, QDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data 2 ******
    C_ATTR(afterInvalidValidationData2, :Local :AutoArgs)
    void afterInvalidValidationData2(Context *c)
    {
        Validator v({new ValidatorAfter(
            u"after_field"_qs, u"schiet"_qs, QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with time zone *****
    C_ATTR(afterValidWithTimeZone, :Local :AutoArgs)
    void afterValidWithTimeZone(Context *c)
    {
        Validator v({new ValidatorAfter(u"after_field"_qs,
                                        QDateTime(QDate(2018, 1, 15),
                                                  QTime(12, 0),
                                                  QTimeZone(QByteArrayLiteral("Indian/Christmas"))),
                                        u"Europe/Berlin"_qs,
                                        nullptr,
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with time zone in iput field *****
    C_ATTR(afterValidWithTimeZoneField, :Local :AutoArgs)
    void afterValidWithTimeZoneField(Context *c)
    {
        Validator v({new ValidatorAfter(u"after_field"_qs,
                                        QDateTime(QDate(2018, 1, 15),
                                                  QTime(12, 0),
                                                  QTimeZone(QByteArrayLiteral("Indian/Christmas"))),
                                        u"tz_field"_qs,
                                        nullptr,
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlpha ******
    C_ATTR(alpha, :Local :AutoArgs)
    void alpha(Context *c)
    {
        Validator v({new ValidatorAlpha(u"alpha_field"_qs, false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // **** Endpoint for ValidatorAlpha only ASCII *****
    C_ATTR(alphaAscii, :Local :AutoArgs)
    void alphaAscii(Context *c)
    {
        Validator v({new ValidatorAlpha(u"alpha_field"_qs, true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaDash ******
    C_ATTR(alphaDash, :Local :AutoArgs)
    void alphaDash(Context *c)
    {
        Validator v({new ValidatorAlphaDash(u"alphadash_field"_qs, false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaDash only ASCII ******
    C_ATTR(alphaDashAscii, :Local :AutoArgs)
    void alphaDashAscii(Context *c)
    {
        Validator v({new ValidatorAlphaDash(u"alphadash_field"_qs, true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaNum ******
    C_ATTR(alphaNum, :Local :AutoArgs)
    void alphaNum(Context *c)
    {
        Validator v({new ValidatorAlphaNum(u"alphanum_field"_qs, false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaNum only ASCII ******
    C_ATTR(alphaNumAscii, :Local :AutoArgs)
    void alphaNumAscii(Context *c)
    {
        Validator v({new ValidatorAlphaNum(u"alphanum_field"_qs, true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QDate ******
    C_ATTR(beforeDate, :Local :AutoArgs)
    void beforeDate(Context *c)
    {
        Validator v({new ValidatorBefore(
            u"before_field"_qs, QDate::currentDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QTime ******
    C_ATTR(beforeTime, :Local :AutoArgs)
    void beforeTime(Context *c)
    {
        Validator v({new ValidatorBefore(
            u"before_field"_qs, QTime(12, 0), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QDateTime ******
    C_ATTR(beforeDateTime, :Local :AutoArgs)
    void beforeDateTime(Context *c)
    {
        Validator v({new ValidatorBefore(u"before_field"_qs,
                                         QDateTime::currentDateTime(),
                                         QString(),
                                         nullptr,
                                         m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with custom format ******
    C_ATTR(beforeFormat, :Local :AutoArgs)
    void beforeFormat(Context *c)
    {
        Validator v({new ValidatorBefore(u"before_field"_qs,
                                         QDateTime::currentDateTime(),
                                         QString(),
                                         "yyyy d MM HH:mm",
                                         m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data ******
    C_ATTR(beforeInvalidValidationData, :Local :AutoArgs)
    void beforeInvalidValidationData(Context *c)
    {
        Validator v({new ValidatorBefore(
            u"before_field"_qs, QDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data 2 ******
    C_ATTR(beforeInvalidValidationData2, :Local :AutoArgs)
    void beforeInvalidValidationData2(Context *c)
    {
        Validator v({new ValidatorBefore(
            u"before_field"_qs, u"schiet"_qs, QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with time zone *****
    C_ATTR(beforeValidWithTimeZone, :Local :AutoArgs)
    void beforeValidWithTimeZone(Context *c)
    {
        Validator v({new ValidatorBefore(u"after_field"_qs,
                                         QDateTime(QDate(2018, 1, 15),
                                                   QTime(12, 0),
                                                   QTimeZone(QByteArrayLiteral("America/Tijuana"))),
                                         u"Europe/Berlin"_qs,
                                         nullptr,
                                         m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with time zone in iput field *****
    C_ATTR(beforeValidWithTimeZoneField, :Local :AutoArgs)
    void beforeValidWithTimeZoneField(Context *c)
    {
        Validator v({new ValidatorBefore(u"after_field"_qs,
                                         QDateTime(QDate(2018, 1, 15),
                                                   QTime(12, 0),
                                                   QTimeZone(QByteArrayLiteral("America/Tijuana"))),
                                         u"tz_field"_qs,
                                         nullptr,
                                         m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with int ******
    C_ATTR(betweenInt, :Local :AutoArgs)
    void betweenInt(Context *c)
    {
        Validator v({new ValidatorBetween(
            u"between_field"_qs, QMetaType::Int, -10, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with uint ******
    C_ATTR(betweenUint, :Local :AutoArgs)
    void betweenUint(Context *c)
    {
        Validator v({new ValidatorBetween(
            u"between_field"_qs, QMetaType::UInt, 10, 20, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with float ******
    C_ATTR(betweenFloat, :Local :AutoArgs)
    void betweenFloat(Context *c)
    {
        Validator v({new ValidatorBetween(
            u"between_field"_qs, QMetaType::Float, -10.0, 10.0, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with string ******
    C_ATTR(betweenString, :Local :AutoArgs)
    void betweenString(Context *c)
    {
        Validator v({new ValidatorBetween(
            u"between_field"_qs, QMetaType::QString, 5, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBoolean ******
    C_ATTR(boolean, :Local :AutoArgs)
    void boolean(Context *c)
    {
        Validator v({new ValidatorBoolean(u"boolean_field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorCharNotAllowed ******
    C_ATTR(charNotAllowed, :Local :AutoArgs)
    void charNotAllowed(Context *c)
    {
        Validator v({new ValidatorCharNotAllowed(
            u"char_not_allowed_field"_qs, u"#%*."_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorConfirmed ******
    C_ATTR(confirmed, :Local :AutoArgs)
    void confirmed(Context *c)
    {
        Validator v({new ValidatorConfirmed(u"pass"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDate with standard formats ******
    C_ATTR(date, :Local :AutoArgs)
    void date(Context *c)
    {
        Validator v({new ValidatorDate(u"field"_qs, nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDate with custom format ******
    C_ATTR(dateFormat, :Local :AutoArgs)
    void dateFormat(Context *c)
    {
        Validator v({new ValidatorDate(u"field"_qs, "yyyy d MM", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDateTime with standard formats ******
    C_ATTR(dateTime, :Local :AutoArgs)
    void dateTime(Context *c)
    {
        Validator v({new ValidatorDateTime(u"field"_qs, QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDateTime with custom format ******
    C_ATTR(dateTimeFormat, :Local :AutoArgs)
    void dateTimeFormat(Context *c)
    {
        Validator v({new ValidatorDateTime(
            u"field"_qs, QString(), "yyyy d MM mm:HH", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDifferent ******
    C_ATTR(different, :Local :AutoArgs)
    void different(Context *c)
    {
        Validator v(
            {new ValidatorDifferent(u"field"_qs, u"other"_qs, nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigits without exact length ******
    C_ATTR(digits, :Local :AutoArgs)
    void digits(Context *c)
    {
        Validator v({new ValidatorDigits(u"field"_qs, -1, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigits with exact length ******
    C_ATTR(digitsLength, :Local :AutoArgs)
    void digitsLength(Context *c)
    {
        Validator v({new ValidatorDigits(u"field"_qs, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigitsBetween ******
    C_ATTR(digitsBetween, :Local :AutoArgs)
    void digitsBetween(Context *c)
    {
        Validator v({new ValidatorDigitsBetween(u"field"_qs, 5, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDomain without DNS check *****
    C_ATTR(domain, :Local :AutoArgs)
    void domain(Context *c)
    {
        Validator v({new ValidatorDomain(u"field"_qs, false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDomain with DNS check *****
    C_ATTR(domainDns, :Local :AutoArgs)
    void domainDns(Context *c)
    {
        Validator v({new ValidatorDomain(u"field"_qs, true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorEmail valid ****
    C_ATTR(emailValid, :Local :AutoArgs)
    void emailValid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::Valid, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail valid emails completely valid *****
    C_ATTR(emailDnsWarnValid, :Local :AutoArgs)
    void emailDnsWarnValid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::Valid, ValidatorEmail::CheckDNS, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5321 conformant emails valid *****
    C_ATTR(emailRfc5321Valid, :Local :AutoArgs)
    void emailRfc5321Valid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::RFC5321, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5321 conformant emails invalid *****
    C_ATTR(emailRfc5321Invalid, :Local :AutoArgs)
    void emailRfc5321Invalid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::DNSWarn, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails valid *****
    C_ATTR(emailCfwsValid, :Local :AutoArgs)
    void emailCfwsValid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::CFWS, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails invalid *****
    C_ATTR(emailCfwsInvalid, :Local :AutoArgs)
    void emailCfwsInvalid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::RFC5321, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail Deprecated emails valid *****
    C_ATTR(emailDeprecatedValid, :Local :AutoArgs)
    void emailDeprecatedValid(Context *c)
    {
        Validator v({new ValidatorEmail(u"field"_qs,
                                        ValidatorEmail::Deprecated,
                                        ValidatorEmail::NoOption,
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails invalid *****
    C_ATTR(emailDeprecatedInvalid, :Local :AutoArgs)
    void emailDeprecatedInvalid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::CFWS, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5322 emails valid *****
    C_ATTR(emailRfc5322Valid, :Local :AutoArgs)
    void emailRfc5322Valid(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::RFC5322, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5322 conformant emails invalid *****
    C_ATTR(emailRfc5322Invalid, :Local :AutoArgs)
    void emailRfc5322Invalid(Context *c)
    {
        Validator v({new ValidatorEmail(u"field"_qs,
                                        ValidatorEmail::Deprecated,
                                        ValidatorEmail::NoOption,
                                        m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with errors *****
    C_ATTR(emailErrors, :Local :AutoArgs)
    void emailErrors(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::RFC5322, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed IDNs
    C_ATTR(emailIdnAllowed, :Local :AutoArgs)
    void emailIdnAllowed(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::Valid, ValidatorEmail::AllowIDN, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed UTF8 local part
    C_ATTR(emailUtf8Local, :Local :AutoArgs)
    void emailUtf8Local(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::Valid, ValidatorEmail::UTF8Local, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed UTF8 local part and IDN
    C_ATTR(emailUtf8, :Local :AutoArgs)
    void emailUtf8(Context *c)
    {
        Validator v({new ValidatorEmail(
            u"field"_qs, ValidatorEmail::Valid, ValidatorEmail::AllowUTF8, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming | Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorFileSize ******
    C_ATTR(fileSize, :Local :AutoArgs)
    void fileSize(Context *c)
    {
        ValidatorFileSize::Option option = ValidatorFileSize::NoOption;
        const QString opt                = c->req()->bodyParameter(u"option"_qs);
        if (opt == QLatin1String("OnlyBinary")) {
            option = ValidatorFileSize::OnlyBinary;
        } else if (opt == QLatin1String("OnlyDecimal")) {
            option = ValidatorFileSize::OnlyDecimal;
        } else if (opt == QLatin1String("ForceBinary")) {
            option = ValidatorFileSize::ForceBinary;
        } else if (opt == QLatin1String("ForceDecimal")) {
            option = ValidatorFileSize::ForceDecimal;
        }
        const double min = c->req()->bodyParameter(u"min"_qs, u"-1.0"_qs).toDouble();
        const double max = c->req()->bodyParameter(u"max"_qs, u"-1.0"_qs).toDouble();
        c->setLocale(QLocale(c->req()->bodyParameter(u"locale"_qs, u"C"_qs)));
        Validator v({new ValidatorFileSize(u"field"_qs, option, min, max, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming));
    }

    // ***** Endpoint for ValidatorFileSize with return value check *****
    C_ATTR(fileSizeValue, :Local :AutoArgs)
    void fileSizeValue(Context *c)
    {
        c->setLocale(QLocale::c());
        Validator v({new ValidatorFileSize(u"field"_qs)});
        const ValidatorResult r = v.validate(c);
        if (r) {
            QString sizeString;
            const QVariant rv = r.value(u"field"_qs);
            if (rv.typeId() == QMetaType::Double) {
                sizeString = QString::number(rv.toDouble(), 'f', 2);
            } else {
                sizeString = QString::number(rv.toULongLong());
            }
            c->response()->setBody(sizeString.toUtf8());
        } else {
            c->response()->setBody(r.errorStrings().constFirst());
        }
    }

    // ***** Endpoint for ValidatorFilled ******
    C_ATTR(filled, :Local :AutoArgs)
    void filled(Context *c)
    {
        Validator v({new ValidatorFilled(u"field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorIn ******
    C_ATTR(in, :Local :AutoArgs)
    void in(Context *c)
    {
        Validator v({new ValidatorIn(u"field"_qs,
                                     QStringList({u"eins"_qs, u"zwei"_qs, u"drei"_qs}),
                                     Qt::CaseSensitive,
                                     m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorInteger ******
    C_ATTR(integer, :Local :AutoArgs)
    void integer(Context *c)
    {
        Validator v({new ValidatorInteger(u"field"_qs, QMetaType::Int, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorIp ******
    C_ATTR(ip, :Local :AutoArgs)
    void ip(Context *c)
    {
        ValidatorIp::Constraints constraints = ValidatorIp::NoConstraint;
        if (!c->request()->bodyParameter(u"constraints"_qs).isEmpty()) {
            QStringList cons = c->request()->bodyParameter(u"constraints"_qs).split(u","_qs);
            if (cons.contains(u"IPv4Only"_qs)) {
                constraints |= ValidatorIp::IPv4Only;
            }

            if (cons.contains(u"IPv6Only"_qs)) {
                constraints |= ValidatorIp::IPv6Only;
            }

            if (cons.contains(u"NoPrivateRange"_qs)) {
                constraints |= ValidatorIp::NoPrivateRange;
            }

            if (cons.contains(u"NoReservedRange"_qs)) {
                constraints |= ValidatorIp::NoReservedRange;
            }

            if (cons.contains(u"NoMultiCast"_qs)) {
                constraints |= ValidatorIp::NoMultiCast;
            }
        }
        Validator v({new ValidatorIp(u"field"_qs, constraints, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorJson ******
    C_ATTR(json, :Local :AutoArgs)
    void json(Context *c)
    {
        Validator v({new ValidatorJson(
            u"field"_qs, ValidatorJson::ExpectedType::All, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorJson with object expected ******
    C_ATTR(jsonObject, :Local :AutoArgs)
    void jsonObject(Context *c)
    {
        Validator v({new ValidatorJson(
            u"field"_qs, ValidatorJson::ExpectedType::Object, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorJson with array expected ******
    C_ATTR(jsonArray, :Local :AutoArgs)
    void jsonArray(Context *c)
    {
        Validator v({new ValidatorJson(
            u"field"_qs, ValidatorJson::ExpectedType::Array, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorMax ******
    C_ATTR(max, :Local :AutoArgs)
    void max(Context *c)
    {
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(u"type"_qs).isEmpty()) {
            const QString t = c->request()->bodyParameter(u"type"_qs);
            if (t == QLatin1String("sint")) {
                type = QMetaType::Int;
            } else if (t == QLatin1String("uint")) {
                type = QMetaType::UInt;
            } else if (t == QLatin1String("float")) {
                type = QMetaType::Float;
            } else if (t == QLatin1String("string")) {
                type = QMetaType::QString;
            }
        }
        Validator v({new ValidatorMax(u"field"_qs, type, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorMin ******
    C_ATTR(min, :Local :AutoArgs)
    void min(Context *c)
    {
        c->setStash(u"compval"_qs, 10);
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(u"type"_qs).isEmpty()) {
            const QString t = c->request()->bodyParameter(u"type"_qs);
            if (t == QLatin1String("sint")) {
                type = QMetaType::Int;
            } else if (t == QLatin1String("uint")) {
                type = QMetaType::UInt;
            } else if (t == QLatin1String("float")) {
                type = QMetaType::Float;
            } else if (t == QLatin1String("string")) {
                type = QMetaType::QString;
            }
        }
        Validator v({new ValidatorMin(u"field"_qs, type, u"compval"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorNotIn ******
    C_ATTR(notIn, :Local :AutoArgs)
    void notIn(Context *c)
    {
        Validator v(
            {new ValidatorNotIn(u"field"_qs,
                                QStringList({u"eins"_qs, u"zwei"_qs, u"drei"_qs, u"vier"_qs}),
                                Qt::CaseSensitive,
                                m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorNumeric ******
    C_ATTR(numeric, :Local :AutoArgs)
    void numeric(Context *c)
    {
        Validator v({new ValidatorNumeric(u"field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorPresent ******
    C_ATTR(present, :Local :AutoArgs)
    void present(Context *c)
    {
        Validator v({new ValidatorPresent(u"field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorPwQuality *****
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    C_ATTR(pwQuality, :Local :AutoArgs)
    void pwQuality(Context *c)
    {
        static const QVariantMap options({{u"difok"_qs, 1},
                                          {u"minlen"_qs, 8},
                                          {u"dcredit"_qs, 0},
                                          {u"ucredit"_qs, 0},
                                          {u"ocredit"_qs, 0},
                                          {u"lcredit"_qs, 0},
                                          {u"minclass"_qs, 0},
                                          {u"maxrepeat"_qs, 0},
                                          {u"maxclassrepeat"_qs, 0},
                                          {u"maxsequence"_qs, 0},
                                          {u"gecoscheck"_qs, 0},
                                          {u"dictcheck"_qs, 1},
                                          {u"usercheck"_qs, 0}});
        static Validator v({new ValidatorPwQuality(
            u"field"_qs, 50, options, QString(), QString(), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }
#endif

    // ***** Endpoint for ValidatorRegularExpression ******
    C_ATTR(regex, :Local :AutoArgs)
    void regex(Context *c)
    {
        Validator v({new ValidatorRegularExpression(
            u"field"_qs,
            QRegularExpression(u"^(\\d\\d)/(\\d\\d)/(\\d\\d\\d\\d)$"_qs),
            m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequired ******
    C_ATTR(required, :Local :AutoArgs)
    void required(Context *c)
    {
        Validator v({new ValidatorRequired(u"field"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIf ******
    C_ATTR(requiredIf, :Local :AutoArgs)
    void requiredIf(Context *c)
    {
        Validator v({new ValidatorRequiredIf(u"field"_qs,
                                             u"field2"_qs,
                                             QStringList({u"eins"_qs, u"zwei"_qs}),
                                             m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash match *****
    C_ATTR(requiredIfStashMatch, :Local :AutoArgs)
    void requiredIfStashMatch(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"eins"_qs);
        Validator v({new ValidatorRequiredIfStash(u"field"_qs,
                                                  u"stashkey"_qs,
                                                  QVariantList({u"eins"_qs, u"zwei"_qs}),
                                                  m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash match in other stash key *****
    C_ATTR(requiredIfStashMatchStashKey, :Local :AutoArgs)
    void requiredIfStashMatchStashKey(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"eins"_qs);
        c->setStash(u"otherStashKey"_qs, QStringList({u"eins"_qs, u"zwei"_qs}));
        Validator v({new ValidatorRequiredIfStash(
            u"field"_qs, u"stashkey"_qs, u"otherStashKey"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash not match *****
    C_ATTR(requiredIfStashNotMatch, :Local :AutoArgs)
    void requiredIfStashNotMatch(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"drei"_qs);
        Validator v({new ValidatorRequiredIfStash(u"field"_qs,
                                                  u"stashkey"_qs,
                                                  QVariantList({u"eins"_qs, u"zwei"_qs}),
                                                  m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash match in other stash key *****
    C_ATTR(requiredIfStashNotMatchStashKey, :Local :AutoArgs)
    void requiredIfStashNotMatchStashKey(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"drei"_qs);
        c->setStash(u"otherStashKey"_qs, QStringList({u"eins"_qs, u"zwei"_qs}));
        Validator v({new ValidatorRequiredIfStash(
            u"field"_qs, u"stashkey"_qs, u"otherStashKey"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnless ******
    C_ATTR(requiredUnless, :Local :AutoArgs)
    void requiredUnless(Context *c)
    {
        Validator v({new ValidatorRequiredUnless(u"field"_qs,
                                                 u"field2"_qs,
                                                 QStringList({u"eins"_qs, u"zwei"_qs}),
                                                 m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash match *****
    C_ATTR(requiredUnlessStashMatch, :Local :AutoArgs)
    void requiredUnlessStashMatch(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"eins"_qs);
        Validator v({new ValidatorRequiredUnlessStash(u"field"_qs,
                                                      u"stashkey"_qs,
                                                      QVariantList({u"eins"_qs, u"zwei"_qs}),
                                                      m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash match in other stash key *****
    C_ATTR(requiredUnlessStashMatchStashKey, :Local :AutoArgs)
    void requiredUnlessStashMatchStashKey(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"eins"_qs);
        c->setStash(u"otherStashKey"_qs, QStringList({u"eins"_qs, u"zwei"_qs}));
        Validator v({new ValidatorRequiredUnlessStash(
            u"field"_qs, u"stashkey"_qs, u"otherStashKey"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash not match *****
    C_ATTR(requiredUnlessStashNotMatch, :Local :AutoArgs)
    void requiredUnlessStashNotMatch(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"drei"_qs);
        Validator v({new ValidatorRequiredUnlessStash(u"field"_qs,
                                                      u"stashkey"_qs,
                                                      QVariantList({u"eins"_qs, u"zwei"_qs}),
                                                      m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash not match in other stash key *****
    C_ATTR(requiredUnlessStashNotMatchStashKey, :Local :AutoArgs)
    void requiredUnlessStashNotMatchStashKey(Context *c)
    {
        c->setStash(u"stashkey"_qs, u"drei"_qs);
        c->setStash(u"otherStashKey"_qs, QStringList({u"eins"_qs, u"zwei"_qs}));
        Validator v({new ValidatorRequiredUnlessStash(
            u"field"_qs, u"stashkey"_qs, u"otherStashKey"_qs, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWith ******
    C_ATTR(requiredWith, :Local :AutoArgs)
    void requiredWith(Context *c)
    {
        Validator v({new ValidatorRequiredWith(
            u"field"_qs, QStringList({u"field2"_qs, u"field3"_qs}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithAll ******
    C_ATTR(requiredWithAll, :Local :AutoArgs)
    void requiredWithAll(Context *c)
    {
        Validator v(
            {new ValidatorRequiredWithAll(u"field"_qs,
                                          QStringList({u"field2"_qs, u"field3"_qs, u"field4"_qs}),
                                          m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithout ******
    C_ATTR(requiredWithout, :Local :AutoArgs)
    void requiredWithout(Context *c)
    {
        Validator v({new ValidatorRequiredWithout(
            u"field"_qs, QStringList({u"field2"_qs, u"field3"_qs}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithoutAll ******
    C_ATTR(requiredWithoutAll, :Local :AutoArgs)
    void requiredWithoutAll(Context *c)
    {
        Validator v({new ValidatorRequiredWithoutAll(
            u"field"_qs, QStringList({u"field2"_qs, u"field3"_qs}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorSame ******
    C_ATTR(same, :Local :AutoArgs)
    void same(Context *c)
    {
        Validator v({new ValidatorSame(u"field"_qs, u"other"_qs, nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorSize ******
    C_ATTR(size, :Local :AutoArgs)
    void size(Context *c)
    {
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(u"type"_qs).isEmpty()) {
            const QString t = c->request()->bodyParameter(u"type"_qs);
            if (t == QLatin1String("sint")) {
                type = QMetaType::Int;
            } else if (t == QLatin1String("uint")) {
                type = QMetaType::UInt;
            } else if (t == QLatin1String("float")) {
                type = QMetaType::Float;
            } else if (t == QLatin1String("string")) {
                type = QMetaType::QString;
            }
        }
        Validator v({new ValidatorSize(u"field"_qs, type, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorTime
    C_ATTR(time, :Local :AutoArgs)
    void time(Context *c)
    {
        Validator v({new ValidatorTime(u"field"_qs, nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorTime with custom format ******
    C_ATTR(timeFormat, :Local :AutoArgs)
    void timeFormat(Context *c)
    {
        Validator v({new ValidatorTime(u"field"_qs, "m:hh", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorUrl
    C_ATTR(url, :Local :AutoArgs)
    void url(Context *c)
    {
        ValidatorUrl::Constraints constraints = ValidatorUrl::NoConstraint;
        QStringList schemes;
        QString scheme = c->request()->bodyParameter(u"schemes"_qs);
        if (!scheme.isEmpty()) {
            schemes = scheme.split(u","_qs);
        }

        if (!c->request()->bodyParameter(u"constraints"_qs).isEmpty()) {
            const QStringList cons = c->request()->bodyParameter(u"constraints"_qs).split(u","_qs);
            if (cons.contains(u"StrictParsing"_qs)) {
                constraints |= ValidatorUrl::StrictParsing;
            }

            if (cons.contains(u"NoRelative"_qs)) {
                constraints |= ValidatorUrl::NoRelative;
            }

            if (cons.contains(u"NoLocalFile"_qs)) {
                constraints |= ValidatorUrl::NoLocalFile;
            }

            if (cons.contains(u"WebsiteOnly"_qs)) {
                constraints |= ValidatorUrl::WebsiteOnly;
            }
        }

        Validator v({new ValidatorUrl(u"field"_qs, constraints, schemes, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

private:
    ValidatorMessages m_validatorMessages =
        ValidatorMessages(nullptr, "invalid", "parsingerror", "validationdataerror");

    void checkResponse(Context *c, const ValidatorResult &r)
    {
        if (r) {
            c->response()->setBody("valid"_qba);
        } else {
            c->response()->setBody(r.errorStrings().constFirst());
        }
    }
};

void TestValidator::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestValidator::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new ValidatorTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestValidator::cleanupTestCase()
{
    delete m_engine;
}

void TestValidator::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, output);

    const QUrl urlAux(u"/validator/test" + url);
    static const Headers headers{{"Content-Type"_qba, "application/x-www-form-urlencoded"_qba}};

    const auto result = m_engine->createRequest(
        "POST", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), headers, &body);

    QCOMPARE(result.body, output);
}

void TestValidator::testValidator_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing if the correct parameters are extracted according to the validator flags

    QTest::newRow("body-params-only-valid")
        << u"/bodyParamsOnly"_qs << QByteArrayLiteral("req_field=hallo") << valid;

    QTest::newRow("body-params-only-invalid")
        << u"/bodyParamsOnly?req_field=hallo"_qs << QByteArray() << invalid;

    QTest::newRow("query-params-only-valid")
        << u"/queryParamsOnly?req_field=hallo"_qs << QByteArray() << valid;

    QTest::newRow("query-params-only-invalid")
        << u"/queryParamsOnly"_qs << QByteArrayLiteral("req_field=hallo") << invalid;
}

void TestValidator::testValidatorAccepted_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorAccepted *****

    int count = 0;
    for (const QString &val : {u"yes"_qs, u"on"_qs, u"1"_qs, u"true"_qs}) {
        QTest::newRow(u"valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/accepted?accepted_field="_qs + val << QByteArray() << valid;
        count++;
    }

    QTest::newRow("invalid") << u"/accepted?accepted_field=asdf"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/accepted?accepted_field="_qs << QByteArray() << invalid;

    QTest::newRow("missing") << u"/accepted"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorAfter_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorAfter *****

    int count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QUrlQuery query;
        query.addQueryItem(u"after_field"_qs, QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(u"date-valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/afterDate?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << valid;

        query.clear();
        query.addQueryItem(u"after_field"_qs, QDate(1999, 9, 9).toString(df));
        QTest::newRow(u"date-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/afterDate?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("date-parsingerror")
        << u"/afterDate?after_field=lökjasdfjh"_qs << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QUrlQuery query;
        query.addQueryItem(u"after_field"_qs, QTime(13, 0).toString(df));
        QTest::newRow(u"time-valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/afterTime?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << valid;

        query.clear();
        query.addQueryItem(u"after_field"_qs, QTime(11, 0).toString(df));
        QTest::newRow(u"time-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/afterTime?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("time-parsingerror")
        << u"/afterTime?after_field=kjnagiuh"_qs << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QString queryPath = u"/afterDateTime?after_field="_qs +
                            QString::fromLatin1(QUrl::toPercentEncoding(
                                QDateTime::currentDateTime().addDays(2).toString(df),
                                QByteArray(),
                                QByteArrayLiteral("+")));
        QTest::newRow(u"after-datetime-valid0%1"_qs.arg(count).toUtf8().constData())
            << queryPath << QByteArray() << valid;

        queryPath = u"/afterDateTime?after_field="_qs +
                    QString::fromLatin1(QUrl::toPercentEncoding(
                        QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df),
                        QByteArray(),
                        QByteArrayLiteral("+")));
        QTest::newRow(u"after-datetime-invalid0%1"_qs.arg(count).toUtf8().constData())
            << queryPath << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("datetime-parsingerror")
        << u"/afterDateTime?after_field=aio,aü"_qs << QByteArray() << parsingError;

    QTest::newRow("invalidvalidationdata00")
        << u"/afterInvalidValidationData?after_field="_qs +
               QDate::currentDate().addDays(2).toString(Qt::ISODate)
        << QByteArray() << validationDataError;

    QTest::newRow("invalidvalidationdata01")
        << u"/afterInvalidValidationData2?after_field="_qs +
               QDate::currentDate().addDays(2).toString(Qt::ISODate)
        << QByteArray() << validationDataError;

    QTest::newRow("format-valid") << u"/afterFormat?after_field="_qs +
                                         QDateTime::currentDateTime().addDays(2).toString(
                                             u"yyyy d MM HH:mm"_qs)
                                  << QByteArray() << valid;

    QTest::newRow("format-invalid")
        << u"/afterFormat?after_field="_qs +
               QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(u"yyyy d MM HH:mm"_qs)
        << QByteArray() << invalid;

    QTest::newRow("format-parsingerror")
        << u"/afterFormat?after_field=23590uj09"_qs << QByteArray() << parsingError;

    {
        const QString queryPath =
            u"/afterValidWithTimeZone?after_field="_qs +
            QString::fromLatin1(QUrl::toPercentEncoding(
                QDateTime(QDate(2018, 1, 15), QTime(13, 0)).toString(Qt::ISODate),
                QByteArray(),
                QByteArrayLiteral("+")));
        QTest::newRow("timezone-valid") << queryPath << QByteArray() << valid;
    }

    {
        const QString queryPath =
            u"/afterValidWithTimeZoneField?after_field="_qs +
            QString::fromLatin1(QUrl::toPercentEncoding(
                QDateTime(QDate(2018, 1, 15), QTime(13, 0)).toString(Qt::ISODate),
                QByteArray(),
                QByteArrayLiteral("+"))) +
            QLatin1String("&tz_field=Europe/Berlin");
        QTest::newRow("timezone-fromfield-valid") << queryPath << QByteArray() << valid;
    }
}

void TestValidator::testValidatorAlpha_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorAlpha *****

    QTest::newRow("valid") << u"/alpha?alpha_field=adsfä"_qs << QByteArray() << valid;

    QTest::newRow("invalid") << u"/alpha?alpha_field=ad_sf 2ä!"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/alpha?alpha_field="_qs << QByteArray() << valid;

    QTest::newRow("missing") << u"/alpha"_qs << QByteArray() << valid;

    QTest::newRow("ascii-valid") << u"/alphaAscii?alpha_field=basdf"_qs << QByteArray() << valid;

    QTest::newRow("ascii-invalid")
        << u"/alphaAscii?alpha_field=asdfös"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorAlphaDash_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorAlphaDash *****

    QTest::newRow("valid") << u"/alphaDash?alphadash_field=ads2-fä_3"_qs << QByteArray() << valid;

    QTest::newRow("invalid") << u"/alphaDash?alphadash_field=ad sf_2ä!"_qs << QByteArray()
                             << invalid;

    QTest::newRow("empty") << u"/alphaDash?alphadash_field="_qs << QByteArray() << valid;

    QTest::newRow("missing") << u"/alphaDash"_qs << QByteArray() << valid;

    QTest::newRow("ascii-valid") << u"/alphaDashAscii?alphadash_field=s342-4d_3"_qs << QByteArray()
                                 << valid;

    QTest::newRow("ascii-invalid")
        << u"/alphaDashAscii?alphadash_field=s342 4ä_3"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorAlphaNum_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorAlphaNum *****

    QTest::newRow("valid") << u"/alphaNum?alphanum_field=ads2fä3"_qs << QByteArray() << valid;

    QTest::newRow("invalid") << u"/alphaNum?alphanum_field=ad sf_2ä!"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/alphaNum?alphanum_field="_qs << QByteArray() << valid;

    QTest::newRow("missing") << u"/alphaNum"_qs << QByteArray() << valid;

    QTest::newRow("ascii-valid") << u"/alphaNumAscii?alphanum_field=ba34sdf"_qs << QByteArray()
                                 << valid;

    QTest::newRow("ascii-invalid")
        << u"/alphaNumAscii?alphanum_field=as3dfös"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorBefore_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorBefore *****

    int count = 0;
    QUrlQuery query;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(u"before_field"_qs, QDate(1999, 9, 9).toString(df));
        QTest::newRow(u"before-date-valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/beforeDate?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << valid;

        query.clear();
        query.addQueryItem(u"before_field"_qs, QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(u"before-date-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/beforeDate?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-date-parsingerror")
        << u"/beforeDate?before_field=lökjasdfjh"_qs << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(u"before_field"_qs, QTime(11, 0).toString(df));
        QTest::newRow(u"before-time-valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/beforeTime?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << valid;

        query.clear();
        query.addQueryItem(u"before_field"_qs, QTime(13, 0).toString(df));
        QTest::newRow(u"before-time-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/beforeTime?"_qs + query.toString(QUrl::FullyEncoded) << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-time-parsingerror")
        << u"/beforeTime?before_field=kjnagiuh"_qs << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QString pathQuery = u"/beforeDateTime?before_field="_qs +
                            QString::fromLatin1(QUrl::toPercentEncoding(
                                QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df),
                                QByteArray(),
                                QByteArrayLiteral("+")));
        QTest::newRow(QString(u"before-datetime-valid0%1"_qs.arg(count)).toUtf8().constData())
            << pathQuery << QByteArray() << valid;

        pathQuery = u"/beforeDateTime?before_field="_qs +
                    QString::fromLatin1(QUrl::toPercentEncoding(
                        QDateTime::currentDateTime().addDays(2).toString(df),
                        QByteArray(),
                        QByteArrayLiteral("+")));
        QTest::newRow(u"before-datetime-invalid0%1"_qs.arg(count).toUtf8().constData())
            << pathQuery << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-datetime-parsingerror")
        << u"/beforeDateTime?before_field=aio,aü"_qs << QByteArray() << parsingError;

    QTest::newRow("before-invalidvalidationdata00")
        << u"/beforeInvalidValidationData?before_field="_qs +
               QDate(1999, 9, 9).toString(Qt::ISODate)
        << QByteArray() << validationDataError;

    QTest::newRow("before-invalidvalidationdata01")
        << u"/beforeInvalidValidationData2?before_field="_qs +
               QDate(1999, 9, 9).toString(Qt::ISODate)
        << QByteArray() << validationDataError;

    QTest::newRow("before-format-valid")
        << u"/beforeFormat?before_field="_qs +
               QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(u"yyyy d MM HH:mm"_qs)
        << QByteArray() << valid;

    QTest::newRow("before-format-invalid")
        << u"/beforeFormat?before_field="_qs +
               QDateTime::currentDateTime().addDays(2).toString(u"yyyy d MM HH:mm"_qs)
        << QByteArray() << invalid;

    QTest::newRow("before-format-parsingerror")
        << u"/beforeFormat?before_field=23590uj09"_qs << QByteArray() << parsingError;

    {
        const QString pathQuery =
            u"/beforeValidWithTimeZone?after_field="_qs +
            QString::fromLatin1(QUrl::toPercentEncoding(
                QDateTime(QDate(2018, 1, 15), QTime(11, 0)).toString(Qt::ISODate),
                QByteArray(),
                QByteArrayLiteral("+")));
        QTest::newRow("before-timezone-valid") << pathQuery << QByteArray() << valid;
    }

    {
        const QString pathQuery =
            u"/beforeValidWithTimeZoneField?after_field="_qs +
            QString::fromLatin1(QUrl::toPercentEncoding(
                QDateTime(QDate(2018, 1, 15), QTime(11, 0)).toString(Qt::ISODate),
                QByteArray(),
                QByteArrayLiteral("+"))) +
            QLatin1String("&tz_field=Europe/Berlin");
        QTest::newRow("before-timezone-fromfield-valid") << pathQuery << QByteArray() << valid;
    }
}

void TestValidator::testValidatorBetween_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorBetween *****

    QTest::newRow("int-valid") << u"/betweenInt?between_field=0"_qs << QByteArray() << valid;

    QTest::newRow("int-invalid-lower")
        << u"/betweenInt?between_field=-15"_qs << QByteArray() << invalid;

    QTest::newRow("int-invalid-greater")
        << u"/betweenInt?between_field=15"_qs << QByteArray() << invalid;

    QTest::newRow("int-empty") << u"/betweenInt?between_field="_qs << QByteArray() << valid;

    QTest::newRow("uint-valid") << u"/betweenUint?between_field=15"_qs << QByteArray() << valid;

    QTest::newRow("uint-invalid-lower")
        << u"/betweenUint?between_field=5"_qs << QByteArray() << invalid;

    QTest::newRow("uint-invalid-greater")
        << u"/betweenUint?between_field=25"_qs << QByteArray() << invalid;

    QTest::newRow("uint-empty") << u"/betweenUint?between_field="_qs << QByteArray() << valid;

    QTest::newRow("float-valid") << u"/betweenFloat?between_field=0.0"_qs << QByteArray() << valid;

    QTest::newRow("float-invalid-lower")
        << u"/betweenFloat?between_field=-15.2"_qs << QByteArray() << invalid;

    QTest::newRow("float-invalid-greater")
        << u"/betweenFloat?between_field=15.2"_qs << QByteArray() << invalid;

    QTest::newRow("float-empty") << u"/betweenFloat?between_field="_qs << QByteArray() << valid;

    QTest::newRow("string-valid") << u"/betweenString?between_field=abcdefg"_qs << QByteArray()
                                  << valid;

    QTest::newRow("string-invalid-lower")
        << u"/betweenString?between_field=abc"_qs << QByteArray() << invalid;

    QTest::newRow("string-invalid-greater")
        << u"/betweenString?between_field=abcdefghijklmn"_qs << QByteArray() << invalid;

    QTest::newRow("string-empty") << u"/betweenString?between_field="_qs << QByteArray() << valid;
}

void TestValidator::testValidatorBoolean_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorBoolean *****

    for (const QString &bv : {u"1"_qs, u"0"_qs, u"true"_qs, u"false"_qs, u"on"_qs, u"off"_qs}) {
        QTest::newRow(u"valid-%1"_qs.arg(bv).toUtf8().constData())
            << u"/boolean?boolean_field="_qs + bv << QByteArray() << valid;
    }

    for (const QString &bv : {u"2"_qs, u"-45"_qs, u"wahr"_qs, u"unwahr"_qs, u"ja"_qs}) {
        QTest::newRow(u"invalid-%1"_qs.arg(bv).toUtf8().constData())
            << u"/boolean?boolean_field="_qs + bv << QByteArray() << invalid;
    }

    QTest::newRow("empty") << u"/boolean?boolean_field="_qs << QByteArray() << valid;
}

void TestValidator::testValidatorCharNotAllowed_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorCharNotAllowed *****

    QTest::newRow("empty") << u"/charNotAllowed?char_not_allowed_field="_qs << QByteArray()
                           << valid;

    QTest::newRow("valid") << u"/charNotAllowed?char_not_allowed_field=holladiewaldfee"_qs
                           << QByteArray() << valid;

    QTest::newRow("invalid") << u"/charNotAllowed?char_not_allowed_field=holla.die.waldfee"_qs
                             << QByteArray() << invalid;
}

void TestValidator::testValidatorConfirmed_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorConfirmed *****

    QTest::newRow("valid") << u"/confirmed?pass=abcdefg&pass_confirmation=abcdefg"_qs
                           << QByteArray() << valid;

    QTest::newRow("invalid") << u"/confirmed?pass=abcdefg&pass_confirmation=hijklmn"_qs
                             << QByteArray() << invalid;

    QTest::newRow("empty") << u"/confirmed?pass&pass_confirmation=abcdefg"_qs << QByteArray()
                           << valid;

    QTest::newRow("missing-confirmation")
        << u"/confirmed?pass=abcdefg"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorDate_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDate *****

    int count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(u"valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/date?field="_qs + QDate::currentDate().toString(df) << QByteArray() << valid;
        count++;
    }

    QTest::newRow("invalid") << u"/date?field=123456789"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/date?field="_qs << QByteArray() << valid;

    QTest::newRow("format-valid") << u"/dateFormat?field="_qs +
                                         QDate::currentDate().toString(u"yyyy d MM"_qs)
                                  << QByteArray() << valid;

    QTest::newRow("format-invalid")
        << u"/dateFormat?field="_qs + QDate::currentDate().toString(u"MM yyyy d"_qs) << QByteArray()
        << invalid;
}

void TestValidator::testValidatorDateTime_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDateTime *****

    int count = 0;
    for (Qt::DateFormat df : dateFormats) {
        const QString pathQuery =
            u"/dateTime?field="_qs +
            QString::fromLatin1(QUrl::toPercentEncoding(
                QDateTime::currentDateTime().toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(u"datetime-valid0%1"_qs.arg(count).toUtf8().constData())
            << pathQuery << QByteArray() << valid;
        count++;
    }

    QTest::newRow("invalid") << u"/dateTime?field=123456789"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/dateTime?field="_qs << QByteArray() << valid;

    QTest::newRow("format-valid") << u"/dateTimeFormat?field="_qs +
                                         QDateTime::currentDateTime().toString(
                                             u"yyyy d MM mm:HH"_qs)
                                  << QByteArray() << valid;

    QTest::newRow("format-invalid")
        << u"/dateTimeFormat?field="_qs +
               QDateTime::currentDateTime().toString(u"MM mm yyyy HH d"_qs)
        << QByteArray() << invalid;
}

void TestValidator::testValidatorDifferent_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDifferent *****

    QTest::newRow("valid") << u"/different?field=abcdefg&other=hijklmno"_qs << QByteArray()
                           << valid;

    QTest::newRow("invalid") << u"/different?field=abcdefg&other=abcdefg"_qs << QByteArray()
                             << invalid;

    QTest::newRow("empty") << u"/different?field=&other=hijklmno"_qs << QByteArray() << valid;

    QTest::newRow("other-missing") << u"/different?field=abcdefg"_qs << QByteArray() << valid;
}

void TestValidator::testValidatorDigits_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDigits *****

    QTest::newRow("valid") << u"/digits?field=0123456"_qs << QByteArray() << valid;

    QTest::newRow("invalid") << u"/digits?field=01234asdf56"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/digits?field="_qs << QByteArray() << valid;

    QTest::newRow("length-valid") << u"/digitsLength?field=0123456789"_qs << QByteArray() << valid;

    QTest::newRow("length-invalid") << u"/digitsLength?field=012345"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorDigitsBetween_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDigitsBetween *****

    QTest::newRow("valid") << u"/digitsBetween?field=0123456"_qs << QByteArray() << valid;

    QTest::newRow("invalid") << u"/digitsBetween?field=01234ad56"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/digitsBetween?field="_qs << QByteArray() << valid;

    QTest::newRow("invalid-lower") << u"/digitsBetween?field=0123"_qs << QByteArray() << invalid;

    QTest::newRow("invalid-greater")
        << u"/digitsBetween?field=0123456789123"_qs << QByteArray() << invalid;
}

void TestValidator::testValidatorDomain_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorDomain *****

    QByteArray domainBody =
        QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"huessenbergnetz.de"_qs);
    QTest::newRow("valid01") << u"/domain"_qs << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"a.de"_qs);
    QTest::newRow("valid02") << u"/domain"_qs << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"a1.de"_qs);
    QTest::newRow("valid03") << u"/domain"_qs << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.com."_qs);
    QTest::newRow("valid04") << u"/domain"_qs << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"test-1.example.com."_qs);
    QTest::newRow("valid05") << u"/domain"_qs << domainBody << valid;

    // label with max length of 63 chars
    domainBody = QByteArrayLiteral("field=") +
                 QUrl::toPercentEncoding(
                     u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com"_qs);
    QTest::newRow("valid06") << u"/domain"_qs << domainBody << valid;

    // total length of 253 chars
    domainBody = QByteArrayLiteral("field=") +
                 QUrl::toPercentEncoding(
                     u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com"_qs);
    QTest::newRow("valid07") << u"/domain"_qs << domainBody << valid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"hüssenbergnetz.de"_qs);
    QTest::newRow("valid08") << u"/domain"_qs << domainBody << valid;

    domainBody =
        QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"موقع.وزارة-الاتصالات.مصر"_qs);
    QTest::newRow("valid09") << u"/domain"_qs << domainBody << valid;
#endif

    // digit in non puny code TLD
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.com1"_qs);
    QTest::newRow("invalid01") << u"/domain"_qs << domainBody << invalid;

    // one char tld
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.c"_qs);
    QTest::newRow("invalid02") << u"/domain"_qs << domainBody << invalid;

    // starts with digit
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.3com"_qs);
    QTest::newRow("invalid03") << u"/domain"_qs << domainBody << invalid;

    // contains digit
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.co3m"_qs);
    QTest::newRow("invalid04") << u"/domain"_qs << domainBody << invalid;

    // label too long, 64 chars
    domainBody = QByteArrayLiteral("field=") +
                 QUrl::toPercentEncoding(
                     u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl.com"_qs);
    QTest::newRow("invalid05") << u"/domain"_qs << domainBody << invalid;

    // too long, 254 chars
    domainBody = QByteArrayLiteral("field=") +
                 QUrl::toPercentEncoding(
                     u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk."
                     "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com"_qs);
    QTest::newRow("invalid06") << u"/domain"_qs << domainBody << invalid;

    // contains dash in tld
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.co-m"_qs);
    QTest::newRow("invalid07") << u"/domain"_qs << domainBody << invalid;

    // contains dash at label start
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"-example.com"_qs);
    QTest::newRow("invalid08") << u"/domain"_qs << domainBody << invalid;

    // contains digit at label start
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"3example.com"_qs);
    QTest::newRow("invalid09") << u"/domain"_qs << domainBody << invalid;

    // contains dash at label end
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example-.com"_qs);
    QTest::newRow("invalid10") << u"/domain"_qs << domainBody << invalid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    domainBody =
        QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"موقع.وزارة-الاتصالات.مصر1"_qs);
    QTest::newRow("invalid11") << u"/domain"_qs << domainBody << invalid;

    domainBody =
        QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"موقع.وزارة-الاتصالات.مصر-"_qs);
    QTest::newRow("invalid12") << u"/domain"_qs << domainBody << invalid;
#endif

    if (qEnvironmentVariableIsSet("CUTELYST_VALIDATORS_TEST_NETWORK")) {
        domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"example.com"_qs);
        QTest::newRow("dns-valid") << u"/domainDns"_qs << domainBody << valid;

        domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"test.example.com"_qs);
        QTest::newRow("dns-invalid") << u"/domainDns"_qs << domainBody << invalid;
    }
}

void TestValidator::testValidatorEmail_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorEmail *****

    const QList<QString> validEmails(
        {u"test@huessenbergnetz.de"_qs,
         // addresses are taken from
         // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
         u"test@iana.org"_qs,
         u"test@nominet.org.uk"_qs,
         u"test@about.museum"_qs,
         u"a@iana.org"_qs,
         u"test.test@iana.org"_qs,
         u"!#$%&`*+/=?^`{|}~@iana.org"_qs,
         u"123@iana.org"_qs,
         u"test@123.com"_qs,
         u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@iana.org"_qs,
         u"test@mason-dixon.com"_qs,
         u"test@c--n.com"_qs,
         u"test@xn--hxajbheg2az3al.xn--jxalpdlp"_qs,
         u"xn--test@iana.org"_qs,
         // addresses are taken from
         // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
         u"first.last@iana.org"_qs,
         u"1234567890123456789012345678901234567890123456789012345678901234@iana.org"_qs,
         u"first.last@3com.com"_qs,
         u"user+mailbox@iana.org"_qs,
         u"customer/department=shipping@iana.org"_qs,
         u"$A12345@iana.org"_qs,
         u"!def!xyz%abc@iana.org"_qs,
         u"_somename@iana.org"_qs,
         u"dclo@us.ibm.com"_qs,
         u"peter.piper@iana.org"_qs,
         u"TEST@iana.org"_qs,
         u"1234567890@iana.org"_qs,
         u"test+test@iana.org"_qs,
         u"test-test@iana.org"_qs,
         u"t*est@iana.org"_qs,
         u"+1~1+@iana.org"_qs,
         u"{_test_}@iana.org"_qs,
         u"test.test@iana.org"_qs,
         u"customer/department@iana.org"_qs,
         u"Yosemite.Sam@iana.org"_qs,
         u"~@iana.org"_qs,
         u"Ima.Fool@iana.org"_qs,
         u"name.lastname@domain.com"_qs,
         u"a@bar.com"_qs,
         u"a-b@bar.com"_qs,
         u"valid@about.museum"_qs,
         u"user%uucp!path@berkeley.edu"_qs,
         u"cdburgess+!#$%&'*-/=?+_{}|~test@gmail.com"_qs});

    const QList<QString> dnsWarnEmails({
        u"test@example.com"_qs, // no mx record
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        u"test@e.com"_qs,                                                               // no record
        u"test@iana.a"_qs,                                                              // no record
        u"test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.com"_qs, // no record
        u"test@iana.co-uk"_qs,                                                          // no record
        u"a@a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j."
        "k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u."
        "v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f."
        "g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v"_qs, // no record
        u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghi"_qs,
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"x@x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456789.x23456789.x23456789.x2"_qs, // no record
        u"1234567890123456789012345678901234567890123456789012345678901@"
        "12345678901234567890123456789012345678901234567890123456789."
        "12345678901234567890123456789012345678901234567890123456789."
        "123456789012345678901234567890123456789012345678901234567890123.iana.org"_qs, // no record
        // no record
        u"first.last@x23456789012345678901234567890123456789012345678901234567890123.iana.org"_qs,
        u"first.last@123.iana.org"_qs,          // no record
        u"test@123.123.123.x123"_qs,            // no record
        u"test@example.iana.org"_qs,            // no record
        u"test@example.example.iana.org"_qs,    // no record
        u"+@b.c"_qs,                            // no record
        u"+@b.com"_qs,                          // no record
        u"a@b.co-foo.uk"_qs,                    // no record
        u"shaitan@my-domain.thisisminekthx"_qs, // no record
        u"test@xn--example.com"_qs              // no record
    });

    const QList<QString> rfc5321Emails({
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"\"first\\\"last\"@iana.org"_qs,                                  // quoted string
        u"\"first@last\"@iana.org"_qs,                                     // quoted string
        u"\"first\\\\last\"@iana.org"_qs,                                  // quoted string
        u"first.last@[12.34.56.78]"_qs,                                    // address literal
        u"first.last@[IPv6:::12.34.56.78]"_qs,                             // address literal
        u"first.last@[IPv6:1111:2222:3333::4444:12.34.56.78]"_qs,          // address literal
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:12.34.56.78]"_qs, // address literal
        u"first.last@[IPv6:::1111:2222:3333:4444:5555:6666]"_qs,           // address literal
        u"first.last@[IPv6:1111:2222:3333::4444:5555:6666]"_qs,            // address literal
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666::]"_qs,           // address literal
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888]"_qs,   // address literal
        u"\"first\\last\"@iana.org"_qs,                                    // quoted string
        u"\"\"@iana.org"_qs,                                               // quoted string
        u"first.last@[IPv6:1111:2222:3333::4444:5555:12.34.56.78]"_qs,     // ipv6 deprecated
        u"first.last@example.123"_qs,                                      // tld numeric
        u"first.last@com"_qs,                                              // tld
        u"\"Abc\\@def\"@iana.org"_qs,                                      // quoted string
        u"\"Fred\\ Bloggs\"@iana.org"_qs,                                  // quoted string
        u"\"Joe.\\\\Blow\"@iana.org"_qs,                                   // quoted string
        u"\"Abc@def\"@iana.org"_qs,                                        // quoted string
        u"\"Fred Bloggs\"@iana.org"_qs,                                    // quoted string
        u"\"Doug \\\"Ace\\\" L.\"@iana.org"_qs,                            // quoted string
        u"\"[[ test ]]\"@iana.org"_qs,                                     // quoted string
        u"\"test.test\"@iana.org"_qs,                                      // quoted string
        u"\"test@test\"@iana.org"_qs,                                      // quoted string
        u"test@123.123.123.123"_qs,                                        // tld numeric
        u"test@[123.123.123.123]"_qs,                                      // address literal
        u"\"test\\test\"@iana.org"_qs,                                     // quoted string
        u"test@example"_qs,                                                // tld
        u"\"test\\\\blah\"@iana.org"_qs,                                   // quoted string
        u"\"test\\blah\"@iana.org"_qs,                                     // quoted string
        u"\"test\\\"blah\"@iana.org"_qs,                                   // quoted string
        u"\"Austin@Powers\"@iana.org"_qs,                                  // quoted string
        u"\"Ima.Fool\"@iana.org"_qs,                                       // quoted string
        u"\"Ima Fool\"@iana.org"_qs,                                       // quoted string
        u"\"first.middle.last\"@iana.org"_qs,                              // quoted string
        u"\"first..last\"@iana.org"_qs,                                    // quoted string
        u"\"first\\\\\\\"last\"@iana.org"_qs,                              // quoted string
        u"a@b"_qs,                                                         // tld
        u"aaa@[123.123.123.123]"_qs,                                       // address literal
        u"a@bar"_qs,                                                       // tld
        u"\"hello my name is\"@stutter.com"_qs,                            // quoted string
        u"\"Test \\\"Fail\\\" Ing\"@iana.org"_qs,                          // quoted string
        u"foobar@192.168.0.1"_qs,                                          // tld numeric
        u"\"Joe\\\\Blow\"@iana.org"_qs,                                    // quoted string
        u"\"first(last)\"@iana.org"_qs,                                    // quoted string
        u"first.last@[IPv6:::a2:a3:a4:b1:b2:b3:b4]"_qs,                    // ipv6 deprecated
        u"first.last@[IPv6:a1:a2:a3:a4:b1:b2:b3::]"_qs,                    // ipv6 deprecated
        u"first.last@[IPv6:::]"_qs,                                        // address literal
        u"first.last@[IPv6:::b4]"_qs,                                      // address literal
        u"first.last@[IPv6:::b3:b4]"_qs,                                   // address literal
        u"first.last@[IPv6:a1::b4]"_qs,                                    // address literal
        u"first.last@[IPv6:a1::]"_qs,                                      // address literal
        u"first.last@[IPv6:a1:a2::]"_qs,                                   // address literal
        u"first.last@[IPv6:0123:4567:89ab:cdef::]"_qs,                     // address literal
        u"first.last@[IPv6:0123:4567:89ab:CDEF::]"_qs,                     // address literal
        u"first.last@[IPv6:::a3:a4:b1:ffff:11.22.33.44]"_qs,               // address literal
        u"first.last@[IPv6:::a2:a3:a4:b1:ffff:11.22.33.44]"_qs,            // ipv6 deprecated
        u"first.last@[IPv6:a1:a2:a3:a4::11.22.33.44]"_qs,                  // address literal
        u"first.last@[IPv6:a1:a2:a3:a4:b1::11.22.33.44]"_qs,               // ipv6 deprecated
        u"first.last@[IPv6:a1::11.22.33.44]"_qs,                           // address literal
        u"first.last@[IPv6:a1:a2::11.22.33.44]"_qs,                        // address literal
        u"first.last@[IPv6:0123:4567:89ab:cdef::11.22.33.44]"_qs,          // address literal
        u"first.last@[IPv6:0123:4567:89ab:CDEF::11.22.33.44]"_qs,          // address literal
        u"first.last@[IPv6:a1::b2:11.22.33.44]"_qs,                        // address literal

        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        u"test@iana.123"_qs,                                             // tld numeric
        u"test@255.255.255.255"_qs,                                      // tld numeric
        u"\"test\"@iana.org"_qs,                                         // quoted string
        u"\"\"@iana.org"_qs,                                             // quoted string
        u"\"\\a\"@iana.org"_qs,                                          // quoted string
        u"\"\\\"\"@iana.org"_qs,                                         // quoted string
        u"\"\\\\\"@iana.org"_qs,                                         // quoted string
        u"\"test\\ test\"@iana.org"_qs,                                  // quoted string
        u"test@[255.255.255.255]"_qs,                                    // address literal
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888]"_qs,       // address literal
        u"test@[IPv6:1111:2222:3333:4444:5555:6666::8888]"_qs,           // ipv6 deprecated
        u"test@[IPv6:1111:2222:3333:4444:5555::8888]"_qs,                // address literal
        u"test@[IPv6:::3333:4444:5555:6666:7777:8888]"_qs,               // address literal
        u"test@[IPv6:::]"_qs,                                            // address literal
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:255.255.255.255]"_qs, // address literal
        u"test@[IPv6:1111:2222:3333:4444::255.255.255.255]"_qs,          // address literal
        u"test@org"_qs                                                   // tld
    });

    const QList<QString> cfwsEmails({
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        u"\r\n test@iana.org"_qs,              // folding white space
        u"(comment)test@iana.org"_qs,          // comment
        u"(comment(comment))test@iana.org"_qs, // coment
        u"(comment)abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@iana.org"_qs, // comment
        u"(comment)test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghik."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghik."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk."
        "abcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstu"_qs, // comment
        u" \r\n test@iana.org"_qs,                                        // folding white space
        u"test@iana.org\r\n "_qs,                                         // folding white space
        u"test@iana.org \r\n "_qs,                                        // folding white space
        u" test@iana.org"_qs,                                             // folding white space
        u"test@iana.org "_qs,                                             // folding white space

        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"\"test\r\n blah\"@iana.org"_qs, // folding white space
        u"first.last@iana("
        "1234567890123456789012345678901234567890123456789012345678901234567890)."
        "org"_qs // comment
    });

    const QList<QString> deprecatedEmails({
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        u"\"test\".\"test\"@iana.org"_qs, // local part
        u"\"test\".test@iana.org"_qs,     // local part
        // u"\"test\\\0\"@iana.org"_qs, // quoted pair
        u" test @iana.org"_qs,                 // folding white space near at
        u"test@ iana .com"_qs,                 // folding white space near at
        u"test . test@iana.org"_qs,            // folding white space
        u"\r\n \r\n test@iana.org"_qs,         // folding white space
        u"test@(comment)iana.org"_qs,          // comment near at
        u"test@(comment)[255.255.255.255]"_qs, // comment near at
        // comment near at
        u"test@(comment)abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.com"_qs,
        // quoted string with deprecated char - currently also not working on upstream
        // u"\"\"@iana.org"_qs,
        // quoted string with deprecated char
        // u"\"\\\"@iana.org"_qs,
        // comment string with deprecated char - currently also not working on upstream
        // u"()test@iana.org"_qs,
        u"\"\\\n\"@iana.org"_qs,           // quoted pair with deprecated char
        u"\"\a\"@iana.org"_qs,             // quoted string with deprecated char
        u"\"\\\a\"@iana.org"_qs,           // quoted pair with deprecated char
        u"(\a)test@iana.org"_qs,           // comment with deprecated char
        u"test@iana.org\r\n \r\n "_qs,     // obsolete folding white space
        u"test.(comment)test@iana.org"_qs, // deprecated comment position

        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"test.\"test\"@iana.org"_qs,                     // local part
        u"\"test\\\rblah\"@iana.org"_qs,                  // quoted pair with deprecated char
        u"\"first\".\"last\"@iana.org"_qs,                // local part
        u"\"first\".middle.\"last\"@iana.org"_qs,         // local part
        u"\"first\".last@iana.org"_qs,                    // local part
        u"first.\"last\"@iana.org"_qs,                    // local part
        u"\"first\".\"middle\".\"last\"@iana.org"_qs,     // local part
        u"\"first.middle\".\"last\"@iana.org"_qs,         // local part
        u"first.\"mid\\dle\".\"last\"@iana.org"_qs,       // local part
        u"Test.\r\n Folding.\r\n Whitespace@iana.org"_qs, // folding white space
        u"first.\"\".last@iana.org"_qs,                   // local part
        u"(foo)cal(bar)@(baz)iamcal.com(quux)"_qs,        // comment near at
        u"cal@iamcal(woo).(yay)com"_qs,                   // comment position
        u"\"foo\"(yay)@(hoopla)[1.2.3.4]"_qs,             // comment near at
        u"cal(woo(yay)hoopla)@iamcal.com"_qs,             // comment near at
        u"cal(foo\\@bar)@iamcal.com"_qs,                  // comment near at
        u"cal(foo\\)bar)@iamcal.com"_qs,                  // comment near at
        u"first().last@iana.org"_qs,                      // local part
        u"first.(\r\n middle\r\n )last@iana.org"_qs,      // deprecated comment
        // comment near at
        u"first(Welcome to\r\n the (\"wonderful\" (!)) world\r\n of email)@iana.org"_qs,
        u"pete(his account)@silly.test(his host)"_qs, // comment near at
        u"c@(Chris's host.)public.example"_qs,        // comment near at
        u"jdoe@machine(comment). example"_qs,         // folding white space
        u"1234 @ local(blah) .machine .example"_qs,   // white space near at
        u"first(abc.def).last@iana.org"_qs,           // local part
        u"first(a\"bc.def).last@iana.org"_qs,         // local part
        u"first.(\")middle.last(\")@iana.org"_qs,     // local part
        u"first(abc\\(def)@iana.org"_qs,              // comment near at
        u"a(a(b(c)d(e(f))g)h(i)j)@iana.org"_qs,
        u"HM2Kinsists@(that comments are allowed)this.is.ok"_qs, // comment near at
        u" \r\n (\r\n x \r\n ) \r\n first\r\n ( \r\n x\r\n ) \r\n .\r\n ( \r\n x) "
        "\r\n last \r\n ( x \r\n ) \r\n @iana.org"_qs, // folding white space near at
        u"first.last @iana.org"_qs,                    // folding white space near at
        u"test. \r\n \r\n obs@syntax.com"_qs           // folding white space
        // u"\"Unicode NULL\\\0\"@char.com"_qs // quoted pair contains deprecated char
    });

    const QList<QString> rfc5322Emails({
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        // local too long
        u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklmn@iana.org"_qs,
        // label too long
        u"test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm.com"_qs,
        u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"_qs, // too long
        u"a@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg.hij"_qs, // too long
        u"a@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl."
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg.hijk"_qs, // too long
        // local too long
        u"\"abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghj\"@iana.org"_qs,
        // local too long
        u"\"abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefg\\h\"@iana.org"_qs,
        u"test@[255.255.255]"_qs,                                       // invalid domain literal
        u"test@[255.255.255.255.255]"_qs,                               // invalid domain litearl
        u"test@[255.255.255.256]"_qs,                                   // invalid domain literal
        u"test@[1111:2222:3333:4444:5555:6666:7777:8888]"_qs,           // invalid domain literal
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:7777]"_qs,           // ipv6 group count
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888:9999]"_qs, // ipv6 group count
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:7777:888G]"_qs,      // ipv6 bad char
        u"test@[IPv6:1111:2222:3333:4444:5555:6666::7777:8888]"_qs,     // ipv6 max groups
        u"test@[IPv6::3333:4444:5555:6666:7777:8888]"_qs,               // ipv6 colon start
        u"test@[IPv6:1111::4444:5555::8888]"_qs,                        // ipv6 2x2x colon
        u"test@[IPv6:1111:2222:3333:4444:5555:255.255.255.255]"_qs,     // ipv6 group count
        u"test@[IPv6:1111:2222:3333:4444:5555:6666:7777:255.255.255.255]"_qs, // ipv6 group count
        u"test@[IPv6:1111:2222:3333:4444:5555:6666::255.255.255.255]"_qs,     // ipv6 max groups
        u"test@[IPv6:1111:2222:3333:4444:::255.255.255.255]"_qs,              // ipv6 2x2x colon
        u"test@[IPv6::255.255.255.255]"_qs,                                   // ipv6 colon start
        u"test@[RFC-5322-domain-literal]"_qs, // invalid domain literal
        // invalid domain literal containing obsolete chars
        u"test@[RFC-5322-\\\a-domain-literal]"_qs,
        // invalid domain literal containing obsolete chars
        u"test@[RFC-5322-\\\t-domain-literal]"_qs,
        // invalid domain literal containing obsolete chars
        u"test@[RFC-5322-\\]-domain-literal]"_qs,
        u"test@[RFC 5322 domain literal]"_qs,           // invalid domain literal
        u"test@[RFC-5322-domain-literal] (comment)"_qs, // invalid domain literal
        u"test@[IPv6:1::2:]"_qs,                        // ipv6 colon end
        u"test@iana/icann.org"_qs,                      // domain invalid for DNS

        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"123456789012345678901234567890123456789012345678901234567890@"
        "12345678901234567890123456789012345678901234567890123456789."
        "12345678901234567890123456789012345678901234567890123456789."
        "12345678901234567890123456789012345678901234567890123456789.12345.iana."
        "org"_qs, // too long
        u"12345678901234567890123456789012345678901234567890123456789012345@iana."
        "org"_qs, // local too long
        u"x@x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789."
        "x23456789.x23456"_qs,                                        // domain too long
        u"first.last@[.12.34.56.78]"_qs,                              // invalid domain literal
        u"first.last@[12.34.56.789]"_qs,                              // invalid domain literal
        u"first.last@[::12.34.56.78]"_qs,                             // invalid domain literal
        u"first.last@[IPv5:::12.34.56.78]"_qs,                        // invalid domain literal
        u"first.last@[IPv6:1111:2222:3333:4444:5555:12.34.56.78]"_qs, // ipv6 group count
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:12.34.56.78]"_qs, // ipv6 group count
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777]"_qs,             // ipv6 group count
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888:9999]"_qs,   // ipv6 group count
        u"first.last@[IPv6:1111:2222::3333::4444:5555:6666]"_qs,                // ipv6 2x2x colon
        u"first.last@[IPv6:1111:2222:333x::4444:5555]"_qs,                      // ipv6 bad char
        u"first.last@[IPv6:1111:2222:33333::4444:5555]"_qs,                     // ipv6 bad char
        // label too long
        u"first.last@x234567890123456789012345678901234567890123456789012345678901234.iana.org"_qs,
        u"test@123456789012345678901234567890123456789012345678901234567890123."
        "123456789012345678901234567890123456789012345678901234567890123."
        "123456789012345678901234567890123456789012345678901234567890123."
        "123456789012345678901234567890123456789012345678901234567890.com"_qs, // domain too long
        u"foo@[\\1.2.3.4]"_qs, // invalid domain literal containing obsolete chars
        u"first.last@[IPv6:1111:2222:3333:4444:5555:6666:12.34.567.89]"_qs, // ipv6 bad char
        u"aaa@[123.123.123.333]"_qs,                               // invalid domain literal
        u"first.last@[IPv6::]"_qs,                                 // ipv6 colon start
        u"first.last@[IPv6::::]"_qs,                               // ipv6 2x2x colon
        u"first.last@[IPv6::b4]"_qs,                               // ipv6 colon start
        u"first.last@[IPv6::::b4]"_qs,                             // ipv6 2x2x colon
        u"first.last@[IPv6::b3:b4]"_qs,                            // ipv6 colon start
        u"first.last@[IPv6::::b3:b4]"_qs,                          // ipv6 2x2x colon
        u"first.last@[IPv6:a1:::b4]"_qs,                           // ipv6 2x2x colon
        u"first.last@[IPv6:a1:]"_qs,                               // ipv6 colon end
        u"first.last@[IPv6:a1:::]"_qs,                             // ipv6 2x2x colon
        u"first.last@[IPv6:a1:a2:]"_qs,                            // ipv6 colon end
        u"first.last@[IPv6:a1:a2:::]"_qs,                          // ipv6 2x2x colon
        u"first.last@[IPv6::11.22.33.44]"_qs,                      // ipv6 colon start
        u"first.last@[IPv6::::11.22.33.44]"_qs,                    // ipv6 2x2x colon
        u"first.last@[IPv6:a1:11.22.33.44]"_qs,                    // ipv6 group count
        u"first.last@[IPv6:a1:::11.22.33.44]"_qs,                  // ipv6 2x2x colon
        u"first.last@[IPv6:a1:a2:::11.22.33.44]"_qs,               // ipv6 2x2x colon
        u"first.last@[IPv6:0123:4567:89ab:cdef::11.22.33.xx]"_qs,  // ipv6 bad char
        u"first.last@[IPv6:0123:4567:89ab:CDEFF::11.22.33.44]"_qs, // ipv6 bad char
        u"first.last@[IPv6:a1::a4:b1::b4:11.22.33.44]"_qs,         // ipv6 2x2x colon
        u"first.last@[IPv6:a1::11.22.33]"_qs,                      // ipv6 bad char
        u"first.last@[IPv6:a1::11.22.33.44.55]"_qs,                // ipv6 bad char
        u"first.last@[IPv6:a1::b211.22.33.44]"_qs,                 // ipv6 bad char
        u"first.last@[IPv6:a1::b2::11.22.33.44]"_qs,               // ipv6 2x2x colon
        u"first.last@[IPv6:a1::b3:]"_qs,                           // ipv6 colon end
        u"first.last@[IPv6::a2::b4]"_qs,                           // ipv6 colon start
        u"first.last@[IPv6:a1:a2:a3:a4:b1:b2:b3:]"_qs,             // ipv6 colon end
        u"first.last@[IPv6::a2:a3:a4:b1:b2:b3:b4]"_qs,             // ipv6 colon end
        u"first.last@[IPv6:a1:a2:a3:a4::b1:b2:b3:b4]"_qs           // ipv6 max groups
    });

    QList<QString> errorEmails({
        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
        u" "_qs,                         // no domain
        u"test"_qs,                      // no domain
        u"@"_qs,                         // no local part
        u"test@"_qs,                     // no domain
        u"@io"_qs,                       // no local part
        u"@iana.org"_qs,                 // no local part
        u".test@iana.org"_qs,            // dot start
        u"test.@iana.org"_qs,            // dot end
        u"test..iana.org"_qs,            // consecutive dots
        u"test_exa-mple.com"_qs,         // no domain
        u"test\\@test@iana.org"_qs,      // expecting atext
        u"test@-iana.org"_qs,            // domain hypen start
        u"test@iana-.com"_qs,            // domain hypen end
        u"test@.iana.org"_qs,            // dot start
        u"test@iana.org."_qs,            // dot end
        u"test@iana..com"_qs,            // consecutive dots
        u"\"\"\"@iana.org"_qs,           // expecting atext
        u"\"\\\"@iana.org"_qs,           // unclosed quoted string
        u"test\"@iana.org"_qs,           // expecting atext
        u"\"test@iana.org"_qs,           // unclosed quoted string
        u"\"test\"test@iana.org"_qs,     // atext after quoted string
        u"test\"text\"@iana.org"_qs,     // expecting atext
        u"\"test\"\"test\"@iana.org"_qs, // expecting atext
        // u"\"test\0\"@iana.org"_qs, // expecting qtext
        u"test@a[255.255.255.255]"_qs,          // expecting atext
        u"((comment)test@iana.org"_qs,          // unclosed comment
        u"test(comment)test@iana.org"_qs,       // atext after comment
        u"test@iana.org\n"_qs,                  // expecting atext
        u"test@iana.org-"_qs,                   // domain hypehn end
        u"\"test@iana.org"_qs,                  // unclosed quoted string
        u"(test@iana.org"_qs,                   // unclosed comment
        u"test@(iana.org"_qs,                   // unclosed comment
        u"test@[1.2.3.4"_qs,                    // unclosed domain literal
        u"\"test\\\"@iana.org"_qs,              // unclosed quoted string
        u"(comment\\)test@iana.org"_qs,         // unclosed comment
        u"test@iana.org(comment\\)"_qs,         // unclosed comment
        u"test@iana.org(comment\\"_qs,          // backslash end
        u"test@[RFC-5322]-domain-literal]"_qs,  // atext after domain literal
        u"test@[RFC-5322-[domain-literal]"_qs,  // expecting dtext
        u"test@[RFC-5322-domain-literal\\]"_qs, // unclosed domain literal
        u"test@[RFC-5322-domain-literal\\"_qs,  // backslash end
        u"@iana.org"_qs,                        // expecting atext
        u"test@.org"_qs,                        // expecting atext
        u"test@iana.org\r"_qs,                  // no lf after cr
        u"\rtest@iana.org"_qs,                  // no lf after cr
        u"\"\rtest\"@iana.org"_qs,              // no lf after cr
        u"(\r)test@iana.org"_qs,                // no lf after cr
        u"test@iana.org(\r)"_qs,                // no lf after cr
        u"\ntest@iana.org"_qs,                  // expecting atext
        u"\"\n\"@iana.org"_qs,                  // expecting qtext
        u"(\n)test@iana.org"_qs,                // expecting ctext
        u"\a@iana.org"_qs,                      // expecting atext
        u"test@\a.org"_qs,                      // expecting atext
        u"\r\ntest@iana.org"_qs,                // folding white space ends with CRLF
        u"\r\n \r\ntest@iana.org"_qs,           // folding white space ends with CRLF
        u" \r\ntest@iana.org"_qs,               // folding white space ends with CRLF
        u" \r\n \r\ntest@iana.org"_qs,          // folding white space ends with CRLF
        u" \r\n\r\ntest@iana.org"_qs,  // Folding White Space contains consecutive CRLF sequences
        u" \r\n\r\n test@iana.org"_qs, // Folding White Space contains consecutive CRLF sequences
        u"test@iana.org\r\n"_qs,       // Folding White Space ends with a CRLF sequence
        u"test@iana.org\r\n \r\n"_qs,  // Folding White Space ends with a CRLF sequence
        u"test@iana.org \r\n"_qs,      // Folding White Space ends with a CRLF sequence
        u"test@iana.org \r\n \r\n"_qs, // Folding White Space ends with a CRLF sequence
        u"test@iana.org \r\n\r\n"_qs,  // Folding White Space contains consecutive CRLF sequences
        u"test@iana.org \r\n\r\n "_qs, // Folding White Space contains consecutive CRLF sequences
        u"\"test\\©\"@iana.org"_qs,    // expecting quoted pair

        // addresses are taken from
        // https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
        u"first.last@sub.do,com"_qs,                // expecting atext
        u"first\\@last@iana.org"_qs,                // expecting atext
        u"first.last"_qs,                           // no domain
        u".first.last@iana.org"_qs,                 // dot start
        u"first.last.@iana.org"_qs,                 // dot end
        u"first..last@iana.org"_qs,                 // consecutive dots
        u"\"first\"last\"@iana.org"_qs,             // atext after quoted string
        u"\"\"\"@iana.org"_qs,                      // expecting atext
        u"\"\\\"@iana.org"_qs,                      // unclosed quoted string
        u"first\\\\@last@iana.org"_qs,              // expecting atext
        u"first.last@"_qs,                          // no domain
        u"first.last@-xample.com"_qs,               // domain hyphen start
        u"first.last@exampl-.com"_qs,               // domain hyphen end
        u"abc\\@def@iana.org"_qs,                   // expecting atext
        u"abc\\\\@iana.org"_qs,                     // expecting atext
        u"Doug\\ \\\"Ace\\\"\\ Lovell@iana.org"_qs, // expecting atext
        u"abc@def@iana.org"_qs,                     // expecting atext
        u"abc\\\\@def@iana.org"_qs,                 // expecting atext
        u"abc\\@iana.org"_qs,                       // expecting atext
        u"@iana.org"_qs,                            // no local part
        u"doug@"_qs,                                // no domain
        u"\"qu@iana.org"_qs,                        // unclosed quoted string
        u"ote\"@iana.org"_qs,                       // expecting atext
        u".dot@iana.org"_qs,                        // dot start
        u"dot.@iana.org"_qs,                        // dot end
        u"two..dot@iana.org"_qs,                    // consecutive dots
        u"\"Doug \"Ace\" L.\"@iana.org"_qs,         // atext after quoted string
        u"Doug\\ \\\"Ace\\\"\\ L\\.@iana.org"_qs,   // expecting atext
        u"hello world@iana.org"_qs,                 // atext after folding white space
        u"gatsby@f.sc.ot.t.f.i.tzg.era.l.d."_qs,    // dot end
        u"test.iana.org"_qs,                        // no domain
        u"test.@iana.org"_qs,                       // dot end
        u"test..test@iana.org"_qs,                  // consecutive dots
        u".test@iana.org"_qs,                       // dot start
        u"test@test@iana.org"_qs,                   // expecting atext
        u"test@@iana.org"_qs,                       // expecting atext
        u"-- test --@iana.org"_qs,                  // atext after folding white space
        u"[test]@iana.org"_qs,                      // expecting atext
        u"\"test\"test\"@iana.org"_qs,              // atext after quoted string
        u"()[]\\;:,><@iana.org"_qs,                 // expecting atext
        u"test@."_qs,                               // dot start
        u"test@example."_qs,                        // dot end
        u"test@.org"_qs,                            // dot start
        u"test@[123.123.123.123"_qs,                // unclosed domain literal
        u"test@123.123.123.123]"_qs,                // expecting atext
        u"NotAnEmail"_qs,                           // no domain
        u"@NotAnEmail"_qs,                          // no local part
        u"\"test\rblah\"@iana.org"_qs,              // cr no lf
        u"\"test\"blah\"@iana.org"_qs,              // atext after quoted string
        u".wooly@iana.org"_qs,                      // dot start
        u"wo..oly@iana.org"_qs,                     // consecutive dots
        u"pootietang.@iana.org"_qs,                 // dot end
        u".@iana.org"_qs,                           // dot start
        u"Ima Fool@iana.org"_qs,                    // atext after white space
        u"phil.h\\@\\@ck@haacked.com"_qs,           // expecting atext
        u"\"first\\\\\"last\"@iana.org"_qs,         // atext after quoted string
        u"first\\last@iana.org"_qs,                 // expecting atext
        u"Abc\\@def@iana.org"_qs,                   // expecting atext
        u"Fred\\ Bloggs@iana.org"_qs,               // expectin atext
        u"Joe.\\\\Blow@iana.org"_qs,                // expecting atext
        u"\"test\\\r\n blah\"@iana.org"_qs,         // expecting qtext
        u"{^c\\@**Dog^}@cartoon.com"_qs,            // expecting atext
        u"cal(foo(bar)@iamcal.com"_qs,              // unclosed comment
        u"cal(foo)bar)@iamcal.com"_qs,              // atext after comment
        u"cal(foo\\)@iamcal.com"_qs,                // unclosed comment
        u"first(12345678901234567890123456789012345678901234567890)last@("
        "123456789012345678901234567890123456789012345678901234567890123456789012345"
        "678901234567890123456789012345678901234567890123456789012345678901234567890"
        "123456789012345678901234567890123456789012345678901234567890123456789012345"
        "6789012345678901234567890)iana.org"_qs, // atext after comment
        u"first(middle)last@iana.org"_qs,        // atext after comment
        u"first(abc(\"def\".ghi).mno)middle(abc(\"def\".ghi).mno).last@(abc(\"def\"."
        "ghi).mno)example(abc(\"def\".ghi).mno).(abc(\"def\".ghi).mno)com(abc("
        "\"def\".ghi).mno)"_qs,                              // atext after comment
        u"a(a(b(c)d(e(f))g)(h(i)j)@iana.org"_qs,             // unclosed comment
        u".@"_qs,                                            // dot start
        u"@bar.com"_qs,                                      // no local part
        u"@@bar.com"_qs,                                     // no local part
        u"aaa.com"_qs,                                       // no domain
        u"aaa@.com"_qs,                                      // dot start
        u"aaa@.123"_qs,                                      // dot start
        u"aaa@[123.123.123.123]a"_qs,                        // atext after domain literal
        u"a@bar.com."_qs,                                    // dot end
        u"a@-b.com"_qs,                                      // domain hyphen start
        u"a@b-.com"_qs,                                      // domain hypen end
        u"-@..com"_qs,                                       // dot start
        u"-@a..com"_qs,                                      // consecutive dots
        u"invalid@about.museum-"_qs,                         // domain hyphen end
        u"test@...........com"_qs,                           // dot start
        u"Invalid \\\n Folding \\\n Whitespace@iana.org"_qs, // atext after white space
        // Folding White Space contains consecutive CRLF sequences
        u"test.\r\n\r\n obs@syntax.com"_qs,
        // u"\"Unicode NULL \0\"@char.com"_qs, // expecting qtext
        // u"Unicode NULL \\0@char.com"_qs, // atext after cfws
        u"test@example.com\n"_qs // expecting atext
    });

    int count = 0;
    for (const QString &email : validEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"valid-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailValid"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : {u"test@hüssenbergnetz.de"_qs,
                                 u"täst@huessenbergnetz.de"_qs,
                                 u"täst@hüssenbergnetz.de"_qs}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"valid-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailValid"_qs << body << invalid;
        count++;
    }

    if (qEnvironmentVariableIsSet("CUTELYST_VALIDATORS_TEST_NETWORK")) {
        QTest::newRow("valid-dns") << u"/emailDnsWarnValid"_qs
                                   << QByteArrayLiteral("field=test@huessenbergnetz.de") << valid;
        count = 0;
        for (const QString &email : dnsWarnEmails) {
            const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
            QTest::newRow(u"dnswarn-valid-%1"_qs.arg(count).toUtf8().constData())
                << u"/emailDnsWarnValid"_qs << body << invalid;
            count++;
        }
    }

    count = 0;
    for (const QString &email : rfc5321Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"rfc5321-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailRfc5321Valid"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5321Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"rfc5321-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailRfc5321Invalid"_qs << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : cfwsEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"cfws-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailCfwsValid"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : deprecatedEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"deprecated-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailDeprecatedValid"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : deprecatedEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"deprecated-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailDeprecatedInvalid"_qs << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5322Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"rfc5322-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailRfc5322Valid"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5322Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"rfc5322-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailRfc5322Invalid"_qs << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : errorEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"errors-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailErrors"_qs << body << invalid;
        count++;
    }

    {
        QByteArray body =
            QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"test@hüssenbergnetz.de"_qs);
        QTest::newRow("idnallowed-valid") << u"/emailIdnAllowed"_qs << body << valid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"täst@hüssenbergnetz.de"_qs);
        QTest::newRow("idnallowed-invalid") << u"/emailIdnAllowed"_qs << body << invalid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"täst@huessenbergnetz.de"_qs);
        QTest::newRow("utf8localallowed-valid") << u"/emailUtf8Local"_qs << body << valid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"täst@hüssenbergnetz.de"_qs);
        QTest::newRow("utf8localallowed-invalid") << u"/emailUtf8Local"_qs << body << invalid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(u"täst@hüssenbergnetz.de"_qs);
        QTest::newRow("utf8allowed-valid-0") << u"/emailUtf8"_qs << body << valid;
    }

    count = 1;
    for (const QString &email : validEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"utf8allowed-valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailUtf8"_qs << body << valid;
        count++;
    }

    QList<QString> utf8InvalidEmails;
    utf8InvalidEmails.append(rfc5321Emails);
    utf8InvalidEmails.append(cfwsEmails);
    utf8InvalidEmails.append(deprecatedEmails);
    utf8InvalidEmails.append(rfc5322Emails);
    utf8InvalidEmails.append(errorEmails);

    count = 0;
    for (const QString &email : utf8InvalidEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(u"utf8allowed-invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/emailUtf8"_qs << body << invalid;
        count++;
    }

    QTest::newRow("empty") << u"/emailValid"_qs << QByteArrayLiteral("field=") << valid;
}

void TestValidator::testValidatorFileSize_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorFileSize *****

    int count = 0;
    for (const QString &size :
         {u"1M"_qs,      u"M1"_qs,        u"1 G"_qs,      u"G 1"_qs,         u"1.5 G"_qs,
          u"G 1.5"_qs,   u"2.345 TiB"_qs, u"TiB2.345"_qs, u"5B"_qs,          u"B5"_qs,
          u"5 B"_qs,     u"B 5"_qs,       u" 2.0 Gi"_qs,  u" Gi 2.0"_qs,     u"2.0 Gi "_qs,
          u"Gi 2.0 "_qs, u" 2.0 Gi "_qs,  u" Gi 2.0 "_qs, u" 2.0    Gi "_qs, u" Gi    2.0 "_qs,
          u"3.67YB"_qs,  u"YB3.67"_qs,    u"1"_qs,        u"1024"_qs,        u".5MB"_qs,
          u"MB.5"_qs}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(size);
        QTest::newRow(u"valid-%1"_qs.arg(count).toUtf8().constData())
            << u"/fileSize"_qs << body << valid;
        count++;
    }

    count = 0;
    for (const QString &size : {u"1QiB"_qs,
                                u"QiB1"_qs,
                                u" 1QiB"_qs,
                                u" QiB1"_qs,
                                u"1QiB "_qs,
                                u"QiB1 "_qs,
                                u"1 QiB"_qs,
                                u"Q iB1"_qs,
                                u"1   QiB"_qs,
                                u"Q   iB1"_qs,
                                u"1..4 G"_qs,
                                u"G 1..4"_qs,
                                u"1iB"_qs,
                                u"iB1"_qs,
                                u"1Byte"_qs,
                                u"Byte1"_qs,
                                u"1024iK"_qs,
                                u"iK 2048"_qs}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(size);
        QTest::newRow(u"invalid-%1"_qs.arg(count).toUtf8().constData())
            << u"/fileSize"_qs << body << invalid;
        count++;
    }

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"1,5M"_qs);
    query.addQueryItem(u"locale"_qs, u"de"_qs);
    QTest::newRow("locale-de-valid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5M"_qs);
    query.addQueryItem(u"locale"_qs, u"de"_qs);
    QTest::newRow("locale-de-invalid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    query.clear();
    query.addQueryItem(u"field"_qs, u"1٫5M"_qs);
    query.addQueryItem(u"locale"_qs, u"ar"_qs);
    QTest::newRow("locale-ar-valid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
#endif

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5M"_qs);
    query.addQueryItem(u"locale"_qs, u"ar"_qs);
    QTest::newRow("locale-ar-invalid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5TiB"_qs);
    query.addQueryItem(u"option"_qs, u"OnlyBinary"_qs);
    QTest::newRow("onlybinary-valid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5TB"_qs);
    query.addQueryItem(u"option"_qs, u"OnlyBinary"_qs);
    QTest::newRow("onlybinary-invalid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5TB"_qs);
    query.addQueryItem(u"option"_qs, u"OnlyDecimal"_qs);
    QTest::newRow("onlydecimyl-valid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"1.5TiB"_qs);
    query.addQueryItem(u"option"_qs, u"OnlyDecimal"_qs);
    QTest::newRow("onlydecimyl-invalid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2K"_qs);
    query.addQueryItem(u"min"_qs, u"1000"_qs);
    QTest::newRow("min-valid") << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2K"_qs);
    query.addQueryItem(u"min"_qs, u"2048"_qs);
    QTest::newRow("min-invalid") << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2KiB"_qs);
    query.addQueryItem(u"max"_qs, u"2048"_qs);
    QTest::newRow("max-valid") << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2KiB"_qs);
    query.addQueryItem(u"max"_qs, u"2047"_qs);
    QTest::newRow("max-invalid") << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2KiB"_qs);
    query.addQueryItem(u"min"_qs, u"2048"_qs);
    query.addQueryItem(u"max"_qs, u"2048"_qs);
    QTest::newRow("min-max-valid")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"0.5KiB"_qs);
    query.addQueryItem(u"min"_qs, u"1024"_qs);
    query.addQueryItem(u"max"_qs, u"2048"_qs);
    QTest::newRow("min-max-invalid-1")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"3.5KiB"_qs);
    query.addQueryItem(u"min"_qs, u"1024"_qs);
    query.addQueryItem(u"max"_qs, u"2048"_qs);
    QTest::newRow("min-max-invalid-2")
        << u"/fileSize"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    // **** Start testing ValidatorFileSize with return values

    const QMap<QString, QString> fileSizes({{u"1"_qs, u"1"_qs},
                                            {u"1B"_qs, u"1"_qs},
                                            {u"1K"_qs, u"1000"_qs},
                                            {u"1KiB"_qs, u"1024"_qs},
                                            {u"3.45K"_qs, u"3450"_qs},
                                            {u"3.45KiB"_qs, u"3533"_qs},
                                            {u"3456MB"_qs, u"3456000000"_qs},
                                            {u"3456MiB"_qs, u"3623878656"_qs},
                                            {u"4.321GB"_qs, u"4321000000"_qs},
                                            {u"4.321GiB"_qs, u"4639638422"_qs},
                                            {u"45.7890TB"_qs, u"45789000000000"_qs},
                                            {u"45.7890TiB"_qs, u"50345537924235"_qs},
                                            {u"123.456789PB"_qs, u"123456789000000000"_qs},
                                            {u"123.456789PiB"_qs, u"138999987234189488"_qs},
                                            {u"1.23EB"_qs, u"1230000000000000000"_qs},
                                            {u"1.23EiB"_qs, u"1418093450666421760"_qs},
                                            {u"2ZB"_qs, u"2000000000000000000000.00"_qs},
                                            {u"2ZiB"_qs, u"2361183241434822606848.00"_qs}});

    count            = 0;
    auto fileSizesIt = fileSizes.constBegin();
    while (fileSizesIt != fileSizes.constEnd()) {
        query.clear();
        query.addQueryItem(u"field"_qs, fileSizesIt.key());
        QTest::newRow(u"return-value-%1"_qs.arg(count).toUtf8().constData())
            << u"/fileSizeValue"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
            << fileSizesIt.value().toUtf8();
        ++fileSizesIt;
        count++;
    }
}

void TestValidator::testValidatorFilled_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorFilled *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"toll"_qs);
    QTest::newRow("valid") << u"/filled"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    QTest::newRow("missing") << u"/filled"_qs << QByteArray() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("invalid") << u"/filled"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;
}

void TestValidator::testValidatorIn_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorIn *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"zwei"_qs);
    QTest::newRow("valid") << u"/in"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"vier"_qs);
    QTest::newRow("invalid") << u"/in"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("empty") << u"/in"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    QTest::newRow("missing") << u"/in"_qs << QByteArray() << valid;
}

void TestValidator::testValidatorInteger_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorInteger *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"2345"_qs);
    QTest::newRow("valid01") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"-2345"_qs);
    QTest::newRow("valid02") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, QString::number(std::numeric_limits<int>::max()));
    QTest::newRow("valid03") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"-23a45 f"_qs);
    QTest::newRow("invalid01") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"a-23f45"_qs);
    QTest::newRow("invalid02") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, QString::number(std::numeric_limits<qlonglong>::max()));
    QTest::newRow("invalid03") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("empty") << u"/integer"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    QTest::newRow("missing") << u"/integer"_qs << QByteArray() << valid;
}

void TestValidator::testValidatorIp_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorIp *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    QTest::newRow("v4-valid") << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                              << valid;

    const QList<QString> invalidIpv4({u"192.0.s.34"_qs,
                                      u"192.0.43."_qs,
                                      u"192.0.43"_qs,
                                      u"300.167.168.5"_qs,
                                      u"192.168.178.-5"_qs});
    int count = 0;
    for (const QString &ipv4 : invalidIpv4) {
        query.clear();
        query.addQueryItem(u"field"_qs, ipv4);
        QTest::newRow(u"v4-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    const QList<QString> validIpv6({u"::"_qs,
                                    u"::123"_qs,
                                    u"::123:456"_qs,
                                    u"::123:456:789:abc:def:6666"_qs,
                                    u"::123:456:789:abc:def:6666:7"_qs,
                                    u"123::456"_qs,
                                    u"123::456:789"_qs,
                                    u"123::456:789:abc"_qs,
                                    u"123::456:789:abc:def"_qs,
                                    u"123::456:789:abc:def:6"_qs,
                                    u"123:456::789:abc:def:6666"_qs,
                                    u"2001:0db8:85a3:08d3:1319:8a2e:0370:7344"_qs,
                                    u"2001:0db8:0000:08d3:0000:8a2e:0070:7344"_qs,
                                    u"2001:db8:0:8d3:0:8a2e:70:7344"_qs,
                                    u"2001:0db8:0:0:0:0:1428:57ab"_qs,
                                    u"2001:db8::1428:57ab"_qs,
                                    u"2001:0db8:0:0:8d3:0:0:0"_qs,
                                    u"2001:db8:0:0:8d3::"_qs,
                                    u"2001:db8::8d3:0:0:0"_qs,
                                    u"::ffff:127.0.0.1"_qs,
                                    u"::ffff:7f00:1"_qs});

    count = 0;
    for (const QString &ipv6 : validIpv6) {
        query.clear();
        query.addQueryItem(u"field"_qs, ipv6);
        QTest::newRow(u"v6-valid0%1"_qs.arg(count).toUtf8().constData())
            << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
        count++;
    }

    const QList<QString> invalidIpv6({u"2001:db8::8d3::"_qs,
                                      u"2001:0db8:85a3:08d3:1319:8a2e:0370:7344:1234"_qs,
                                      u":::08d3:1319:8a2e:0370:7344"_qs,
                                      u"2001:0db8:85a3:08d3:1319:8a2k:0370:7344"_qs,
                                      u"127.0.0.1:1319:8a2k:0370:7344"_qs,
                                      u"2001::0db8:85a3:08d3::1319:8a2k:0370:7344"_qs,
                                      u"2001::0DB8:85A3:08D3::1319:8a2k:0370:7344"_qs,
                                      u":::"_qs});
    count = 0;
    for (const QString &ipv6 : invalidIpv6) {
        query.clear();
        query.addQueryItem(u"field"_qs, ipv6);
        QTest::newRow(u"v6-invalid0%1"_qs.arg(count).toUtf8().constData())
            << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"IPv4Only"_qs);
    QTest::newRow("ipv4only-valid")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"IPv4Only"_qs);
    QTest::newRow("ipv4only-invalid")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"IPv6Only"_qs);
    QTest::newRow("ipv6only-valid")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"IPv6Only"_qs);
    QTest::newRow("ipv6only-invalid")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoPrivateRange"_qs);
    QTest::newRow("noprivate-valid00")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoPrivateRange"_qs);
    QTest::newRow("noprivate-valid01")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QList<QString> invalidIpNoPrivate({u"10.1.2.3"_qs,
                                             u"172.21.158.56"_qs,
                                             u"192.168.178.100"_qs,
                                             u"169.254.254.254"_qs,
                                             u"fe80::5652:697b:2531:a7ed"_qs,
                                             u"fd00:26:5bf0:abd2:15ff:1adb:e8c4:8453"_qs});
    count = 0;
    for (const QString &ip : invalidIpNoPrivate) {
        query.clear();
        query.addQueryItem(u"field"_qs, ip);
        query.addQueryItem(u"constraints"_qs, u"NoPrivateRange"_qs);
        QTest::newRow(qUtf8Printable(u"noprivate-invalid0%1"_qs.arg(count)))
            << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoReservedRange"_qs);
    QTest::newRow("noreserved-valid00")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoReservedRange"_qs);
    QTest::newRow("noreserved-valid01")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QList<QString> invalidIpNoReserved({u"0.1.2.3"_qs,
                                              u"127.0.0.1"_qs,
                                              u"100.88.5.89"_qs,
                                              u"192.0.0.56"_qs,
                                              u"192.0.2.165"_qs,
                                              u"192.88.99.67"_qs,
                                              u"198.18.5.85"_qs,
                                              u"198.51.100.33"_qs,
                                              u"203.0.113.97"_qs,
                                              u"250.240.230.230"_qs,
                                              u"255.255.255.255"_qs,
                                              u"::"_qs,
                                              u"::1"_qs,
                                              u"0000:0000:0000:0000:0000:ffff:1234:abcd"_qs,
                                              u"0100:0000:0000:0000:1234:5678:9abc:def0"_qs,
                                              u"64:ff9b::95.4.66.32"_qs,
                                              u"2001:0000:1234:5678:90ab:cdef:1234:5678"_qs,
                                              u"2001:0010:0000:9876:abcd:5432:0000:a5b4"_qs,
                                              u"2001:0020:0000:9876:abcd:5432:0000:a5b4"_qs,
                                              u"2001:0db8:5b8e:6b5c:cdab:8546:abde:abdf"_qs,
                                              u"2002:fd4b:5b8e:6b5c:cdab:8546:abde:abdf"_qs});
    count = 0;
    for (const QString &ip : invalidIpNoReserved) {
        query.clear();
        query.addQueryItem(u"field"_qs, ip);
        query.addQueryItem(u"constraints"_qs, u"NoReservedRange"_qs);
        QTest::newRow(qUtf8Printable(u"noreserved-invalid0%1"_qs.arg(count)))
            << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(u"field"_qs, u"192.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoMultiCast"_qs);
    QTest::newRow("nomulticast-valid00")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoMultiCast"_qs);
    QTest::newRow("nomulticast-valid01")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"229.0.43.8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoMultiCast"_qs);
    QTest::newRow("nomulticast-invalid00")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"ff02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoMultiCast"_qs);
    QTest::newRow("nomulticast-invalid01")
        << u"/ip"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorJson_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorJson *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs,
                       u"{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":"
                       "2e%2B6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\","
                       "\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\","
                       "\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"_qs);
    QTest::newRow("valid") << u"/json"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(
        u"field"_qs,
        u"{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":2e "
        "6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\",\"Vorname\":\"Max\","
        "\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\",\"Lesen\"],\"Alter\":42,"
        "\"Kinder\":[],\"Partner\":null}}"_qs);
    QTest::newRow("invalid") << u"/json"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs,
                       u"{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":"
                       "2e%2B6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\","
                       "\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\","
                       "\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"_qs);
    QTest::newRow("valid-object") << u"/jsonObject"_qs
                                  << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"[\"value1\", \"value2\", \"value3\"]"_qs);
    QTest::newRow("invalid-object")
        << u"/jsonObject"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"[\"value1\", \"value2\", \"value3\"]"_qs);
    QTest::newRow("valid-array") << u"/jsonArray"_qs
                                 << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs,
                       u"{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":"
                       "2e%2B6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\","
                       "\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\","
                       "\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"_qs);
    QTest::newRow("invalid-array")
        << u"/jsonArray"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorMax_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorMax *****

    QUrlQuery query;
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("sint-empty") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"-5"_qs);
    QTest::newRow("sint-valid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"15"_qs);
    QTest::newRow("sint-invalid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"5"_qs);
    QTest::newRow("uint-valid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"15"_qs);
    QTest::newRow("uint-invalid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"-5.234652435"_qs);
    QTest::newRow("uint-valid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"15.912037"_qs);
    QTest::newRow("uint-invalid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghij"_qs);
    QTest::newRow("uint-valid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghijlmnop"_qs);
    QTest::newRow("uint-invalid") << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"strsdf"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghijlmnop"_qs);
    QTest::newRow("validationdataerror")
        << u"/max"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;
}

void TestValidator::testValidatorMin_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorMin *****

    QUrlQuery query;
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("sint-empty") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"15"_qs);
    QTest::newRow("sint-valid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"-5"_qs);
    QTest::newRow("sint-invalid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"15"_qs);
    QTest::newRow("uint-valid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"5"_qs);
    QTest::newRow("uint-invalid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"15.912037"_qs);
    QTest::newRow("float-valid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"-5.234652435"_qs);
    QTest::newRow("float-invalid")
        << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghijklmnop"_qs);
    QTest::newRow("string-valid") << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdef"_qs);
    QTest::newRow("string-invalid")
        << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"strsdf"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghijlmnop"_qs);
    QTest::newRow("validationdataerror")
        << u"/min"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;
}

void TestValidator::testValidatorNotIn_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorNotIn *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"fünf"_qs);
    QTest::newRow("valid") << u"/notIn"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"vier"_qs);
    QTest::newRow("invalid") << u"/notIn"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;
}

void TestValidator::testValidatorNumeric_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorNumeric *****

    const QList<QString> validNumerics({u"23"_qs,
                                        u"-3465"_qs,
                                        u"23.45"_qs,
                                        u"-3456.32453245"_qs,
                                        u"23.345345e15"_qs,
                                        u"-1.23e4"_qs});

    int count = 0;
    QUrlQuery query;
    for (const QString &num : validNumerics) {
        query.clear();
        query.addQueryItem(u"field"_qs, num);
        QTest::newRow(qUtf8Printable(u"valid0%1"_qs.arg(count)))
            << u"/numeric"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
        count++;
    }

    const QList<QString> invalidNumerics({u"2s3"_qs,
                                          u"-a3465"_qs,
                                          u"23:45"_qs,
                                          u"-3456:32453245"_qs,
                                          u"23.345345c15"_qs,
                                          u"-1.23D4"_qs});

    count = 0;
    for (const QString &num : invalidNumerics) {
        query.clear();
        query.addQueryItem(u"field"_qs, num);
        QTest::newRow(qUtf8Printable(u"invalid0%1"_qs.arg(count)))
            << u"/numeric"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("empty") << u"/numeric"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;
}

void TestValidator::testValidatorPresent_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorPresent *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("valid") << u"/present"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdfasdf"_qs);
    QTest::newRow("invalid") << u"/present"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;
}

#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
void TestValidator::testValidatorPwQuality_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorPwQuality

    const QList<QString> invalidPws({
        u"ovkaCPa"_qs,  // too short, lower than 8
        u"password"_qs, // dictionary
        u"aceg1234"_qs  // score too low
    });
    int count = 0;
    QUrlQuery query;
    for (const QString &pw : invalidPws) {
        query.clear();
        query.addQueryItem(u"field"_qs, pw);
        QTest::newRow(qUtf8Printable(u"invalid0%1"_qs.arg(count)))
            << u"/pwQuality"_qs << query.toString(QUrl::FullyEncoded).toUtf8() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(u"field"_qs, u"niK3sd2eHAm@M0vZ!8sd$uJv?4AYlDaP6"_qs);
    QTest::newRow("valid") << u"/pwQuality"_qs << query.toString(QUrl::FullyEncoded).toUtf8()
                           << valid;
}
#endif

void TestValidator::testValidatorRegex_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRegex *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"08/12/1985"_qs);
    QTest::newRow("valid") << u"/regex"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"8/2/85"_qs);
    QTest::newRow("invalid") << u"/regex"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("empty") << u"/regex"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;
}

void TestValidator::testValidatorRequired_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequired *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"08/12/1985"_qs);
    QTest::newRow("valid") << u"/required"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("empty") << u"/required"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                           << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"08/12/1985"_qs);
    QTest::newRow("missing") << u"/required"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;
}

void TestValidator::testValidatorRequiredIf_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredIf *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"asdfasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"eins"_qs);
    QTest::newRow("valid00") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"adfasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("valid01") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("valid02") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("valid03") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field3"_qs, u"eins"_qs);
    QTest::newRow("valid04") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"eins"_qs);
    QTest::newRow("invalid00") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"eins"_qs);
    QTest::newRow("invalid01") << u"/requiredIf"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << invalid;
}

void TestValidator::testValidatorRequiredIfStash_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredIfStash *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"adsf"_qs);
    QTest::newRow("valid01") << u"/requiredIfStashMatch"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"adsf"_qs);
    QTest::newRow("valid02") << u"/requiredIfStashNotMatch"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"adsf"_qs);
    QTest::newRow("valid03") << u"/requiredIfStashMatchStashKey"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"adsf"_qs);
    QTest::newRow("valid04") << u"/requiredIfStashNotMatchStashKey"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"adsf"_qs);
    QTest::newRow("invalid01") << u"/requiredIfStashNotMatch"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"adsf"_qs);
    QTest::newRow("invalid02") << u"/requiredIfStashMatch"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"adsf"_qs);
    QTest::newRow("invalid03") << u"/requiredIfStashNotMatchStashKey"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"adsf"_qs);
    QTest::newRow("invalid04") << u"/requiredIfStashMatchStashKey"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredUnless_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredUnless *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"asdfasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"eins"_qs);
    QTest::newRow("valid00") << u"/requiredUnless"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"asdfasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("valid01") << u"/requiredUnless"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"eins"_qs);
    QTest::newRow("valid02") << u"/requiredUnless"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"zwei"_qs);
    QTest::newRow("valid03") << u"/requiredUnless"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("invalid00") << u"/requiredUnless"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"vier"_qs);
    QTest::newRow("invalid01") << u"/requiredUnless"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredUnlessStash_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredUnlessStash *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"asdf"_qs);
    QTest::newRow("valid00") << u"/requiredUnlessStashMatch"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdf"_qs);
    QTest::newRow("valid01") << u"/requiredUnlessStashMatch"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"asdf"_qs);
    QTest::newRow("valid02") << u"/requiredUnlessStashNotMatch"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"asdf"_qs);
    QTest::newRow("valid03") << u"/requiredUnlessStashMatchStashKey"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdf"_qs);
    QTest::newRow("valid04") << u"/requiredUnlessStashMatchStashKey"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdf"_qs);
    QTest::newRow("invalid00") << u"/requiredUnlessStashNotMatch"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"%20"_qs);
    QTest::newRow("invalid01") << u"/requiredUnlessStashNotMatch"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdf"_qs);
    QTest::newRow("invalid03") << u"/requiredUnlessStashNotMatchStashKey"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredWith_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredWith *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid00") << u"/requiredWith"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field3"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid01") << u"/requiredWith"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field3"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid02") << u"/requiredWith"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"wlklasdf"_qs);
    QTest::newRow("invalid00") << u"/requiredWith"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"wlklasdf"_qs);
    QTest::newRow("invalid01") << u"/requiredWith"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredWithAll_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredWithAll *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field3"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("valid00") << u"/requiredWithAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("valid01") << u"/requiredWithAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("valid02") << u"/requiredWithAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("valid03") << u"/requiredWithAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field3"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("invalid00") << u"/requiredWithAll"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field3"_qs, u"asdfdasf"_qs);
    query.addQueryItem(u"field4"_qs, u"asdfdasf"_qs);
    QTest::newRow("invalid01") << u"/requiredWithAll"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredWithout_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredWithout *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid00") << u"/requiredWithout"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid01") << u"/requiredWithout"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("invalid00") << u"/requiredWithout"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field4"_qs, u"asdfasdf"_qs);
    QTest::newRow("invalid01") << u"/requiredWithout"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field2"_qs, u"asdfasdf"_qs);
    QTest::newRow("invalid02") << u"/requiredWithout"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorRequiredWithoutAll_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorRequiredWithoutAll *****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field2"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid00") << u"/requiredWithoutAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid01") << u"/requiredWithoutAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"field4"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid02") << u"/requiredWithoutAll"_qs
                             << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    QTest::newRow("invalid00") << u"/requiredWithoutAll"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field4"_qs, u"wlklasdf"_qs);
    QTest::newRow("invalid01") << u"/requiredWithoutAll"_qs
                               << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
}

void TestValidator::testValidatorSame_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorSame *****
    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"other"_qs, u"wlklasdf"_qs);
    QTest::newRow("valid") << u"/same"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"wlklasdf"_qs);
    query.addQueryItem(u"other"_qs, u"wlkla"_qs);
    QTest::newRow("invalid") << u"/same"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                             << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    query.addQueryItem(u"other"_qs, u"wlkla"_qs);
    QTest::newRow("empty") << u"/same"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
}

void TestValidator::testValidatorSize_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorSize *****

    QUrlQuery query;
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("sint-empty") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"10"_qs);
    QTest::newRow("sint-valid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"sint"_qs);
    query.addQueryItem(u"field"_qs, u"-5"_qs);
    QTest::newRow("sint-invalid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"10"_qs);
    QTest::newRow("uint-valid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"uint"_qs);
    query.addQueryItem(u"field"_qs, u"5"_qs);
    QTest::newRow("uint-invalid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"10.0"_qs);
    QTest::newRow("float-valid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"float"_qs);
    query.addQueryItem(u"field"_qs, u"-5.234652435"_qs);
    QTest::newRow("flost-invalid")
        << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghij"_qs);
    QTest::newRow("string-valid") << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                  << valid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"string"_qs);
    query.addQueryItem(u"field"_qs, u"abcdef"_qs);
    QTest::newRow("string-invalid")
        << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"type"_qs, u"strsdf"_qs);
    query.addQueryItem(u"field"_qs, u"abcdefghijlmnop"_qs);
    QTest::newRow("validationdataerror")
        << u"/size"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;
}

void TestValidator::testValidatorTime_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorTime *****

    int count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(u"valid0%1"_qs.arg(count)).toUtf8().constData())
            << u"/time?field="_qs + QTime::currentTime().toString(df) << QByteArray() << valid;
        count++;
    }

    QTest::newRow("invalid") << u"/time?field=123456789"_qs << QByteArray() << invalid;

    QTest::newRow("empty") << u"/time?field=%20"_qs << QByteArray() << valid;

    QTest::newRow("format-valid") << u"/timeFormat?field="_qs +
                                         QTime::currentTime().toString(u"m:hh"_qs)
                                  << QByteArray() << valid;

    QTest::newRow("format-invalid")
        << u"/timeFormat?field="_qs + QTime::currentTime().toString(u"m:AP"_qs) << QByteArray()
        << invalid;
}

void TestValidator::testValidatorUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    // **** Start testing ValidatorUrl*****

    QUrlQuery query;
    query.addQueryItem(u"field"_qs, u"http://www.example.org"_qs);
    QTest::newRow("url-valid00") << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"/home/user"_qs);
    QTest::newRow("url-valid01") << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"user"_qs);
    QTest::newRow("url-valid02") << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"file:///home/user/test.txt"_qs);
    QTest::newRow("url-valid03") << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                                 << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"%20"_qs);
    QTest::newRow("url-empty") << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1()
                               << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"http://www.example.org"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoRelative"_qs);
    QTest::newRow("url-norelative-valid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"/home/user"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoRelative"_qs);
    QTest::newRow("url-norelative-invalid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"http://www.example.org"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoLocalFile"_qs);
    QTest::newRow("url-nolocalfile-valid00")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"/home/user/test.txt"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoLocalFile"_qs);
    QTest::newRow("url-nolocalfile-valid01")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"file:///home/user/test.txt"_qs);
    query.addQueryItem(u"constraints"_qs, u"NoLocalFile"_qs);
    QTest::newRow("url-nolocalfile-invalid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"http://www.example.org"_qs);
    query.addQueryItem(u"schemes"_qs, u"HTTP,https"_qs);
    QTest::newRow("url-scheme-valid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"ftp://www.example.org"_qs);
    query.addQueryItem(u"schemes"_qs, u"HTTP,https"_qs);
    QTest::newRow("url-scheme-invalid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(u"field"_qs, u"http://www.example.org"_qs);
    query.addQueryItem(u"constraints"_qs, u"WebsiteOnly"_qs);
    QTest::newRow("url-websiteonly-valid")
        << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QStringList invalidWebsiteUrls(
        {u"ftp://www.example.org"_qs, u"file:///home/user/test.txt"_qs, u"/home/user"_qs});
    int count = 0;
    for (const QString &invalidWebsite : invalidWebsiteUrls) {
        query.clear();
        query.addQueryItem(u"field"_qs, invalidWebsite);
        query.addQueryItem(u"constraints"_qs, u"WebsiteOnly"_qs);
        QTest::newRow(qUtf8Printable(u"url-websiteonly-invalid0%1"_qs.arg(count)))
            << u"/url"_qs << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }
}

QTEST_MAIN(TestValidator)

#include "testvalidator.moc"

#endif // VALIDATORTEST_H
