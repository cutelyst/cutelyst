#ifndef VALIDATORTEST_H
#define VALIDATORTEST_H

#include <QTest>
#include <QObject>
#include <QHostInfo>
#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QStringList>
#include <QRegularExpression>
#include <QLocale>
#include <QTimeZone>
#include <limits>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>
#include <Cutelyst/Plugins/Utils/Validator/Validator>
#include <Cutelyst/Plugins/Utils/Validator/Validators>
#include <Cutelyst/Plugins/Utils/Validator/validatorresult.h>

using namespace Cutelyst;

class TestValidator : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestValidator(QObject *parent = nullptr) :
        CoverageObject(parent)
    {
    }

    ~TestValidator()
    {
    }

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


class ValidatorTest : public Controller
{
    Q_OBJECT
public:
    explicit ValidatorTest(QObject *parent) : Controller(parent) {}

    // ***** Endpoint for checking Validator::BodyParamsOnly
    C_ATTR(bodyParamsOnly, :Local :AutoArgs)
    void bodyParamsOnly(Context *c) {
        Validator v({new ValidatorRequired(QStringLiteral("req_field"), m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::BodyParamsOnly));
    }

    // ***** Endpoint for checking Validator::QueryParamsOnly
    C_ATTR(queryParamsOnly, :Local :AutoArgs)
    void queryParamsOnly(Context *c) {
        Validator v({new ValidatorRequired(QStringLiteral("req_field"), m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::QueryParamsOnly));
    }

    // ***** Endpoint for ValidatorAccepted ******
    C_ATTR(accepted, :Local :AutoArgs)
    void accepted(Context *c) {
        Validator v({new ValidatorAccepted(QStringLiteral("accepted_field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QDate ******
    C_ATTR(afterDate, :Local :AutoArgs)
    void afterDate(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QDate::currentDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QTime ******
    C_ATTR(afterTime, :Local :AutoArgs)
    void afterTime(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QTime(12, 0), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with QDateTime ******
    C_ATTR(afterDateTime, :Local :AutoArgs)
    void afterDateTime(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QDateTime::currentDateTime(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with custom format ******
    C_ATTR(afterFormat, :Local :AutoArgs)
    void afterFormat(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QDateTime::currentDateTime(), QString(), "yyyy d MM HH:mm", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data ******
    C_ATTR(afterInvalidValidationData, :Local :AutoArgs)
    void afterInvalidValidationData(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data 2 ******
    C_ATTR(afterInvalidValidationData2, :Local :AutoArgs)
    void afterInvalidValidationData2(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QStringLiteral("schiet"), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with time zone *****
    C_ATTR(afterValidWithTimeZone, :Local :AutoArgs)
    void afterValidWithTimeZone(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"), QDateTime(QDate(2018, 1, 15), QTime(12,0), QTimeZone(QByteArrayLiteral("Indian/Christmas"))), QStringLiteral("Europe/Berlin"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAfter with time zone in iput field *****
    C_ATTR(afterValidWithTimeZoneField, :Local :AutoArgs)
    void afterValidWithTimeZoneField(Context *c) {
        Validator v({new ValidatorAfter(QStringLiteral("after_field"),
                     QDateTime(QDate(2018,1,15), QTime(12,0), QTimeZone(QByteArrayLiteral("Indian/Christmas"))),
                     QStringLiteral("tz_field"),
                     nullptr,
                     m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlpha ******
    C_ATTR(alpha, :Local :AutoArgs)
    void alpha(Context *c) {
        Validator v({new ValidatorAlpha(QStringLiteral("alpha_field"), false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // **** Endpoint for ValidatorAlpha only ASCII *****
    C_ATTR(alphaAscii, :Local :AutoArgs)
    void alphaAscii(Context *c) {
        Validator v({new ValidatorAlpha(QStringLiteral("alpha_field"), true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaDash ******
    C_ATTR(alphaDash, :Local :AutoArgs)
    void alphaDash(Context *c) {
        Validator v({new ValidatorAlphaDash(QStringLiteral("alphadash_field"), false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaDash only ASCII ******
    C_ATTR(alphaDashAscii, :Local :AutoArgs)
    void alphaDashAscii(Context *c) {
        Validator v({new ValidatorAlphaDash(QStringLiteral("alphadash_field"), true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaNum ******
    C_ATTR(alphaNum, :Local :AutoArgs)
    void alphaNum(Context *c) {
        Validator v({new ValidatorAlphaNum(QStringLiteral("alphanum_field"), false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorAlphaNum only ASCII ******
    C_ATTR(alphaNumAscii, :Local :AutoArgs)
    void alphaNumAscii(Context *c) {
        Validator v({new ValidatorAlphaNum(QStringLiteral("alphanum_field"), true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QDate ******
    C_ATTR(beforeDate, :Local :AutoArgs)
    void beforeDate(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QDate::currentDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QTime ******
    C_ATTR(beforeTime, :Local :AutoArgs)
    void beforeTime(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QTime(12, 0), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with QDateTime ******
    C_ATTR(beforeDateTime, :Local :AutoArgs)
    void beforeDateTime(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QDateTime::currentDateTime(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with custom format ******
    C_ATTR(beforeFormat, :Local :AutoArgs)
    void beforeFormat(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QDateTime::currentDateTime(), QString(), "yyyy d MM HH:mm", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data ******
    C_ATTR(beforeInvalidValidationData, :Local :AutoArgs)
    void beforeInvalidValidationData(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QDate(), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data 2 ******
    C_ATTR(beforeInvalidValidationData2, :Local :AutoArgs)
    void beforeInvalidValidationData2(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("before_field"), QStringLiteral("schiet"), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with time zone *****
    C_ATTR(beforeValidWithTimeZone, :Local :AutoArgs)
    void beforeValidWithTimeZone(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("after_field"), QDateTime(QDate(2018, 1, 15), QTime(12,0), QTimeZone(QByteArrayLiteral("America/Tijuana"))), QStringLiteral("Europe/Berlin"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBefore with time zone in iput field *****
    C_ATTR(beforeValidWithTimeZoneField, :Local :AutoArgs)
    void beforeValidWithTimeZoneField(Context *c) {
        Validator v({new ValidatorBefore(QStringLiteral("after_field"),
                     QDateTime(QDate(2018,1,15), QTime(12,0), QTimeZone(QByteArrayLiteral("America/Tijuana"))),
                     QStringLiteral("tz_field"),
                     nullptr,
                     m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with int ******
    C_ATTR(betweenInt, :Local :AutoArgs)
    void betweenInt(Context *c) {
        Validator v({new ValidatorBetween(QStringLiteral("between_field"), QMetaType::Int, -10, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with uint ******
    C_ATTR(betweenUint, :Local :AutoArgs)
    void betweenUint(Context *c) {
        Validator v({new ValidatorBetween(QStringLiteral("between_field"), QMetaType::UInt, 10, 20, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with float ******
    C_ATTR(betweenFloat, :Local :AutoArgs)
    void betweenFloat(Context *c) {
        Validator v({new ValidatorBetween(QStringLiteral("between_field"), QMetaType::Float, -10.0, 10.0, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBetween with string ******
    C_ATTR(betweenString, :Local :AutoArgs)
    void betweenString(Context *c) {
        Validator v({new ValidatorBetween(QStringLiteral("between_field"), QMetaType::QString, 5, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorBoolean ******
    C_ATTR(boolean, :Local :AutoArgs)
    void boolean(Context *c) {
        Validator v({new ValidatorBoolean(QStringLiteral("boolean_field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorCharNotAllowed ******
    C_ATTR(charNotAllowed, :Local :AutoArgs)
    void charNotAllowed(Context *c) {
        Validator v({new ValidatorCharNotAllowed(QStringLiteral("char_not_allowed_field"), QStringLiteral("#%*."), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorConfirmed ******
    C_ATTR(confirmed, :Local :AutoArgs)
    void confirmed(Context *c) {
        Validator v({new ValidatorConfirmed(QStringLiteral("pass"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDate with standard formats ******
    C_ATTR(date, :Local :AutoArgs)
    void date(Context *c) {
        Validator v({new ValidatorDate(QStringLiteral("field"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDate with custom format ******
    C_ATTR(dateFormat, :Local :AutoArgs)
    void dateFormat(Context *c) {
        Validator v({new ValidatorDate(QStringLiteral("field"), "yyyy d MM", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDateTime with standard formats ******
    C_ATTR(dateTime, :Local :AutoArgs)
    void dateTime(Context *c) {
        Validator v({new ValidatorDateTime(QStringLiteral("field"), QString(), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDateTime with custom format ******
    C_ATTR(dateTimeFormat, :Local :AutoArgs)
    void dateTimeFormat(Context *c) {
        Validator v({new ValidatorDateTime(QStringLiteral("field"), QString(), "yyyy d MM mm:HH", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDifferent ******
    C_ATTR(different, :Local :AutoArgs)
    void different(Context *c) {
        Validator v({new ValidatorDifferent(QStringLiteral("field"), QStringLiteral("other"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigits without exact length ******
    C_ATTR(digits, :Local :AutoArgs)
    void digits(Context *c) {
        Validator v({new ValidatorDigits(QStringLiteral("field"), -1, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigits with exact length ******
    C_ATTR(digitsLength, :Local :AutoArgs)
    void digitsLength(Context *c) {
        Validator v({new ValidatorDigits(QStringLiteral("field"), 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDigitsBetween ******
    C_ATTR(digitsBetween, :Local :AutoArgs)
    void digitsBetween(Context *c) {
        Validator v({new ValidatorDigitsBetween(QStringLiteral("field"), 5, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDomain without DNS check *****
    C_ATTR(domain, :Local :AutoArgs)
    void domain(Context *c) {
        Validator v({new ValidatorDomain(QStringLiteral("field"), false, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorDomain with DNS check *****
    C_ATTR(domainDns, :Local :AutoArgs)
    void domainDns(Context *c) {
        Validator v({new ValidatorDomain(QStringLiteral("field"), true, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorEmail valid ****
    C_ATTR(emailValid, :Local :AutoArgs)
    void emailValid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Valid, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail valid emails completely valid *****
    C_ATTR(emailDnsWarnValid, :Local :AutoArgs)
    void emailDnsWarnValid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Valid, ValidatorEmail::CheckDNS, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5321 conformant emails valid *****
    C_ATTR(emailRfc5321Valid, :Local :AutoArgs)
    void emailRfc5321Valid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::RFC5321, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5321 conformant emails invalid *****
    C_ATTR(emailRfc5321Invalid, :Local :AutoArgs)
    void emailRfc5321Invalid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::DNSWarn, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails valid *****
    C_ATTR(emailCfwsValid, :Local :AutoArgs)
    void emailCfwsValid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::CFWS, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails invalid *****
    C_ATTR(emailCfwsInvalid, :Local :AutoArgs)
    void emailCfwsInvalid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::RFC5321, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail Deprecated emails valid *****
    C_ATTR(emailDeprecatedValid, :Local :AutoArgs)
    void emailDeprecatedValid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Deprecated, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail CFWS conformant emails invalid *****
    C_ATTR(emailDeprecatedInvalid, :Local :AutoArgs)
    void emailDeprecatedInvalid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::CFWS, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5322 emails valid *****
    C_ATTR(emailRfc5322Valid, :Local :AutoArgs)
    void emailRfc5322Valid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::RFC5322, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail RFC5322 conformant emails invalid *****
    C_ATTR(emailRfc5322Invalid, :Local :AutoArgs)
    void emailRfc5322Invalid(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Deprecated, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with errors *****
    C_ATTR(emailErrors, :Local :AutoArgs)
    void emailErrors(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::RFC5322, ValidatorEmail::NoOption, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed IDNs
    C_ATTR(emailIdnAllowed, :Local :AutoArgs)
    void emailIdnAllowed(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Valid, ValidatorEmail::AllowIDN, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed UTF8 local part
    C_ATTR(emailUtf8Local, :Local :AutoArgs)
    void emailUtf8Local(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Valid, ValidatorEmail::UTF8Local, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorEmail with allowed UTF8 local part and IDN
    C_ATTR(emailUtf8, :Local :AutoArgs)
    void emailUtf8(Context *c) {
        Validator v({new ValidatorEmail(QStringLiteral("field"), ValidatorEmail::Valid, ValidatorEmail::AllowUTF8, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming|Validator::BodyParamsOnly));
    }

    // ***** Endpoint for ValidatorFileSize ******
    C_ATTR(fileSize, :Local :AutoArgs)
    void fileSize(Context *c) {
        ValidatorFileSize::Option option = ValidatorFileSize::NoOption;
        const QString opt = c->req()->bodyParameter(QStringLiteral("option"));
        if (opt == QLatin1String("OnlyBinary")) {
            option = ValidatorFileSize::OnlyBinary;
        } else if (opt == QLatin1String("OnlyDecimal")) {
            option = ValidatorFileSize::OnlyDecimal;
        } else if (opt == QLatin1String("ForceBinary")) {
            option = ValidatorFileSize::ForceBinary;
        } else if (opt == QLatin1String("ForceDecimal")) {
            option = ValidatorFileSize::ForceDecimal;
        }
        const double min = c->req()->bodyParameter(QStringLiteral("min"), QStringLiteral("-1.0")).toDouble();
        const double max = c->req()->bodyParameter(QStringLiteral("max"), QStringLiteral("-1.0")).toDouble();
        c->setLocale(QLocale(c->req()->bodyParameter(QStringLiteral("locale"), QStringLiteral("C"))));
        Validator v({new ValidatorFileSize(QStringLiteral("field"), option, min, max, m_validatorMessages)});
        checkResponse(c, v.validate(c, Validator::NoTrimming));
    }

    // ***** Endpoint for ValidatorFileSize with return value check *****
    C_ATTR(fileSizeValue, :Local :AutoArgs)
    void fileSizeValue(Context *c) {
        c->setLocale(QLocale::c());
        Validator v({new ValidatorFileSize(QStringLiteral("field"))});
        const ValidatorResult r = v.validate(c);
        if (r) {
            QString sizeString;
            const QVariant rv = r.value(QStringLiteral("field"));
            if (rv.type() == QVariant::Double) {
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
    void filled(Context *c) {
        Validator v({new ValidatorFilled(QStringLiteral("field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorIn ******
    C_ATTR(in, :Local :AutoArgs)
    void in(Context *c) {
        Validator v({new ValidatorIn(QStringLiteral("field"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei"), QStringLiteral("drei")}), Qt::CaseSensitive, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorInteger ******
    C_ATTR(integer, :Local :AutoArgs)
    void integer(Context *c) {
        Validator v({new ValidatorInteger(QStringLiteral("field"), QMetaType::Int, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorIp ******
    C_ATTR(ip, :Local :AutoArgs)
    void ip(Context *c) {
        ValidatorIp::Constraints constraints = ValidatorIp::NoConstraint;
        if (!c->request()->bodyParameter(QStringLiteral("constraints")).isEmpty()) {
            QStringList cons = c->request()->bodyParameter(QStringLiteral("constraints")).split(QStringLiteral(","));
            if (cons.contains(QStringLiteral("IPv4Only"))) {
                constraints |= ValidatorIp::IPv4Only;
            }

            if (cons.contains(QStringLiteral("IPv6Only"))) {
                constraints |= ValidatorIp::IPv6Only;
            }

            if (cons.contains(QStringLiteral("NoPrivateRange"))) {
                constraints |= ValidatorIp::NoPrivateRange;
            }

            if (cons.contains(QStringLiteral("NoReservedRange"))) {
                constraints |= ValidatorIp::NoReservedRange;
            }

            if (cons.contains(QStringLiteral("NoMultiCast"))) {
                constraints |= ValidatorIp::NoMultiCast;
            }
        }
        Validator v({new ValidatorIp(QStringLiteral("field"), constraints, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }


    // ***** Endpoint for ValidatorJson ******
    C_ATTR(json, :Local :AutoArgs)
    void json(Context *c) {
        Validator v({new ValidatorJson(QStringLiteral("field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }


    // ***** Endpoint for ValidatorMax ******
    C_ATTR(max, :Local :AutoArgs)
    void max(Context *c) {
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(QStringLiteral("type")).isEmpty()) {
            const QString t = c->request()->bodyParameter(QStringLiteral("type"));
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
        Validator v({new ValidatorMax(QStringLiteral("field"), type, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorMin ******
    C_ATTR(min, :Local :AutoArgs)
    void min(Context *c) {
        c->setStash(QStringLiteral("compval"), 10);
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(QStringLiteral("type")).isEmpty()) {
            const QString t = c->request()->bodyParameter(QStringLiteral("type"));
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
        Validator v({new ValidatorMin(QStringLiteral("field"), type, QStringLiteral("compval"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorNotIn ******
    C_ATTR(notIn, :Local :AutoArgs)
    void notIn(Context *c) {
        Validator v({new ValidatorNotIn(QStringLiteral("field"), QStringList({QStringLiteral("eins"),QStringLiteral("zwei"),QStringLiteral("drei"),QStringLiteral("vier")}), Qt::CaseSensitive, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }


    // ***** Endpoint for ValidatorNumeric ******
    C_ATTR(numeric, :Local :AutoArgs)
    void numeric(Context *c) {
        Validator v({new ValidatorNumeric(QStringLiteral("field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }


    // ***** Endpoint for ValidatorPresent ******
    C_ATTR(present, :Local :AutoArgs)
    void present(Context *c) {
        Validator v({new ValidatorPresent(QStringLiteral("field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorPwQuality *****
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    C_ATTR(pwQuality, :Local :AutoArgs)
    void pwQuality(Context *c) {
        static const QVariantMap options({
                                             {QStringLiteral("difok"), 1},
                                             {QStringLiteral("minlen"), 8},
                                             {QStringLiteral("dcredit"), 0},
                                             {QStringLiteral("ucredit"), 0},
                                             {QStringLiteral("ocredit"), 0},
                                             {QStringLiteral("lcredit"), 0},
                                             {QStringLiteral("minclass"), 0},
                                             {QStringLiteral("maxrepeat"), 0},
                                             {QStringLiteral("maxclassrepeat"), 0},
                                             {QStringLiteral("maxsequence"), 0},
                                             {QStringLiteral("gecoscheck"), 0},
                                             {QStringLiteral("dictcheck"), 1},
                                             {QStringLiteral("usercheck"), 0}
                                  });
        static Validator v({new ValidatorPwQuality(QStringLiteral("field"), 50, options, QString(), QString(), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }
#endif

    // ***** Endpoint for ValidatorRegularExpression ******
    C_ATTR(regex, :Local :AutoArgs)
    void regex(Context *c) {
        Validator v({new ValidatorRegularExpression(QStringLiteral("field"), QRegularExpression(QStringLiteral("^(\\d\\d)/(\\d\\d)/(\\d\\d\\d\\d)$")), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequired ******
    C_ATTR(required, :Local :AutoArgs)
    void required(Context *c) {
        Validator v({new ValidatorRequired(QStringLiteral("field"), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIf ******
    C_ATTR(requiredIf, :Local :AutoArgs)
    void requiredIf(Context *c) {
        Validator v({new ValidatorRequiredIf(QStringLiteral("field"), QStringLiteral("field2"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash match *****
    C_ATTR(requiredIfStashMatch, :Local :AutoArgs)
    void requiredIfStashMatch(Context *c) {
        c->setStash(QStringLiteral("stashkey"), QStringLiteral("eins"));
        Validator v({new ValidatorRequiredIfStash(QStringLiteral("field"), QStringLiteral("stashkey"), QVariantList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredIfStash with stash not match *****
    C_ATTR(requiredIfStashNotMatch, :Local :AutoArgs)
    void requiredIfStashNotMatch(Context *c) {
        c->setStash(QStringLiteral("stashkey"), QStringLiteral("drei"));
        Validator v({new ValidatorRequiredIfStash(QStringLiteral("field"), QStringLiteral("stashkey"), QVariantList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnless ******
    C_ATTR(requiredUnless, :Local :AutoArgs)
    void requiredUnless(Context *c) {
        Validator v({new ValidatorRequiredUnless(QStringLiteral("field"), QStringLiteral("field2"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash match *****
    C_ATTR(requiredUnlessStashMatch, :Local :AutoArgs)
    void requiredUnlessStashMatch(Context *c) {
        c->setStash(QStringLiteral("stashkey"), QStringLiteral("eins"));
        Validator v({new ValidatorRequiredUnlessStash(QStringLiteral("field"), QStringLiteral("stashkey"), QVariantList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredUnlessStash with stash not match *****
    C_ATTR(requiredUnlessStashNotMatch, :Local :AutoArgs)
    void requiredUnlessStashNotMatch(Context *c) {
        c->setStash(QStringLiteral("stashkey"), QStringLiteral("drei"));
        Validator v({new ValidatorRequiredUnlessStash(QStringLiteral("field"), QStringLiteral("stashkey"), QVariantList({QStringLiteral("eins"), QStringLiteral("zwei")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWith ******
    C_ATTR(requiredWith, :Local :AutoArgs)
    void requiredWith(Context *c) {
        Validator v({new ValidatorRequiredWith(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithAll ******
    C_ATTR(requiredWithAll, :Local :AutoArgs)
    void requiredWithAll(Context *c) {
        Validator v({new ValidatorRequiredWithAll(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3"), QStringLiteral("field4")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithout ******
    C_ATTR(requiredWithout, :Local :AutoArgs)
    void requiredWithout(Context *c) {
        Validator v({new ValidatorRequiredWithout(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorRequiredWithoutAll ******
    C_ATTR(requiredWithoutAll, :Local :AutoArgs)
    void requiredWithoutAll(Context *c) {
        Validator v({new ValidatorRequiredWithoutAll(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorSame ******
    C_ATTR(same, :Local :AutoArgs)
    void same(Context *c) {
        Validator v({new ValidatorSame(QStringLiteral("field"), QStringLiteral("other"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorSize ******
    C_ATTR(size, :Local :AutoArgs)
    void size(Context *c) {
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->bodyParameter(QStringLiteral("type")).isEmpty()) {
            const QString t = c->request()->bodyParameter(QStringLiteral("type"));
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
        Validator v({new ValidatorSize(QStringLiteral("field"), type, 10, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorTime
    C_ATTR(time, :Local :AutoArgs)
    void time(Context *c) {
        Validator v({new ValidatorTime(QStringLiteral("field"), nullptr, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorTime with custom format ******
    C_ATTR(timeFormat, :Local :AutoArgs)
    void timeFormat(Context *c) {
        Validator v({new ValidatorTime(QStringLiteral("field"), "m:hh", m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

    // ***** Endpoint for ValidatorUrl
    C_ATTR(url, :Local :AutoArgs)
    void url(Context *c) {
        ValidatorUrl::Constraints constraints = ValidatorUrl::NoConstraint;
        QStringList schemes;
        QString scheme = c->request()->bodyParameter(QStringLiteral("schemes"));
        if (!scheme.isEmpty()) {
            schemes = scheme.split(QStringLiteral(","));
        }

        if (!c->request()->bodyParameter(QStringLiteral("constraints")).isEmpty()) {
            const QStringList cons = c->request()->bodyParameter(QStringLiteral("constraints")).split(QStringLiteral(","));
            if (cons.contains(QStringLiteral("StrictParsing"))) {
                constraints |= ValidatorUrl::StrictParsing;
            }

            if (cons.contains(QStringLiteral("NoRelative"))) {
                constraints |= ValidatorUrl::NoRelative;
            }

            if (cons.contains(QStringLiteral("NoLocalFile"))) {
                constraints |= ValidatorUrl::NoLocalFile;
            }

            if (cons.contains(QStringLiteral("WebsiteOnly"))) {
                constraints |= ValidatorUrl::WebsiteOnly;
            }
        }

        Validator v({new ValidatorUrl(QStringLiteral("field"), constraints, schemes, m_validatorMessages)});
        checkResponse(c, v.validate(c));
    }

private:
    ValidatorMessages m_validatorMessages = ValidatorMessages(nullptr, "invalid", "parsingerror", "validationdataerror");

    void checkResponse(Context *c, const ValidatorResult &r) {
        if (r) {
            c->response()->setBody(QByteArrayLiteral("valid"));
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


TestEngine* TestValidator::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
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
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, output);

    const QUrl urlAux(QLatin1String("validator/test") + url);

    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"),
                                                       urlAux.path(),
                                                       urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                       headers,
                                                       &body);

    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}


void TestValidator::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    const QByteArray valid = QByteArrayLiteral("valid");
    const QByteArray invalid = QByteArrayLiteral("invalid");
    const QByteArray parsingError = QByteArrayLiteral("parsingerror");
    const QByteArray validationDataError = QByteArrayLiteral("validationdataerror");

    Headers headers;
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QUrlQuery query;

    const QList<Qt::DateFormat> dateFormats({Qt::ISODate, Qt::RFC2822Date, Qt::TextDate});

    // **** Start testing if the correct paramters are extracted according to the validator flags

    QTest::newRow("body-params-only-valid") << QStringLiteral("/bodyParamsOnly") << headers << QByteArrayLiteral("req_field=hallo") << valid;

    QTest::newRow("body-params-only-invalid") << QStringLiteral("/bodyParamsOnly?req_field=hallo") << headers << QByteArray() << invalid;

    QTest::newRow("query-params-only-valid") << QStringLiteral("/queryParamsOnly?req_field=hallo") << headers << QByteArray() << valid;

    QTest::newRow("query-params-only-invalid") << QStringLiteral("/queryParamsOnly") << headers << QByteArrayLiteral("req_field=hallo") << invalid;

    // **** Start testing ValidatorAccepted *****

    int count = 0;
    for (const QString &val : {QStringLiteral("yes"), QStringLiteral("on"), QStringLiteral("1"), QStringLiteral("true")}) {
        QTest::newRow(QString(QStringLiteral("accepted-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/accepted?accepted_field=") + val << headers << QByteArray() << valid;
        count++;
    }

    QTest::newRow("accepted-invalid") << QStringLiteral("/accepted?accepted_field=asdf") << headers << QByteArray() << invalid;

    QTest::newRow("accepted-empty") << QStringLiteral("/accepted?accepted_field=") << headers << QByteArray() << invalid;

    QTest::newRow("accepted-missing") << QStringLiteral("/accepted") << headers << QByteArray() << invalid;


    // **** Start testing ValidatorAfter *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("after-date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/afterDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << valid;


        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDate(1999, 9, 9).toString(df));
        QTest::newRow(QString(QStringLiteral("after-date-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/afterDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("after-date-parsingerror") << QStringLiteral("/afterDate?after_field=lökjasdfjh") << headers << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QTime(13, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("after-time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/afterTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << valid;


        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QTime(11, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("after-time-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/afterTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("after-time-parsingerror") << QStringLiteral("/afterTime?after_field=kjnagiuh") << headers << QByteArray() << parsingError;


    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QString queryPath = QStringLiteral("/afterDateTime?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime::currentDateTime().addDays(2).toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(QString(QStringLiteral("after-datetime-valid0%1").arg(count)).toUtf8().constData())
                << queryPath << headers << QByteArray() << valid;

        queryPath = QStringLiteral("/afterDateTime?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(QString(QStringLiteral("after-datetime-invalid0%1").arg(count)).toUtf8().constData())
                << queryPath << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("after-datetime-parsingerror")
            << QStringLiteral("/afterDateTime?after_field=aio,aü")
            << headers << QByteArray() << parsingError;

    QTest::newRow("after-invalidvalidationdata00")
            << QStringLiteral("/afterInvalidValidationData?after_field=") + QDate::currentDate().addDays(2).toString(Qt::ISODate)
            << headers << QByteArray() << validationDataError;

    QTest::newRow("after-invalidvalidationdata01")
            << QStringLiteral("/afterInvalidValidationData2?after_field=") + QDate::currentDate().addDays(2).toString(Qt::ISODate)
            << headers << QByteArray() << validationDataError;

    QTest::newRow("after-format-valid")
            << QStringLiteral("/afterFormat?after_field=") + QDateTime::currentDateTime().addDays(2).toString(QStringLiteral("yyyy d MM HH:mm"))
            << headers << QByteArray() << valid;

    QTest::newRow("after-format-invalid")
            << QStringLiteral("/afterFormat?after_field=") + QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(QStringLiteral("yyyy d MM HH:mm"))
            << headers << QByteArray() << invalid;

    QTest::newRow("after-format-parsingerror")
            << QStringLiteral("/afterFormat?after_field=23590uj09")
            << headers << QByteArray() << parsingError;

    {
        const QString queryPath = QStringLiteral("/afterValidWithTimeZone?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(2018, 1, 15), QTime(13,0)).toString(Qt::ISODate), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow("after-timezone-valid") << queryPath << headers << QByteArray() << valid;
    }

    {
        const QString queryPath = QStringLiteral("/afterValidWithTimeZoneField?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(2018, 1, 15), QTime(13,0)).toString(Qt::ISODate), QByteArray(), QByteArrayLiteral("+"))) + QLatin1String("&tz_field=Europe/Berlin");
    QTest::newRow("after-timezone-fromfield-valid") << queryPath << headers << QByteArray() << valid;
    }

    // **** Start testing ValidatorAlpha *****

    QTest::newRow("alpha-valid") << QStringLiteral("/alpha?alpha_field=adsfä") << headers << QByteArray() << valid;

    QTest::newRow("alpha-invalid") << QStringLiteral("/alpha?alpha_field=ad_sf 2ä!") << headers << QByteArray() << invalid;

    QTest::newRow("alpha-empty") << QStringLiteral("/alpha?alpha_field=") << headers << QByteArray() << valid;

    QTest::newRow("alpha-missing") << QStringLiteral("/alpha") << headers << QByteArray() << valid;

    QTest::newRow("alpha-ascii-valid") << QStringLiteral("/alphaAscii?alpha_field=basdf") << headers << QByteArray() << valid;

    QTest::newRow("alpha-ascii-invalid") << QStringLiteral("/alphaAscii?alpha_field=asdfös") << headers << QByteArray() << invalid;



    // **** Start testing ValidatorAlphaDash *****

    QTest::newRow("alphadash-valid") << QStringLiteral("/alphaDash?alphadash_field=ads2-fä_3") << headers << QByteArray() << valid;

    QTest::newRow("alphadash-invalid") << QStringLiteral("/alphaDash?alphadash_field=ad sf_2ä!") << headers << QByteArray() << invalid;

    QTest::newRow("alphadash-empty") << QStringLiteral("/alphaDash?alphadash_field=") << headers << QByteArray() << valid;

    QTest::newRow("alphadash-missing") << QStringLiteral("/alphaDash") << headers << QByteArray() << valid;

    QTest::newRow("alphadash-ascii-valid") << QStringLiteral("/alphaDashAscii?alphadash_field=s342-4d_3") << headers << QByteArray() << valid;

    QTest::newRow("alphadash-ascii-invalid") << QStringLiteral("/alphaDashAscii?alphadash_field=s342 4ä_3") << headers << QByteArray() << invalid;



    // **** Start testing ValidatorAlphaNum *****

    QTest::newRow("alphanum-valid") << QStringLiteral("/alphaNum?alphanum_field=ads2fä3") << headers << QByteArray() << valid;

    QTest::newRow("alphanum-invalid") << QStringLiteral("/alphaNum?alphanum_field=ad sf_2ä!") << headers << QByteArray() << invalid;

    QTest::newRow("alphanum-empty") << QStringLiteral("/alphaNum?alphanum_field=") << headers << QByteArray() << valid;

    QTest::newRow("alphanum-missing") << QStringLiteral("/alphaNum") << headers << QByteArray() << valid;

    QTest::newRow("alphanum-ascii-valid") << QStringLiteral("/alphaNumAscii?alphanum_field=ba34sdf") << headers << QByteArray() << valid;

    QTest::newRow("alphanum-ascii-invalid") << QStringLiteral("/alphaNumAscii?alphanum_field=as3dfös") << headers << QByteArray() << invalid;




    // **** Start testing ValidatorBefore *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDate(1999, 9, 9).toString(df));
        QTest::newRow(QString(QStringLiteral("before-date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/beforeDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << valid;


        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("before-date-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/beforeDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-date-parsingerror") << QStringLiteral("/beforeDate?before_field=lökjasdfjh") << headers << QByteArray() << parsingError;

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QTime(11, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("before-time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/beforeTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << valid;


        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QTime(13, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("before-time-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/beforeTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-time-parsingerror") << QStringLiteral("/beforeTime?before_field=kjnagiuh") << headers << QByteArray() << parsingError;


    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QString pathQuery = QStringLiteral("/beforeDateTime?before_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(QString(QStringLiteral("before-datetime-valid0%1").arg(count)).toUtf8().constData())
                << pathQuery << headers << QByteArray() << valid;

        pathQuery = QStringLiteral("/beforeDateTime?before_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime::currentDateTime().addDays(2).toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(QString(QStringLiteral("before-datetime-invalid0%1").arg(count)).toUtf8().constData())
                << pathQuery << headers << QByteArray() << invalid;

        count++;
    }

    QTest::newRow("before-datetime-parsingerror")
            << QStringLiteral("/beforeDateTime?before_field=aio,aü")
            << headers << QByteArray() << parsingError;


    QTest::newRow("before-invalidvalidationdata00")
            << QStringLiteral("/beforeInvalidValidationData?before_field=") + QDate(1999, 9, 9).toString(Qt::ISODate)
            << headers << QByteArray() << validationDataError;

    QTest::newRow("before-invalidvalidationdata01")
            << QStringLiteral("/beforeInvalidValidationData2?before_field=") + QDate(1999, 9, 9).toString(Qt::ISODate)
            << headers << QByteArray() << validationDataError;

    QTest::newRow("before-format-valid")
            << QStringLiteral("/beforeFormat?before_field=") + QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(QStringLiteral("yyyy d MM HH:mm"))
            << headers << QByteArray() << valid;

    QTest::newRow("before-format-invalid")
            << QStringLiteral("/beforeFormat?before_field=") + QDateTime::currentDateTime().addDays(2).toString(QStringLiteral("yyyy d MM HH:mm"))
            << headers << QByteArray() << invalid;

    QTest::newRow("before-format-parsingerror")
            << QStringLiteral("/beforeFormat?before_field=23590uj09")
            << headers << QByteArray() << parsingError;

    {
        const QString pathQuery = QStringLiteral("/beforeValidWithTimeZone?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(2018, 1, 15), QTime(11,0)).toString(Qt::ISODate), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow("before-timezone-valid") << pathQuery << headers << QByteArray() << valid;
    }

    {
        const QString pathQuery = QStringLiteral("/beforeValidWithTimeZoneField?after_field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime(QDate(2018, 1, 15), QTime(11,0)).toString(Qt::ISODate), QByteArray(), QByteArrayLiteral("+"))) + QLatin1String("&tz_field=Europe/Berlin");
        QTest::newRow("before-timezone-fromfield-valid") << pathQuery << headers << QByteArray() << valid;
    }




    // **** Start testing ValidatorBetween *****

    QTest::newRow("between-int-valid") << QStringLiteral("/betweenInt?between_field=0") << headers << QByteArray() << valid;

    QTest::newRow("between-int-invalid-lower") << QStringLiteral("/betweenInt?between_field=-15") << headers << QByteArray() << invalid;

    QTest::newRow("between-int-invalid-greater") << QStringLiteral("/betweenInt?between_field=15") << headers << QByteArray() << invalid;

    QTest::newRow("between-int-empty") << QStringLiteral("/betweenInt?between_field=") << headers << QByteArray() << valid;

    QTest::newRow("between-uint-valid") << QStringLiteral("/betweenUint?between_field=15") << headers << QByteArray() << valid;

    QTest::newRow("between-uint-invalid-lower") << QStringLiteral("/betweenUint?between_field=5") << headers << QByteArray() << invalid;

    QTest::newRow("between-uint-invalid-greater") << QStringLiteral("/betweenUint?between_field=25") << headers << QByteArray() << invalid;

    QTest::newRow("between-uint-empty") << QStringLiteral("/betweenUint?between_field=") << headers << QByteArray() << valid;

    QTest::newRow("between-float-valid") << QStringLiteral("/betweenFloat?between_field=0.0") << headers << QByteArray() << valid;

    QTest::newRow("between-float-invalid-lower") << QStringLiteral("/betweenFloat?between_field=-15.2") << headers << QByteArray() << invalid;

    QTest::newRow("between-float-invalid-greater") << QStringLiteral("/betweenFloat?between_field=15.2") << headers << QByteArray() << invalid;

    QTest::newRow("between-float-empty") << QStringLiteral("/betweenFloat?between_field=") << headers << QByteArray() << valid;

    QTest::newRow("between-string-valid") << QStringLiteral("/betweenString?between_field=abcdefg") << headers << QByteArray() << valid;

    QTest::newRow("between-string-invalid-lower") << QStringLiteral("/betweenString?between_field=abc") << headers << QByteArray() << invalid;

    QTest::newRow("between-string-invalid-greater") << QStringLiteral("/betweenString?between_field=abcdefghijklmn") << headers << QByteArray() << invalid;

    QTest::newRow("between-string-empty") << QStringLiteral("/betweenString?between_field=") << headers << QByteArray() << valid;



    // **** Start testing ValidatorBoolean *****

    for (const QString &bv : {QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("on"), QStringLiteral("off")}) {
        QTest::newRow(QString(QStringLiteral("boolean-valid-%1").arg(bv)).toUtf8().constData())
                << QStringLiteral("/boolean?boolean_field=") + bv << headers << QByteArray() << valid;
    }

    for (const QString &bv : {QStringLiteral("2"), QStringLiteral("-45"), QStringLiteral("wahr"), QStringLiteral("unwahr"), QStringLiteral("ja")}) {
        QTest::newRow(QString(QStringLiteral("boolean-invalid-%1").arg(bv)).toUtf8().constData())
                << QStringLiteral("/boolean?boolean_field=") + bv << headers << QByteArray() << invalid;
    }

    QTest::newRow("boolean-empty") << QStringLiteral("/boolean?boolean_field=") << headers << QByteArray() << valid;


    // **** Start testing ValidatorCharNotAllowed *****

    QTest::newRow("charnotallowed-empty") << QStringLiteral("/charNotAllowed?char_not_allowed_field=") << headers << QByteArray() << valid;

    QTest::newRow("charnotallowed-valid") << QStringLiteral("/charNotAllowed?char_not_allowed_field=holladiewaldfee") << headers << QByteArray() << valid;

    QTest::newRow("charnotallowed-invalid") << QStringLiteral("/charNotAllowed?char_not_allowed_field=holla.die.waldfee") << headers << QByteArray() << invalid;



    // **** Start testing ValidatorConfirmed *****

    QTest::newRow("confirmed-valid") << QStringLiteral("/confirmed?pass=abcdefg&pass_confirmation=abcdefg") << headers << QByteArray() << valid;

    QTest::newRow("confirmed-invalid") << QStringLiteral("/confirmed?pass=abcdefg&pass_confirmation=hijklmn") << headers << QByteArray() << invalid;

    QTest::newRow("confirmed-empty") << QStringLiteral("/confirmed?pass&pass_confirmation=abcdefg") << headers << QByteArray() << valid;

    QTest::newRow("confirmed-missing-confirmation") << QStringLiteral("/confirmed?pass=abcdefg") << headers << QByteArray() << invalid;



    // **** Start testing ValidatorDate *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(QStringLiteral("date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/date?field=") + QDate::currentDate().toString(df) << headers << QByteArray() << valid;
        count++;
    }

    QTest::newRow("date-invalid") << QStringLiteral("/date?field=123456789") << headers << QByteArray() << invalid;

    QTest::newRow("date-empty") << QStringLiteral("/date?field=") << headers << QByteArray() << valid;

    QTest::newRow("date-format-valid") << QStringLiteral("/dateFormat?field=") + QDate::currentDate().toString(QStringLiteral("yyyy d MM")) << headers << QByteArray() << valid;

    QTest::newRow("date-format-invalid") << QStringLiteral("/dateFormat?field=") + QDate::currentDate().toString(QStringLiteral("MM yyyy d")) << headers << QByteArray() << invalid;




    // **** Start testing ValidatorDateTime *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        const QString pathQuery = QStringLiteral("/dateTime?field=") + QString::fromLatin1(QUrl::toPercentEncoding(QDateTime::currentDateTime().toString(df), QByteArray(), QByteArrayLiteral("+")));
        QTest::newRow(QString(QStringLiteral("datetime-valid0%1").arg(count)).toUtf8().constData())
                << pathQuery << headers << QByteArray() << valid;
        count++;
    }

    QTest::newRow("datetime-invalid") << QStringLiteral("/dateTime?field=123456789") << headers << QByteArray() << invalid;

    QTest::newRow("datetime-empty") << QStringLiteral("/dateTime?field=") << headers << QByteArray() << valid;

    QTest::newRow("datetime-format-valid") << QStringLiteral("/dateTimeFormat?field=") + QDateTime::currentDateTime().toString(QStringLiteral("yyyy d MM mm:HH")) << headers << QByteArray() << valid;

    QTest::newRow("datetime-format-invalid") << QStringLiteral("/dateTimeFormat?field=") + QDateTime::currentDateTime().toString(QStringLiteral("MM mm yyyy HH d")) << headers << QByteArray() << invalid;





    // **** Start testing ValidatorDifferent *****

    QTest::newRow("different-valid") << QStringLiteral("/different?field=abcdefg&other=hijklmno") << headers << QByteArray() << valid;

    QTest::newRow("different-invalid") << QStringLiteral("/different?field=abcdefg&other=abcdefg") << headers << QByteArray() << invalid;

    QTest::newRow("different-empty") << QStringLiteral("/different?field=&other=hijklmno") << headers << QByteArray() << valid;

    QTest::newRow("different-other-missing") << QStringLiteral("/different?field=abcdefg") << headers << QByteArray() << valid;




    // **** Start testing ValidatorDigits *****

    QTest::newRow("digits-valid") << QStringLiteral("/digits?field=0123456") << headers << QByteArray() << valid;

    QTest::newRow("digits-invalid") << QStringLiteral("/digits?field=01234asdf56") << headers << QByteArray() << invalid;

    QTest::newRow("digits-empty") << QStringLiteral("/digits?field=") << headers << QByteArray() << valid;

    QTest::newRow("digits-length-valid") << QStringLiteral("/digitsLength?field=0123456789") << headers << QByteArray() << valid;

    QTest::newRow("digits-length-invalid") << QStringLiteral("/digitsLength?field=012345") << headers << QByteArray() << invalid;



    // **** Start testing ValidatorDigitsBetween *****

    QTest::newRow("digitsbetween-valid") << QStringLiteral("/digitsBetween?field=0123456") << headers << QByteArray() << valid;

    QTest::newRow("digitsbetween-invalid") << QStringLiteral("/digitsBetween?field=01234ad56") << headers << QByteArray() << invalid;

    QTest::newRow("digitsbetween-empty") << QStringLiteral("/digitsBetween?field=") << headers << QByteArray() << valid;

    QTest::newRow("digitsbetween-invalid-lower") << QStringLiteral("/digitsBetween?field=0123") << headers << QByteArray() << invalid;

    QTest::newRow("digitsbetween-invalid-greater") << QStringLiteral("/digitsBetween?field=0123456789123") << headers << QByteArray() << invalid;


    // **** Start testing ValidatorDomain *****

    QByteArray domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("huessenbergnetz.de"));
    QTest::newRow("domain-valid01") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("a.de"));
    QTest::newRow("domain-valid02") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("a1.de"));
    QTest::newRow("domain-valid03") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.com."));
    QTest::newRow("domain-valid04") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("test-1.example.com."));
    QTest::newRow("domain-valid05") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com")); // label with max length of 63 chars
    QTest::newRow("domain-valid06") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com")); // total length of 253 chars
    QTest::newRow("domain-valid07") << QStringLiteral("/domain") << headers << domainBody << valid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("hüssenbergnetz.de"));
    QTest::newRow("domain-valid08") << QStringLiteral("/domain") << headers << domainBody << valid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("موقع.وزارة-الاتصالات.مصر"));
    QTest::newRow("domain-valid09") << QStringLiteral("/domain") << headers << domainBody << valid;
#endif

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.com1")); // digit in non puny code TLD
    QTest::newRow("domain-invalid01") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.c")); // one char tld
    QTest::newRow("domain-invalid02") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.3com")); // starts with digit
    QTest::newRow("domain-invalid03") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.co3m")); // contains digit
    QTest::newRow("domain-invalid04") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl.com")); // label too long, 64 chars
    QTest::newRow("domain-invalid05") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.com")); // too long, 254 chars
    QTest::newRow("domain-invalid06") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.co-m")); // contains dash in tld
    QTest::newRow("domain-invalid07") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("-example.com")); // contains dash at label start
    QTest::newRow("domain-invalid08") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("3example.com")); // contains digit at label start
    QTest::newRow("domain-invalid09") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example-.com")); // contains dash at label end
    QTest::newRow("domain-invalid10") << QStringLiteral("/domain") << headers << domainBody << invalid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("موقع.وزارة-الاتصالات.مصر1"));
    QTest::newRow("domain-invalid11") << QStringLiteral("/domain") << headers << domainBody << invalid;

    domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("موقع.وزارة-الاتصالات.مصر-"));
    QTest::newRow("domain-invalid12") << QStringLiteral("/domain") << headers << domainBody << invalid;
#endif

    if (qEnvironmentVariableIsSet("CUTELYST_VALIDATORS_TEST_NETWORK")) {
        domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("example.com"));
        QTest::newRow("domain-dns-valid") << QStringLiteral("/domainDns") << headers << domainBody << valid;

        domainBody = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("test.example.com"));
        QTest::newRow("domain-dns-invalid") << QStringLiteral("/domainDns") << headers << domainBody << invalid;
    }


    // **** Start testing ValidatorEmail *****

    const QList<QString> validEmails({
                                         QStringLiteral("test@huessenbergnetz.de"),
                                         // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                         QStringLiteral("test@iana.org"),
                                         QStringLiteral("test@nominet.org.uk"),
                                         QStringLiteral("test@about.museum"),
                                         QStringLiteral("a@iana.org"),
                                         QStringLiteral("test.test@iana.org"),
                                         QStringLiteral("!#$%&`*+/=?^`{|}~@iana.org"),
                                         QStringLiteral("123@iana.org"),
                                         QStringLiteral("test@123.com"),
                                         QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@iana.org"),
                                         QStringLiteral("test@mason-dixon.com"),
                                         QStringLiteral("test@c--n.com"),
                                         QStringLiteral("test@xn--hxajbheg2az3al.xn--jxalpdlp"),
                                         QStringLiteral("xn--test@iana.org"),
                                         // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                         QStringLiteral("first.last@iana.org"),
                                         QStringLiteral("1234567890123456789012345678901234567890123456789012345678901234@iana.org"),
                                         QStringLiteral("first.last@3com.com"),
                                         QStringLiteral("user+mailbox@iana.org"),
                                         QStringLiteral("customer/department=shipping@iana.org"),
                                         QStringLiteral("$A12345@iana.org"),
                                         QStringLiteral("!def!xyz%abc@iana.org"),
                                         QStringLiteral("_somename@iana.org"),
                                         QStringLiteral("dclo@us.ibm.com"),
                                         QStringLiteral("peter.piper@iana.org"),
                                         QStringLiteral("TEST@iana.org"),
                                         QStringLiteral("1234567890@iana.org"),
                                         QStringLiteral("test+test@iana.org"),
                                         QStringLiteral("test-test@iana.org"),
                                         QStringLiteral("t*est@iana.org"),
                                         QStringLiteral("+1~1+@iana.org"),
                                         QStringLiteral("{_test_}@iana.org"),
                                         QStringLiteral("test.test@iana.org"),
                                         QStringLiteral("customer/department@iana.org"),
                                         QStringLiteral("Yosemite.Sam@iana.org"),
                                         QStringLiteral("~@iana.org"),
                                         QStringLiteral("Ima.Fool@iana.org"),
                                         QStringLiteral("name.lastname@domain.com"),
                                         QStringLiteral("a@bar.com"),
                                         QStringLiteral("a-b@bar.com"),
                                         QStringLiteral("valid@about.museum"),
                                         QStringLiteral("user%uucp!path@berkeley.edu"),
                                         QStringLiteral("cdburgess+!#$%&'*-/=?+_{}|~test@gmail.com")
                                     });

    const QList<QString> dnsWarnEmails({
                                           QStringLiteral("test@example.com"), // no mx record
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                           QStringLiteral("test@e.com"), // no record
                                           QStringLiteral("test@iana.a"), // no record
                                           QStringLiteral("test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.com"), // no record
                                           QStringLiteral("test@iana.co-uk"), // no record
                                           QStringLiteral("a@a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v.w.x.y.z.a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t.u.v"), // no record
                                           QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghi"),
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                           QStringLiteral("x@x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x2"), // no record
                                           QStringLiteral("1234567890123456789012345678901234567890123456789012345678901@12345678901234567890123456789012345678901234567890123456789.12345678901234567890123456789012345678901234567890123456789.123456789012345678901234567890123456789012345678901234567890123.iana.org"), // no record
                                           QStringLiteral("first.last@x23456789012345678901234567890123456789012345678901234567890123.iana.org"), // no record
                                           QStringLiteral("first.last@123.iana.org"), // no record
                                           QStringLiteral("test@123.123.123.x123"), // no record
                                           QStringLiteral("test@example.iana.org"), // no record
                                           QStringLiteral("test@example.example.iana.org"), // no record
                                           QStringLiteral("+@b.c"), // no record
                                           QStringLiteral("+@b.com"), // no record
                                           QStringLiteral("a@b.co-foo.uk"), // no record
                                           QStringLiteral("shaitan@my-domain.thisisminekthx"), // no record
                                           QStringLiteral("test@xn--example.com") // no record
                                       });

    const QList<QString> rfc5321Emails({
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                           QStringLiteral("\"first\\\"last\"@iana.org"), // quoted string
                                           QStringLiteral("\"first@last\"@iana.org"), // quoted string
                                           QStringLiteral("\"first\\\\last\"@iana.org"), // quoted string
                                           QStringLiteral("first.last@[12.34.56.78]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::12.34.56.78]"), // address literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333::4444:12.34.56.78]"), // address literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:12.34.56.78]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::1111:2222:3333:4444:5555:6666]"), // address literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333::4444:5555:6666]"), // address literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888]"), // address literal
                                           QStringLiteral("\"first\\last\"@iana.org"), // quoted string
                                           QStringLiteral("\"\"@iana.org"), // quoted string
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333::4444:5555:12.34.56.78]"), // ipv6 deprecated
                                           QStringLiteral("first.last@example.123"), // tld numeric
                                           QStringLiteral("first.last@com"), // tld
                                           QStringLiteral("\"Abc\\@def\"@iana.org"), // quoted string
                                           QStringLiteral("\"Fred\\ Bloggs\"@iana.org"), // quoted string
                                           QStringLiteral("\"Joe.\\\\Blow\"@iana.org"), // quoted string
                                           QStringLiteral("\"Abc@def\"@iana.org"), // quoted string
                                           QStringLiteral("\"Fred Bloggs\"@iana.org"), // quoted string
                                           QStringLiteral("\"Doug \\\"Ace\\\" L.\"@iana.org"), // quoted string
                                           QStringLiteral("\"[[ test ]]\"@iana.org"), // quoted string
                                           QStringLiteral("\"test.test\"@iana.org"), // quoted string
                                           QStringLiteral("\"test@test\"@iana.org"), // quoted string
                                           QStringLiteral("test@123.123.123.123"), // tld numeric
                                           QStringLiteral("test@[123.123.123.123]"), // address literal
                                           QStringLiteral("\"test\\test\"@iana.org"), // quoted string
                                           QStringLiteral("test@example"), // tld
                                           QStringLiteral("\"test\\\\blah\"@iana.org"), //quoted string
                                           QStringLiteral("\"test\\blah\"@iana.org"), // quoted string
                                           QStringLiteral("\"test\\\"blah\"@iana.org"), // quoted string
                                           QStringLiteral("\"Austin@Powers\"@iana.org"), // quoted string
                                           QStringLiteral("\"Ima.Fool\"@iana.org"), // quoted string
                                           QStringLiteral("\"Ima Fool\"@iana.org"), // quoted string
                                           QStringLiteral("\"first.middle.last\"@iana.org"), // quoted string
                                           QStringLiteral("\"first..last\"@iana.org"), // quoted string
                                           QStringLiteral("\"first\\\\\\\"last\"@iana.org"), // quoted string
                                           QStringLiteral("a@b"), // tld
                                           QStringLiteral("aaa@[123.123.123.123]"), // address literal
                                           QStringLiteral("a@bar"), // tld
                                           QStringLiteral("\"hello my name is\"@stutter.com"), // quoted string
                                           QStringLiteral("\"Test \\\"Fail\\\" Ing\"@iana.org"), // quoted string
                                           QStringLiteral("foobar@192.168.0.1"), // tld numeric
                                           QStringLiteral("\"Joe\\\\Blow\"@iana.org"), // quoted string
                                           QStringLiteral("\"first(last)\"@iana.org"), // quoted string
                                           QStringLiteral("first.last@[IPv6:::a2:a3:a4:b1:b2:b3:b4]"), // ipv6 deprecated
                                           QStringLiteral("first.last@[IPv6:a1:a2:a3:a4:b1:b2:b3::]"), // ipv6 deprecated
                                           QStringLiteral("first.last@[IPv6:::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::b4]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::b3:b4]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1::b4]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1:a2::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:cdef::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:CDEF::]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::a3:a4:b1:ffff:11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:::a2:a3:a4:b1:ffff:11.22.33.44]"), // ipv6 deprecated
                                           QStringLiteral("first.last@[IPv6:a1:a2:a3:a4::11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1:a2:a3:a4:b1::11.22.33.44]"), // ipv6 deprecated
                                           QStringLiteral("first.last@[IPv6:a1::11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1:a2::11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:cdef::11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:CDEF::11.22.33.44]"), // address literal
                                           QStringLiteral("first.last@[IPv6:a1::b2:11.22.33.44]"), // address literal
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                           QStringLiteral("test@iana.123"), // tld numeric
                                           QStringLiteral("test@255.255.255.255"), // tld numeric
                                           QStringLiteral("\"test\"@iana.org"), // quoted string
                                           QStringLiteral("\"\"@iana.org"), // quoted string
                                           QStringLiteral("\"\\a\"@iana.org"), // quoted string
                                           QStringLiteral("\"\\\"\"@iana.org"), // quoted string
                                           QStringLiteral("\"\\\\\"@iana.org"), // quoted string
                                           QStringLiteral("\"test\\ test\"@iana.org"), // quoted string
                                           QStringLiteral("test@[255.255.255.255]"), // address literal
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888]"), // address literal
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666::8888]"), // ipv6 deprecated
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555::8888]"), // address literal
                                           QStringLiteral("test@[IPv6:::3333:4444:5555:6666:7777:8888]"), // address literal
                                           QStringLiteral("test@[IPv6:::]"), // address literal
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:255.255.255.255]"), // address literal
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444::255.255.255.255]"), // address literal
                                           QStringLiteral("test@org") // tld
                                       });

    const QList<QString> cfwsEmails({
                                        // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                        QStringLiteral("\r\n test@iana.org"), // folding white space
                                        QStringLiteral("(comment)test@iana.org"), // comment
                                        QStringLiteral("(comment(comment))test@iana.org"), // coment
                                        QStringLiteral("(comment)abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@iana.org"), // comment
                                        QStringLiteral("(comment)test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghik.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghik.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstuvwxyzabcdefghijk.abcdefghijklmnopqrstu"), // comment
                                        QStringLiteral(" \r\n test@iana.org"), // folding white space
                                        QStringLiteral("test@iana.org\r\n "), // folding white space
                                        QStringLiteral("test@iana.org \r\n "), // folding white space
                                        QStringLiteral(" test@iana.org"), // folding white space
                                        QStringLiteral("test@iana.org "), // folding white space
                                        // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                        QStringLiteral("\"test\r\n blah\"@iana.org"), // folding white space
                                        QStringLiteral("first.last@iana(1234567890123456789012345678901234567890123456789012345678901234567890).org") // comment
                                    });

    const QList<QString> deprecatedEmails({
                                              // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                              QStringLiteral("\"test\".\"test\"@iana.org"), // local part
                                              QStringLiteral("\"test\".test@iana.org"), // local part
//                                              QStringLiteral("\"test\\\0\"@iana.org"), // quoted pair
                                              QStringLiteral(" test @iana.org"), // folding white space near at
                                              QStringLiteral("test@ iana .com"), // folding white space near at
                                              QStringLiteral("test . test@iana.org"), // folding white space
                                              QStringLiteral("\r\n \r\n test@iana.org"), // folding white space
                                              QStringLiteral("test@(comment)iana.org"), // comment near at
                                              QStringLiteral("test@(comment)[255.255.255.255]"), // comment near at
                                              QStringLiteral("test@(comment)abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.com"), // comment near at
//                                              QStringLiteral("\"\"@iana.org"), // quoted string with deprecated char - currently also not working on upstream
//                                              QStringLiteral("\"\\\"@iana.org"), // quoted string with deprecated char
//                                              QStringLiteral("()test@iana.org"), // comment string with deprecated char - currently also not working on upstream
                                              QStringLiteral("\"\\\n\"@iana.org"), // quoted pair with deprecated char
                                              QStringLiteral("\"\a\"@iana.org"), // quoted string with deprecated char
                                              QStringLiteral("\"\\\a\"@iana.org"), // quoted pair with deprecated char
                                              QStringLiteral("(\a)test@iana.org"), // comment with deprecated char
                                              QStringLiteral("test@iana.org\r\n \r\n "), // obsolete folding white space
                                              QStringLiteral("test.(comment)test@iana.org"), // deprecated comment position
                                              // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                              QStringLiteral("test.\"test\"@iana.org"), // local part
                                              QStringLiteral("\"test\\\rblah\"@iana.org"), // quoted pair with deprecated char
                                              QStringLiteral("\"first\".\"last\"@iana.org"), // local part
                                              QStringLiteral("\"first\".middle.\"last\"@iana.org"), // local part
                                              QStringLiteral("\"first\".last@iana.org"), // local part
                                              QStringLiteral("first.\"last\"@iana.org"), // local part
                                              QStringLiteral("\"first\".\"middle\".\"last\"@iana.org"), // local part
                                              QStringLiteral("\"first.middle\".\"last\"@iana.org"), // local part
                                              QStringLiteral("first.\"mid\\dle\".\"last\"@iana.org"), // local part
                                              QStringLiteral("Test.\r\n Folding.\r\n Whitespace@iana.org"), // folding white space
                                              QStringLiteral("first.\"\".last@iana.org"), // local part
                                              QStringLiteral("(foo)cal(bar)@(baz)iamcal.com(quux)"), // comment near at
                                              QStringLiteral("cal@iamcal(woo).(yay)com"), // comment position
                                              QStringLiteral("\"foo\"(yay)@(hoopla)[1.2.3.4]"), // comment near at
                                              QStringLiteral("cal(woo(yay)hoopla)@iamcal.com"), // comment near at
                                              QStringLiteral("cal(foo\\@bar)@iamcal.com"), // comment near at
                                              QStringLiteral("cal(foo\\)bar)@iamcal.com"), // comment near at
                                              QStringLiteral("first().last@iana.org"), // local part
                                              QStringLiteral("first.(\r\n middle\r\n )last@iana.org"), // deprecated comment
                                              QStringLiteral("first(Welcome to\r\n the (\"wonderful\" (!)) world\r\n of email)@iana.org"), // comment near at
                                              QStringLiteral("pete(his account)@silly.test(his host)"), // comment near at
                                              QStringLiteral("c@(Chris's host.)public.example"), // comment near at
                                              QStringLiteral("jdoe@machine(comment). example"), // folding white space
                                              QStringLiteral("1234 @ local(blah) .machine .example"), // white space near at
                                              QStringLiteral("first(abc.def).last@iana.org"), // local part
                                              QStringLiteral("first(a\"bc.def).last@iana.org"), // local part
                                              QStringLiteral("first.(\")middle.last(\")@iana.org"), // local part
                                              QStringLiteral("first(abc\\(def)@iana.org"), // comment near at
                                              QStringLiteral("a(a(b(c)d(e(f))g)h(i)j)@iana.org"),
                                              QStringLiteral("HM2Kinsists@(that comments are allowed)this.is.ok"), // comment near at
                                              QStringLiteral(" \r\n (\r\n x \r\n ) \r\n first\r\n ( \r\n x\r\n ) \r\n .\r\n ( \r\n x) \r\n last \r\n ( x \r\n ) \r\n @iana.org"), // folding white space near at
                                              QStringLiteral("first.last @iana.org"), // folding white space near at
                                              QStringLiteral("test. \r\n \r\n obs@syntax.com") // folding white space
//                                              QStringLiteral("\"Unicode NULL \\\0\"@char.com") // quoted pair contains deprecated char
                                          });

    const QList<QString> rfc5322Emails({
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                           QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklmn@iana.org"), // local too long
                                           QStringLiteral("test@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm.com"), // label too long
                                           QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghiklm@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghij"), // too long
                                           QStringLiteral("a@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg.hij"), // too long
                                           QStringLiteral("a@abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghikl.abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefg.hijk"), // too long
                                           QStringLiteral("\"abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghj\"@iana.org"), // local too long
                                           QStringLiteral("\"abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefg\\h\"@iana.org"), // local too long
                                           QStringLiteral("test@[255.255.255]"), // invalid domain literal
                                           QStringLiteral("test@[255.255.255.255.255]"), // invalid domain litearl
                                           QStringLiteral("test@[255.255.255.256]"), // invalid domain literal
                                           QStringLiteral("test@[1111:2222:3333:4444:5555:6666:7777:8888]"), // invalid domain literal
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:7777]"), // ipv6 group count
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888:9999]"), // ipv6 group count
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:7777:888G]"), // ipv6 bad char
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666::7777:8888]"), // ipv6 max groups
                                           QStringLiteral("test@[IPv6::3333:4444:5555:6666:7777:8888]"), // ipv6 colon start
                                           QStringLiteral("test@[IPv6:1111::4444:5555::8888]"), // ipv6 2x2x colon
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:255.255.255.255]"), // ipv6 group count
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666:7777:255.255.255.255]"), // ipv6 group count
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:5555:6666::255.255.255.255]"), // ipv6 max groups
                                           QStringLiteral("test@[IPv6:1111:2222:3333:4444:::255.255.255.255]"), // ipv6 2x2x colon
                                           QStringLiteral("test@[IPv6::255.255.255.255]"), // ipv6 colon start
                                           QStringLiteral("test@[RFC-5322-domain-literal]"), // invalid domain literal
                                           QStringLiteral("test@[RFC-5322-\\\a-domain-literal]"), // invalid domain literal containing obsolete chars
                                           QStringLiteral("test@[RFC-5322-\\\t-domain-literal]"), // invalid domain literal containing obsolete chars
                                           QStringLiteral("test@[RFC-5322-\\]-domain-literal]"), // invalid domain literal containing obsolete chars
                                           QStringLiteral("test@[RFC 5322 domain literal]"), // invalid domain literal
                                           QStringLiteral("test@[RFC-5322-domain-literal] (comment)"), // invalid domain literal
                                           QStringLiteral("test@[IPv6:1::2:]"), // ipv6 colon end
                                           QStringLiteral("test@iana/icann.org"), // domain invalid for DNS
                                           // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                           QStringLiteral("123456789012345678901234567890123456789012345678901234567890@12345678901234567890123456789012345678901234567890123456789.12345678901234567890123456789012345678901234567890123456789.12345678901234567890123456789012345678901234567890123456789.12345.iana.org"), // too long
                                           QStringLiteral("12345678901234567890123456789012345678901234567890123456789012345@iana.org"), // local too long
                                           QStringLiteral("x@x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456"), // domain too long
                                           QStringLiteral("first.last@[.12.34.56.78]"), // invalid domain literal
                                           QStringLiteral("first.last@[12.34.56.789]"), // invalid domain literal
                                           QStringLiteral("first.last@[::12.34.56.78]"), // invalid domain literal
                                           QStringLiteral("first.last@[IPv5:::12.34.56.78]"), // invalid domain literal
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:12.34.56.78]"), // ipv6 group count
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:12.34.56.78]"), // ipv6 group count
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777]"), // ipv6 group count
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:7777:8888:9999]"), // ipv6 group count
                                           QStringLiteral("first.last@[IPv6:1111:2222::3333::4444:5555:6666]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:1111:2222:333x::4444:5555]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:1111:2222:33333::4444:5555]"), // ipv6 bad char
                                           QStringLiteral("first.last@x234567890123456789012345678901234567890123456789012345678901234.iana.org"), // label too long
                                           QStringLiteral("test@123456789012345678901234567890123456789012345678901234567890123.123456789012345678901234567890123456789012345678901234567890123.123456789012345678901234567890123456789012345678901234567890123.123456789012345678901234567890123456789012345678901234567890.com"), // domain too long
                                           QStringLiteral("foo@[\\1.2.3.4]"), // invalid domain literal containing obsolete chars
                                           QStringLiteral("first.last@[IPv6:1111:2222:3333:4444:5555:6666:12.34.567.89]"), // ipv6 bad char
                                           QStringLiteral("aaa@[123.123.123.333]"), // invalid domain literal
                                           QStringLiteral("first.last@[IPv6::]"), // ipv6 colon start
                                           QStringLiteral("first.last@[IPv6::::]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6::b4]"), // ipv6 colon start
                                           QStringLiteral("first.last@[IPv6::::b4]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6::b3:b4]"), // ipv6 colon start
                                           QStringLiteral("first.last@[IPv6::::b3:b4]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1:::b4]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1:]"), // ipv6 colon end
                                           QStringLiteral("first.last@[IPv6:a1:::]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1:a2:]"), // ipv6 colon end
                                           QStringLiteral("first.last@[IPv6:a1:a2:::]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6::11.22.33.44]"), // ipv6 colon start
                                           QStringLiteral("first.last@[IPv6::::11.22.33.44]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1:11.22.33.44]"), // ipv6 group count
                                           QStringLiteral("first.last@[IPv6:a1:::11.22.33.44]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1:a2:::11.22.33.44]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:cdef::11.22.33.xx]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:0123:4567:89ab:CDEFF::11.22.33.44]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:a1::a4:b1::b4:11.22.33.44]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1::11.22.33]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:a1::11.22.33.44.55]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:a1::b211.22.33.44]"), // ipv6 bad char
                                           QStringLiteral("first.last@[IPv6:a1::b2::11.22.33.44]"), // ipv6 2x2x colon
                                           QStringLiteral("first.last@[IPv6:a1::b3:]"), // ipv6 colon end
                                           QStringLiteral("first.last@[IPv6::a2::b4]"), // ipv6 colon start
                                           QStringLiteral("first.last@[IPv6:a1:a2:a3:a4:b1:b2:b3:]"), // ipv6 colon end
                                           QStringLiteral("first.last@[IPv6::a2:a3:a4:b1:b2:b3:b4]"), // ipv6 colon end
                                           QStringLiteral("first.last@[IPv6:a1:a2:a3:a4::b1:b2:b3:b4]") // ipv6 max groups
                                       });

    QList<QString> errorEmails({
                                   // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests.xml
                                   QStringLiteral(" "), // no domain
                                   QStringLiteral("test"), // no domain
                                   QStringLiteral("@"), // no local part
                                   QStringLiteral("test@"), // no domain
                                   QStringLiteral("@io"), // no local part
                                   QStringLiteral("@iana.org"), // no local part
                                   QStringLiteral(".test@iana.org"), // dot start
                                   QStringLiteral("test.@iana.org"), // dot end
                                   QStringLiteral("test..iana.org"), // consecutive dots
                                   QStringLiteral("test_exa-mple.com"), // no domain
                                   QStringLiteral("test\\@test@iana.org"), // expecting atext
                                   QStringLiteral("test@-iana.org"), // domain hypen start
                                   QStringLiteral("test@iana-.com"), // domain hypen end
                                   QStringLiteral("test@.iana.org"), // dot start
                                   QStringLiteral("test@iana.org."), // dot end
                                   QStringLiteral("test@iana..com"), // consecutive dots
                                   QStringLiteral("\"\"\"@iana.org"), // expecting atext
                                   QStringLiteral("\"\\\"@iana.org"), // unclosed quoted string
                                   QStringLiteral("test\"@iana.org"), // expecting atext
                                   QStringLiteral("\"test@iana.org"), // unclosed quoted string
                                   QStringLiteral("\"test\"test@iana.org"), // atext after quoted string
                                   QStringLiteral("test\"text\"@iana.org"), // expecting atext
                                   QStringLiteral("\"test\"\"test\"@iana.org"), // expecting atext
//                                   QStringLiteral("\"test\0\"@iana.org"), // expecting qtext
                                   QStringLiteral("test@a[255.255.255.255]"), // expecting atext
                                   QStringLiteral("((comment)test@iana.org"), // unclosed comment
                                   QStringLiteral("test(comment)test@iana.org"), // atext after comment
                                   QStringLiteral("test@iana.org\n"), // expecting atext
                                   QStringLiteral("test@iana.org-"), // domain hypehn end
                                   QStringLiteral("\"test@iana.org"), // unclosed quoted string
                                   QStringLiteral("(test@iana.org"), // unclosed comment
                                   QStringLiteral("test@(iana.org"), // unclosed comment
                                   QStringLiteral("test@[1.2.3.4"), // unclosed domain literal
                                   QStringLiteral("\"test\\\"@iana.org"), // unclosed quoted string
                                   QStringLiteral("(comment\\)test@iana.org"), // unclosed comment
                                   QStringLiteral("test@iana.org(comment\\)"), // unclosed comment
                                   QStringLiteral("test@iana.org(comment\\"), // backslash end
                                   QStringLiteral("test@[RFC-5322]-domain-literal]"), // atext after domain literal
                                   QStringLiteral("test@[RFC-5322-[domain-literal]"), // expecting dtext
                                   QStringLiteral("test@[RFC-5322-domain-literal\\]"), // unclosed domain literal
                                   QStringLiteral("test@[RFC-5322-domain-literal\\"), // backslash end
                                   QStringLiteral("@iana.org"), // expecting atext
                                   QStringLiteral("test@.org"), // expecting atext
                                   QStringLiteral("test@iana.org\r"), // no lf after cr
                                   QStringLiteral("\rtest@iana.org"), // no lf after cr
                                   QStringLiteral("\"\rtest\"@iana.org"), // no lf after cr
                                   QStringLiteral("(\r)test@iana.org"), // no lf after cr
                                   QStringLiteral("test@iana.org(\r)"), // no lf after cr
                                   QStringLiteral("\ntest@iana.org"), // expecting atext
                                   QStringLiteral("\"\n\"@iana.org"), // expecting qtext
                                   QStringLiteral("(\n)test@iana.org"), // expecting ctext
                                   QStringLiteral("\a@iana.org"), // expecting atext
                                   QStringLiteral("test@\a.org"), // expecting atext
                                   QStringLiteral("\r\ntest@iana.org"), // folding white space ends with CRLF
                                   QStringLiteral("\r\n \r\ntest@iana.org"), // folding white space ends with CRLF
                                   QStringLiteral(" \r\ntest@iana.org"), // folding white space ends with CRLF
                                   QStringLiteral(" \r\n \r\ntest@iana.org"), // folding white space ends with CRLF
                                   QStringLiteral(" \r\n\r\ntest@iana.org"), // Folding White Space contains consecutive CRLF sequences
                                   QStringLiteral(" \r\n\r\n test@iana.org"), // Folding White Space contains consecutive CRLF sequences
                                   QStringLiteral("test@iana.org\r\n"), // Folding White Space ends with a CRLF sequence
                                   QStringLiteral("test@iana.org\r\n \r\n"), // Folding White Space ends with a CRLF sequence
                                   QStringLiteral("test@iana.org \r\n"), // Folding White Space ends with a CRLF sequence
                                   QStringLiteral("test@iana.org \r\n \r\n"), // Folding White Space ends with a CRLF sequence
                                   QStringLiteral("test@iana.org \r\n\r\n"), // Folding White Space contains consecutive CRLF sequences
                                   QStringLiteral("test@iana.org \r\n\r\n "), // Folding White Space contains consecutive CRLF sequences
                                   QStringLiteral("\"test\\©\"@iana.org"), // expecting quoted pair
                                   // addresses are taken from https://github.com/dominicsayers/isemail/blob/master/test/tests-original.xml
                                   QStringLiteral("first.last@sub.do,com"), // expecting atext
                                   QStringLiteral("first\\@last@iana.org"), // expecting atext
                                   QStringLiteral("first.last"), // no domain
                                   QStringLiteral(".first.last@iana.org"), // dot start
                                   QStringLiteral("first.last.@iana.org"), // dot end
                                   QStringLiteral("first..last@iana.org"), // consecutive dots
                                   QStringLiteral("\"first\"last\"@iana.org"), // atext after quoted string
                                   QStringLiteral("\"\"\"@iana.org"), // expecting atext
                                   QStringLiteral("\"\\\"@iana.org"), // unclosed quoted string
                                   QStringLiteral("first\\\\@last@iana.org"), // expecting atext
                                   QStringLiteral("first.last@"), // no domain
                                   QStringLiteral("first.last@-xample.com"), // domain hyphen start
                                   QStringLiteral("first.last@exampl-.com"), // domain hyphen end
                                   QStringLiteral("abc\\@def@iana.org"), // expecting atext
                                   QStringLiteral("abc\\\\@iana.org"), // expecting atext
                                   QStringLiteral("Doug\\ \\\"Ace\\\"\\ Lovell@iana.org"), // expecting atext
                                   QStringLiteral("abc@def@iana.org"), // expecting atext
                                   QStringLiteral("abc\\\\@def@iana.org"), // expecting atext
                                   QStringLiteral("abc\\@iana.org"), // expecting atext
                                   QStringLiteral("@iana.org"), // no local part
                                   QStringLiteral("doug@"), // no domain
                                   QStringLiteral("\"qu@iana.org"), // unclosed quoted string
                                   QStringLiteral("ote\"@iana.org"), // expecting atext
                                   QStringLiteral(".dot@iana.org"), // dot start
                                   QStringLiteral("dot.@iana.org"), // dot end
                                   QStringLiteral("two..dot@iana.org"), // consecutive dots
                                   QStringLiteral("\"Doug \"Ace\" L.\"@iana.org"), // atext after quoted string
                                   QStringLiteral("Doug\\ \\\"Ace\\\"\\ L\\.@iana.org"), // expecting atext
                                   QStringLiteral("hello world@iana.org"), // atext after folding white space
                                   QStringLiteral("gatsby@f.sc.ot.t.f.i.tzg.era.l.d."), // dot end
                                   QStringLiteral("test.iana.org"), // no domain
                                   QStringLiteral("test.@iana.org"), // dot end
                                   QStringLiteral("test..test@iana.org"), // consecutive dots
                                   QStringLiteral(".test@iana.org"), // dot start
                                   QStringLiteral("test@test@iana.org"), // expecting atext
                                   QStringLiteral("test@@iana.org"), // expecting atext
                                   QStringLiteral("-- test --@iana.org"), // atext after folding white space
                                   QStringLiteral("[test]@iana.org"), // expecting atext
                                   QStringLiteral("\"test\"test\"@iana.org"), // atext after quoted string
                                   QStringLiteral("()[]\\;:,><@iana.org"), // expecting atext
                                   QStringLiteral("test@."), // dot start
                                   QStringLiteral("test@example."), // dot end
                                   QStringLiteral("test@.org"), // dot start
                                   QStringLiteral("test@[123.123.123.123"), // unclosed domain literal
                                   QStringLiteral("test@123.123.123.123]"), // expecting atext
                                   QStringLiteral("NotAnEmail"), // no domain
                                   QStringLiteral("@NotAnEmail"), // no local part
                                   QStringLiteral("\"test\rblah\"@iana.org"), // cr no lf
                                   QStringLiteral("\"test\"blah\"@iana.org"), // atext after quoted string
                                   QStringLiteral(".wooly@iana.org"), // dot start
                                   QStringLiteral("wo..oly@iana.org"), // consecutive dots
                                   QStringLiteral("pootietang.@iana.org"), // dot end
                                   QStringLiteral(".@iana.org"), // dot start
                                   QStringLiteral("Ima Fool@iana.org"), // atext after white space
                                   QStringLiteral("phil.h\\@\\@ck@haacked.com"), // expecting atext
                                   QStringLiteral("\"first\\\\\"last\"@iana.org"), // atext after quoted string
                                   QStringLiteral("first\\last@iana.org"), // expecting atext
                                   QStringLiteral("Abc\\@def@iana.org"), // expecting atext
                                   QStringLiteral("Fred\\ Bloggs@iana.org"), // expectin atext
                                   QStringLiteral("Joe.\\\\Blow@iana.org"), // expecting atext
                                   QStringLiteral("\"test\\\r\n blah\"@iana.org"), // expecting qtext
                                   QStringLiteral("{^c\\@**Dog^}@cartoon.com"), // expecting atext
                                   QStringLiteral("cal(foo(bar)@iamcal.com"), // unclosed comment
                                   QStringLiteral("cal(foo)bar)@iamcal.com"), // atext after comment
                                   QStringLiteral("cal(foo\\)@iamcal.com"), // unclosed comment
                                   QStringLiteral("first(12345678901234567890123456789012345678901234567890)last@(1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890)iana.org"), // atext after comment
                                   QStringLiteral("first(middle)last@iana.org"), // atext after comment
                                   QStringLiteral("first(abc(\"def\".ghi).mno)middle(abc(\"def\".ghi).mno).last@(abc(\"def\".ghi).mno)example(abc(\"def\".ghi).mno).(abc(\"def\".ghi).mno)com(abc(\"def\".ghi).mno)"), // atext after comment
                                   QStringLiteral("a(a(b(c)d(e(f))g)(h(i)j)@iana.org"), // unclosed comment
                                   QStringLiteral(".@"), // dot start
                                   QStringLiteral("@bar.com"), // no local part
                                   QStringLiteral("@@bar.com"), // no local part
                                   QStringLiteral("aaa.com"), // no domain
                                   QStringLiteral("aaa@.com"), // dot start
                                   QStringLiteral("aaa@.123"), // dot start
                                   QStringLiteral("aaa@[123.123.123.123]a"), // atext after domain literal
                                   QStringLiteral("a@bar.com."), // dot end
                                   QStringLiteral("a@-b.com"), // domain hyphen start
                                   QStringLiteral("a@b-.com"), // domain hypen end
                                   QStringLiteral("-@..com"), // dot start
                                   QStringLiteral("-@a..com"), // consecutive dots
                                   QStringLiteral("invalid@about.museum-"), // domain hyphen end
                                   QStringLiteral("test@...........com"), // dot start
                                   QStringLiteral("Invalid \\\n Folding \\\n Whitespace@iana.org"), // atext after white space
                                   QStringLiteral("test.\r\n\r\n obs@syntax.com"), // Folding White Space contains consecutive CRLF sequences
//                                   QStringLiteral("\"Unicode NULL \0\"@char.com"), // expecting qtext
//                                   QStringLiteral("Unicode NULL \\0@char.com"), // atext after cfws
                                   QStringLiteral("test@example.com\n") // expecting atext
                               });

    count = 0;
    for (const QString &email : validEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-valid-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailValid") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : {QStringLiteral("test@hüssenbergnetz.de"), QStringLiteral("täst@huessenbergnetz.de"), QStringLiteral("täst@hüssenbergnetz.de")}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-valid-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailValid") << headers << body << invalid;
        count++;
    }

    if (qEnvironmentVariableIsSet("CUTELYST_VALIDATORS_TEST_NETWORK")) {
        QTest::newRow("email-valid-dns") << QStringLiteral("/emailDnsWarnValid") << headers << QByteArrayLiteral("field=test@huessenbergnetz.de") << valid;
        count = 0;
        for (const QString &email : dnsWarnEmails) {
            const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
            QTest::newRow(QString(QStringLiteral("email-dnswarn-valid-%1").arg(count)).toUtf8().constData())
                    << QStringLiteral("/emailDnsWarnValid") << headers << body << invalid;
            count++;
        }
    }

    count = 0;
    for (const QString &email : rfc5321Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-rfc5321-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailRfc5321Valid") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5321Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-rfc5321-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailRfc5321Invalid") << headers << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : cfwsEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-cfws-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailCfwsValid") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : deprecatedEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-deprecated-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailDeprecatedValid") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : deprecatedEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-deprecated-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailDeprecatedInvalid") << headers << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5322Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-rfc5322-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailRfc5322Valid") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &email : rfc5322Emails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-rfc5322-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailRfc5322Invalid") << headers << body << invalid;
        count++;
    }

    count = 0;
    for (const QString &email : errorEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-errors-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailErrors") << headers << body << invalid;
        count++;
    }

    {
        QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("test@hüssenbergnetz.de"));
        QTest::newRow("email-idnallowed-valid") << QStringLiteral("/emailIdnAllowed") << headers << body << valid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("täst@hüssenbergnetz.de"));
        QTest::newRow("email-idnallowed-invalid") << QStringLiteral("/emailIdnAllowed") << headers << body << invalid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("täst@huessenbergnetz.de"));
        QTest::newRow("email-utf8localallowed-valid") << QStringLiteral("/emailUtf8Local") << headers << body << valid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("täst@hüssenbergnetz.de"));
        QTest::newRow("email-utf8localallowed-invalid") << QStringLiteral("/emailUtf8Local") << headers << body << invalid;

        body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(QStringLiteral("täst@hüssenbergnetz.de"));
        QTest::newRow("email-utf8allowed-valid-0") << QStringLiteral("/emailUtf8") << headers << body << valid;
    }

    count = 1;
    for (const QString &email : validEmails) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(email);
        QTest::newRow(QString(QStringLiteral("email-utf8allowed-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailUtf8") << headers << body << valid;
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
        QTest::newRow(QString(QStringLiteral("email-utf8allowed-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/emailUtf8") << headers << body << invalid;
        count++;
    }

    QTest::newRow("email-empty") << QStringLiteral("/emailValid") << headers << QByteArrayLiteral("field=") << valid;


    // **** Start testing ValidatorFileSize *****

    count = 0;
    for (const QString &size : {
         QStringLiteral("1M"),
         QStringLiteral("M1"),
         QStringLiteral("1 G"),
         QStringLiteral("G 1"),
         QStringLiteral("1.5 G"),
         QStringLiteral("G 1.5"),
         QStringLiteral("2.345 TiB"),
         QStringLiteral("TiB2.345"),
         QStringLiteral("5B"),
         QStringLiteral("B5"),
         QStringLiteral("5 B"),
         QStringLiteral("B 5"),
         QStringLiteral(" 2.0 Gi"),
         QStringLiteral(" Gi 2.0"),
         QStringLiteral("2.0 Gi "),
         QStringLiteral("Gi 2.0 "),
         QStringLiteral(" 2.0 Gi "),
         QStringLiteral(" Gi 2.0 "),
         QStringLiteral(" 2.0    Gi "),
         QStringLiteral(" Gi    2.0 "),
         QStringLiteral("3.67YB"),
         QStringLiteral("YB3.67"),
         QStringLiteral("1"),
         QStringLiteral("1024"),
         QStringLiteral(".5MB"),
         QStringLiteral("MB.5")}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(size);
        QTest::newRow(QString(QStringLiteral("filesize-valid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/fileSize") << headers << body << valid;
        count++;
    }

    count = 0;
    for (const QString &size : {
         QStringLiteral("1QiB"),
         QStringLiteral("QiB1"),
         QStringLiteral(" 1QiB"),
         QStringLiteral(" QiB1"),
         QStringLiteral("1QiB "),
         QStringLiteral("QiB1 "),
         QStringLiteral("1 QiB"),
         QStringLiteral("Q iB1"),
         QStringLiteral("1   QiB"),
         QStringLiteral("Q   iB1"),
         QStringLiteral("1..4 G"),
         QStringLiteral("G 1..4"),
         QStringLiteral("1iB"),
         QStringLiteral("iB1"),
         QStringLiteral("1Byte"),
         QStringLiteral("Byte1"),
         QStringLiteral("1024iK"),
         QStringLiteral("iK 2048")}) {
        const QByteArray body = QByteArrayLiteral("field=") + QUrl::toPercentEncoding(size);
        QTest::newRow(QString(QStringLiteral("filesize-invalid-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/fileSize") << headers << body << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1,5M"));
    query.addQueryItem(QStringLiteral("locale"), QStringLiteral("de"));
    QTest::newRow("filesize-locale-de-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5M"));
    query.addQueryItem(QStringLiteral("locale"), QStringLiteral("de"));
    QTest::newRow("filesize-locale-de-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    // disabled on MSVC because that shit still has problems with utf8 in 2018...
#ifndef _MSC_VER
    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1٫5M"));
    query.addQueryItem(QStringLiteral("locale"), QStringLiteral("ar"));
    QTest::newRow("filesize-locale-ar-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
#endif

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5M"));
    query.addQueryItem(QStringLiteral("locale"), QStringLiteral("ar"));
    QTest::newRow("filesize-locale-ar-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5TiB"));
    query.addQueryItem(QStringLiteral("option"), QStringLiteral("OnlyBinary"));
    QTest::newRow("filesize-onlybinary-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5TB"));
    query.addQueryItem(QStringLiteral("option"), QStringLiteral("OnlyBinary"));
    QTest::newRow("filesize-onlybinary-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5TB"));
    query.addQueryItem(QStringLiteral("option"), QStringLiteral("OnlyDecimal"));
    QTest::newRow("filesize-onlydecimyl-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("1.5TiB"));
    query.addQueryItem(QStringLiteral("option"), QStringLiteral("OnlyDecimal"));
    QTest::newRow("filesize-onlydecimyl-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2K"));
    query.addQueryItem(QStringLiteral("min"), QStringLiteral("1000"));
    QTest::newRow("filesize-min-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2K"));
    query.addQueryItem(QStringLiteral("min"), QStringLiteral("2048"));
    QTest::newRow("filesize-min-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2KiB"));
    query.addQueryItem(QStringLiteral("max"), QStringLiteral("2048"));
    QTest::newRow("filesize-max-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2KiB"));
    query.addQueryItem(QStringLiteral("max"), QStringLiteral("2047"));
    QTest::newRow("filesize-max-invalid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2KiB"));
    query.addQueryItem(QStringLiteral("min"), QStringLiteral("2048"));
    query.addQueryItem(QStringLiteral("max"), QStringLiteral("2048"));
    QTest::newRow("filesize-min-max-valid") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("0.5KiB"));
    query.addQueryItem(QStringLiteral("min"), QStringLiteral("1024"));
    query.addQueryItem(QStringLiteral("max"), QStringLiteral("2048"));
    QTest::newRow("filesize-min-max-invalid-1") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("3.5KiB"));
    query.addQueryItem(QStringLiteral("min"), QStringLiteral("1024"));
    query.addQueryItem(QStringLiteral("max"), QStringLiteral("2048"));
    QTest::newRow("filesize-min-max-invalid-2") << QStringLiteral("/fileSize") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorFileSize with return values

    const QMap<QString, QString> fileSizes({
                                               {QStringLiteral("1"), QStringLiteral("1")},
                                               {QStringLiteral("1B"), QStringLiteral("1")},
                                               {QStringLiteral("1K"), QStringLiteral("1000")},
                                               {QStringLiteral("1KiB"), QStringLiteral("1024")},
                                               {QStringLiteral("3.45K"), QStringLiteral("3450")},
                                               {QStringLiteral("3.45KiB"), QStringLiteral("3533")},
                                               {QStringLiteral("3456MB"), QStringLiteral("3456000000")},
                                               {QStringLiteral("3456MiB"), QStringLiteral("3623878656")},
                                               {QStringLiteral("4.321GB"), QStringLiteral("4321000000")},
                                               {QStringLiteral("4.321GiB"), QStringLiteral("4639638422")},
                                               {QStringLiteral("45.7890TB"), QStringLiteral("45789000000000")},
                                               {QStringLiteral("45.7890TiB"), QStringLiteral("50345537924235")},
                                               {QStringLiteral("123.456789PB"), QStringLiteral("123456789000000000")},
                                               {QStringLiteral("123.456789PiB"), QStringLiteral("138999987234189488")},
                                               {QStringLiteral("1.23EB"), QStringLiteral("1230000000000000000")},
                                               {QStringLiteral("1.23EiB"), QStringLiteral("1418093450666421760")},
                                               {QStringLiteral("2ZB"), QStringLiteral("2000000000000000000000.00")},
                                               {QStringLiteral("2ZiB"), QStringLiteral("2361183241434822606848.00")}
                                           });

    count = 0;
    auto fileSizesIt = fileSizes.constBegin();
    while (fileSizesIt != fileSizes.constEnd()) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), fileSizesIt.key());
        QTest::newRow(QString(QStringLiteral("filesize-return-value-%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/fileSizeValue") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << fileSizesIt.value().toUtf8();
        ++fileSizesIt;
        count++;
    }

    // **** Start testing ValidatorFilled *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("toll"));
    QTest::newRow("filled-valid") << QStringLiteral("/filled") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    QTest::newRow("filled-missing") << QStringLiteral("/filled") << headers << QByteArray() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("filled-invalid") << QStringLiteral("/filled") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;



    // **** Start testing ValidatorIn *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("zwei"));
    QTest::newRow("in-valid") << QStringLiteral("/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("vier"));
    QTest::newRow("in-invalid") << QStringLiteral("/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("in-empty") << QStringLiteral("/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    QTest::newRow("in-missing") << QStringLiteral("/in") << headers << QByteArray() << valid;



    // **** Start testing ValidatorInteger *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2345"));
    QTest::newRow("integer-valid01") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-2345"));
    QTest::newRow("integer-valid02") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QString::number(std::numeric_limits<int>::max()));
    QTest::newRow("integer-valid03") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-23a45 f"));
    QTest::newRow("integer-invalid01") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("a-23f45"));
    QTest::newRow("integer-invalid02") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QString::number(std::numeric_limits<qlonglong>::max()));
    QTest::newRow("integer-invalid03") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("integer-empty") << QStringLiteral("/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    QTest::newRow("integer-missing") << QStringLiteral("/integer") << headers << QByteArray() << valid;




    // **** Start testing ValidatorIp *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    QTest::newRow("ip-v4-valid") << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QList<QString> invalidIpv4({
                                         QStringLiteral("192.0.s.34"),
                                         QStringLiteral("192.0.43."),
                                         QStringLiteral("192.0.43"),
                                         QStringLiteral("300.167.168.5"),
                                         QStringLiteral("192.168.178.-5")
                                     });
    count = 0;
    for (const QString &ipv4 : invalidIpv4) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), ipv4);
        QTest::newRow(QString(QStringLiteral("ip-v4-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    const QList<QString> validIpv6({
                                       QStringLiteral("::"),
                                       QStringLiteral("::123"),
                                       QStringLiteral("::123:456"),
                                       QStringLiteral("::123:456:789:abc:def:6666"),
                                       QStringLiteral("::123:456:789:abc:def:6666:7"),
                                       QStringLiteral("123::456"),
                                       QStringLiteral("123::456:789"),
                                       QStringLiteral("123::456:789:abc"),
                                       QStringLiteral("123::456:789:abc:def"),
                                       QStringLiteral("123::456:789:abc:def:6"),
                                       QStringLiteral("123:456::789:abc:def:6666"),
                                       QStringLiteral("2001:0db8:85a3:08d3:1319:8a2e:0370:7344"),
                                       QStringLiteral("2001:0db8:0000:08d3:0000:8a2e:0070:7344"),
                                       QStringLiteral("2001:db8:0:8d3:0:8a2e:70:7344"),
                                       QStringLiteral("2001:0db8:0:0:0:0:1428:57ab"),
                                       QStringLiteral("2001:db8::1428:57ab"),
                                       QStringLiteral("2001:0db8:0:0:8d3:0:0:0"),
                                       QStringLiteral("2001:db8:0:0:8d3::"),
                                       QStringLiteral("2001:db8::8d3:0:0:0"),
                                       QStringLiteral("::ffff:127.0.0.1"),
                                       QStringLiteral("::ffff:7f00:1")
                                   });

    count = 0;
    for (const QString &ipv6 : validIpv6) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), ipv6);
        QTest::newRow(QString(QStringLiteral("ip-v6-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
        count++;
    }

    const QList<QString> invalidIpv6({
                                         QStringLiteral("2001:db8::8d3::"),
                                         QStringLiteral("2001:0db8:85a3:08d3:1319:8a2e:0370:7344:1234"),
                                         QStringLiteral(":::08d3:1319:8a2e:0370:7344"),
                                         QStringLiteral("2001:0db8:85a3:08d3:1319:8a2k:0370:7344"),
                                         QStringLiteral("127.0.0.1:1319:8a2k:0370:7344"),
                                         QStringLiteral("2001::0db8:85a3:08d3::1319:8a2k:0370:7344"),
                                         QStringLiteral("2001::0DB8:85A3:08D3::1319:8a2k:0370:7344"),
                                         QStringLiteral(":::")
                                     });
    count = 0;
    for (const QString &ipv6 : invalidIpv6) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), ipv6);
        QTest::newRow(QString(QStringLiteral("ip-v6-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv4Only"));
    QTest::newRow("ip-ipv4only-valid") << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv4Only"));
    QTest::newRow("ip-ipv4only-invalid") << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv6Only"));
    QTest::newRow("ip-ipv6only-valid") << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv6Only"));
    QTest::newRow("ip-ipv6only-invalid") << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoPrivateRange"));
    QTest::newRow("ip-noprivate-valid00")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoPrivateRange"));
    QTest::newRow("ip-noprivate-valid01")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QList<QString> invalidIpNoPrivate({
                                                QStringLiteral("10.1.2.3"),
                                                QStringLiteral("172.21.158.56"),
                                                QStringLiteral("192.168.178.100"),
                                                QStringLiteral("169.254.254.254"),
                                                QStringLiteral("fe80::5652:697b:2531:a7ed"),
                                                QStringLiteral("fd00:26:5bf0:abd2:15ff:1adb:e8c4:8453")
                                            });
    count = 0;
    for (const QString &ip : invalidIpNoPrivate) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), ip);
        query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoPrivateRange"));
        QTest::newRow(qUtf8Printable(QStringLiteral("ip-noprivate-invalid0%1").arg(count)))
                << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoReservedRange"));
    QTest::newRow("ip-noreserved-valid00")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoReservedRange"));
    QTest::newRow("ip-noreserved-valid01")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QList<QString> invalidIpNoReserved({
                                                 QStringLiteral("0.1.2.3"),
                                                 QStringLiteral("127.0.0.1"),
                                                 QStringLiteral("100.88.5.89"),
                                                 QStringLiteral("192.0.0.56"),
                                                 QStringLiteral("192.0.2.165"),
                                                 QStringLiteral("192.88.99.67"),
                                                 QStringLiteral("198.18.5.85"),
                                                 QStringLiteral("198.51.100.33"),
                                                 QStringLiteral("203.0.113.97"),
                                                 QStringLiteral("250.240.230.230"),
                                                 QStringLiteral("255.255.255.255"),
                                                 QStringLiteral("::"),
                                                 QStringLiteral("::1"),
                                                 QStringLiteral("0000:0000:0000:0000:0000:ffff:1234:abcd"),
                                                 QStringLiteral("0100:0000:0000:0000:1234:5678:9abc:def0"),
                                                 QStringLiteral("64:ff9b::95.4.66.32"),
                                                 QStringLiteral("2001:0000:1234:5678:90ab:cdef:1234:5678"),
                                                 QStringLiteral("2001:0010:0000:9876:abcd:5432:0000:a5b4"),
                                                 QStringLiteral("2001:0020:0000:9876:abcd:5432:0000:a5b4"),
                                                 QStringLiteral("2001:0db8:5b8e:6b5c:cdab:8546:abde:abdf"),
                                                 QStringLiteral("2002:fd4b:5b8e:6b5c:cdab:8546:abde:abdf")
                                             });
    count = 0;
    for (const QString &ip : invalidIpNoReserved) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), ip);
        query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoReservedRange"));
        QTest::newRow(qUtf8Printable(QStringLiteral("ip-noreserved-invalid0%1").arg(count)))
                << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-valid00")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-valid01")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("229.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-invalid00")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("ff02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-invalid01")  << QStringLiteral("/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;




    // **** Start testing ValidatorJson *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":2e%2B6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\",\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\",\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"));
    QTest::newRow("json-valid") << QStringLiteral("/json") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":2e 6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\",\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\",\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"));
    QTest::newRow("json-invalid") << QStringLiteral("/json") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;



    // **** Start testing ValidatorMax *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("max-sint-empty") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("max-sint-valid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("max-sint-invalid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15.912037"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghij"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("max-validationdataerror") << QStringLiteral("/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;





    // **** Start testing ValidatorMin *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("min-sint-empty") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("min-sint-valid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("min-sint-invalid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("min-uint-valid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("min-uint-invalid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15.912037"));
    QTest::newRow("min-float-valid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("min-float-invalid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijklmnop"));
    QTest::newRow("min-string-valid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdef"));
    QTest::newRow("min-string-invalid") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("min-validationdataerror") << QStringLiteral("/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;




    // **** Start testing ValidatorNotIn *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("fünf"));
    QTest::newRow("notin-valid") << QStringLiteral("/notIn") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("vier"));
    QTest::newRow("notin-invalid") << QStringLiteral("/notIn") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorNumeric *****

    const QList<QString> validNumerics({QStringLiteral("23"), QStringLiteral("-3465"), QStringLiteral("23.45"), QStringLiteral("-3456.32453245"), QStringLiteral("23.345345e15"), QStringLiteral("-1.23e4")});

    count = 0;
    for (const QString &num : validNumerics) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), num);
        QTest::newRow(qUtf8Printable(QStringLiteral("numeric-valid0%1").arg(count)))
                << QStringLiteral("/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;
        count++;
    }

    const QList<QString> invalidNumerics({QStringLiteral("2s3"), QStringLiteral("-a3465"), QStringLiteral("23:45"), QStringLiteral("-3456:32453245"), QStringLiteral("23.345345c15"), QStringLiteral("-1.23D4")});

    count = 0;
    for (const QString &num : invalidNumerics) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), num);
        QTest::newRow(qUtf8Printable(QStringLiteral("numeric-invalid0%1").arg(count)))
                << QStringLiteral("/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("numeric-empty") << QStringLiteral("/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;


    // **** Start testing ValidatorPresent *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("present-valid") << QStringLiteral("/present") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfasdf"));
    QTest::newRow("present-invalid") << QStringLiteral("/present") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorPwQuality
#ifdef CUTELYST_VALIDATOR_WITH_PWQUALITY
    const QList<QString> invalidPws({
                                        QStringLiteral("ovkaCPa"), // too short, lower than 8
                                        QStringLiteral("password"), // dictionary
                                        QStringLiteral("aceg1234") // score too low
                                    });
    count = 0;
    for (const QString &pw : invalidPws) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), pw);
        QTest::newRow(qUtf8Printable(QStringLiteral("pwquality-invalid0%1").arg(count)))
                << QStringLiteral("/pwQuality") << headers << query.toString(QUrl::FullyEncoded).toUtf8() << invalid;
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("niK3sd2eHAm@M0vZ!8sd$uJv?4AYlDaP6"));
    QTest::newRow("pwquality-valid") << QStringLiteral("/pwQuality") << headers << query.toString(QUrl::FullyEncoded).toUtf8() << valid;
#endif


    // **** Start testing ValidatorRegex *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("08/12/1985"));
    QTest::newRow("regex-valid") << QStringLiteral("/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("8/2/85"));
    QTest::newRow("regex-invalid") << QStringLiteral("/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("regex-empty") << QStringLiteral("/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;


    // **** Start testing ValidatorRequired *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("08/12/1985"));
    QTest::newRow("required-valid") << QStringLiteral("/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("required-empty") << QStringLiteral("/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("08/12/1985"));
    QTest::newRow("required-missing") << QStringLiteral("/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;




    // **** Start testing ValidatorRequiredIf *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-valid00") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("adfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid01") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid02") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid03") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("eins"));
    QTest::newRow("requiredif-valid04") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-invalid00") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-invalid01") << QStringLiteral("/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;



    // **** Start testing ValidatorRequiredIfStash *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("adsf"));
    QTest::newRow("requiredifstash-valid01") << QStringLiteral("/requiredIfStashMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("adsf"));
    QTest::newRow("requiredifstash-valid02") << QStringLiteral("/requiredIfStashNotMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("adsf"));
    QTest::newRow("requiredifstash-invalid03") << QStringLiteral("/requiredIfStashNotMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("adsf"));
    QTest::newRow("requiredifstash-invalid") << QStringLiteral("/requiredIfStashMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    // **** Start testing ValidatorRequiredUnless *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredunless-valid00") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-valid01") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredunless-valid02") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("zwei"));
    QTest::newRow("requiredunless-valid03") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-invalid00") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-invalid01") << QStringLiteral("/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorRequiredUnlessStash *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdf"));
    QTest::newRow("requiredunlessstash-valid00") << QStringLiteral("/requiredUnlessStashMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdf"));
    QTest::newRow("requiredunlessstash-valid01") << QStringLiteral("/requiredUnlessStashMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdf"));
    QTest::newRow("requiredunlessstash-valid02") << QStringLiteral("/requiredUnlessStashNotMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdf"));
    QTest::newRow("requiredunlessstash-invalid00") << QStringLiteral("/requiredUnlessStashNotMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("%20"));
    QTest::newRow("requiredunlessstash-invalid01") << QStringLiteral("/requiredUnlessStashNotMatch") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorRequiredWith *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid00") << QStringLiteral("/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid01") << QStringLiteral("/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid02") << QStringLiteral("/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-invalid00") << QStringLiteral("/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-invalid01") << QStringLiteral("/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;



    // **** Start testing ValidatorRequiredWithAll *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid00") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid01") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid02") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid03") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-invalid00") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-invalid01") << QStringLiteral("/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;



    // **** Start testing ValidatorRequiredWithout *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithout-valid00") << QStringLiteral("/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithout-valid01") << QStringLiteral("/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("requiredwithout-invalid00") << QStringLiteral("/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfasdf"));
    QTest::newRow("requiredwithout-invalid01") << QStringLiteral("/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfasdf"));
    QTest::newRow("requiredwithout-invalid02") << QStringLiteral("/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorRequiredWithoutAll *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid00") << QStringLiteral("/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid01") << QStringLiteral("/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid02") << QStringLiteral("/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    QTest::newRow("requiredwithoutall-invalid00") << QStringLiteral("/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-invalid01") << QStringLiteral("/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    // **** Start testing ValidatorSame *****
    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlklasdf"));
    QTest::newRow("same-valid") << QStringLiteral("/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlkla"));
    QTest::newRow("same-invalid") << QStringLiteral("/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlkla"));
    QTest::newRow("same-empty") << QStringLiteral("/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;


    // **** Start testing ValidatorSize *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("size-sint-empty") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10"));
    QTest::newRow("size-sint-valid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("size-sint-invalid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10"));
    QTest::newRow("size-uint-valid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("size-uint-invalid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10.0"));
    QTest::newRow("size-float-valid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("size-flost-invalid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghij"));
    QTest::newRow("size-string-valid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdef"));
    QTest::newRow("size-string-invalid") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("size-validationdataerror") << QStringLiteral("/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << validationDataError;



    // **** Start testing ValidatorTime *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(QStringLiteral("time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/time?field=") + QTime::currentTime().toString(df) << headers << QByteArray() << valid;
        count++;
    }

    QTest::newRow("time-invalid") << QStringLiteral("/time?field=123456789") << headers << QByteArray() << invalid;

    QTest::newRow("time-empty") << QStringLiteral("/time?field=%20") << headers << QByteArray() << valid;

    QTest::newRow("time-format-valid") << QStringLiteral("/timeFormat?field=") + QTime::currentTime().toString(QStringLiteral("m:hh")) << headers << QByteArray() << valid;

    QTest::newRow("time-format-invalid") << QStringLiteral("/timeFormat?field=") + QTime::currentTime().toString(QStringLiteral("m:AP")) << headers << QByteArray() << invalid;



    // **** Start testing ValidatorUrl*****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    QTest::newRow("url-valid00") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user"));
    QTest::newRow("url-valid01") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("user"));
    QTest::newRow("url-valid02") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("file:///home/user/test.txt"));
    QTest::newRow("url-valid03") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("url-empty") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoRelative"));
    QTest::newRow("url-norelative-valid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoRelative"));
    QTest::newRow("url-norelative-invalid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-valid00") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user/test.txt"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-valid01") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("file:///home/user/test.txt"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-invalid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("schemes"), QStringLiteral("HTTP,https"));
    QTest::newRow("url-scheme-valid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("ftp://www.example.org"));
    query.addQueryItem(QStringLiteral("schemes"), QStringLiteral("HTTP,https"));
    QTest::newRow("url-scheme-invalid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("WebsiteOnly"));
    QTest::newRow("url-websiteonly-valid") << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << valid;

    const QStringList invalidWebsiteUrls({QStringLiteral("ftp://www.example.org"), QStringLiteral("file:///home/user/test.txt"), QStringLiteral("/home/user")});
    count = 0;
    for (const QString &invalidWebsite : invalidWebsiteUrls) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), invalidWebsite);
        query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("WebsiteOnly"));
        QTest::newRow(qUtf8Printable(QStringLiteral("url-websiteonly-invalid0%1").arg(count)))
                << QStringLiteral("/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << invalid;
        count++;
    }


}

QTEST_MAIN(TestValidator)

#include "testvalidator.moc"

#endif //VALIDATORTEST_H
