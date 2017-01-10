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

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>
#include <Cutelyst/Plugins/Utils/Validator/Validator>
#include <Cutelyst/Plugins/Utils/Validator/Validators>

using namespace Cutelyst;

class TestValidator : public CoverageObject
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


class ValidatorTest : public Controller
{
    Q_OBJECT
public:
    ValidatorTest(QObject *parent) : Controller(parent) {}

    // ***** Endpoint for ValidatorAccepted ******
    C_ATTR(accepted, :Local :AutoArgs)
    void accepted(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorAccepted(QStringLiteral("accepted_field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with QDate ******
    C_ATTR(afterDate, :Local :AutoArgs)
    void afterDate(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QDate::currentDate());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with QTime ******
    C_ATTR(afterTime, :Local :AutoArgs)
    void afterTime(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QTime(12, 0));
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with QDateTime ******
    C_ATTR(afterDateTime, :Local :AutoArgs)
    void afterDateTime(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QDateTime::currentDateTime());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with custom format ******
    C_ATTR(afterFormat, :Local :AutoArgs)
    void afterFormat(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QDateTime::currentDateTime());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        va->setInputFormat(QStringLiteral("yyyy d MM HH:mm"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data ******
    C_ATTR(afterInvalidValidationData, :Local :AutoArgs)
    void afterInvalidValidationData(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QDate());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAfter with invalid validation data 2 ******
    C_ATTR(afterInvalidValidationData2, :Local :AutoArgs)
    void afterInvalidValidationData2(Context *c) {
        Validator v(c);
        ValidatorAfter *va = new ValidatorAfter(QStringLiteral("after_field"), QStringLiteral("schiet"));
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAlpha ******
    C_ATTR(alpha, :Local :AutoArgs)
    void alpha(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorAlpha(QStringLiteral("alpha_field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAlphaDash ******
    C_ATTR(alphaDash, :Local :AutoArgs)
    void alphaDash(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorAlphaDash(QStringLiteral("alphadash_field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorAlphaNum ******
    C_ATTR(alphaNum, :Local :AutoArgs)
    void alphaNum(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorAlphaNum(QStringLiteral("alphanum_field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with QDate ******
    C_ATTR(beforeDate, :Local :AutoArgs)
    void beforeDate(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QDate::currentDate());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with QTime ******
    C_ATTR(beforeTime, :Local :AutoArgs)
    void beforeTime(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QTime(12, 0));
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with QDateTime ******
    C_ATTR(beforeDateTime, :Local :AutoArgs)
    void beforeDateTime(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QDateTime::currentDateTime());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with custom format ******
    C_ATTR(beforeFormat, :Local :AutoArgs)
    void beforeFormat(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QDateTime::currentDateTime());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomParsingError(QStringLiteral("parsingerror"));
        va->setInputFormat(QStringLiteral("yyyy d MM HH:mm"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data ******
    C_ATTR(beforeInvalidValidationData, :Local :AutoArgs)
    void beforeInvalidValidationData(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QDate());
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBefore with invalid validation data 2 ******
    C_ATTR(beforeInvalidValidationData2, :Local :AutoArgs)
    void beforeInvalidValidationData2(Context *c) {
        Validator v(c);
        ValidatorBefore *va = new ValidatorBefore(QStringLiteral("before_field"), QStringLiteral("schiet"));
        va->setCustomError(QStringLiteral("invalid"));
        va->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBetween with int ******
    C_ATTR(betweenInt, :Local :AutoArgs)
    void betweenInt(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorBetween(QStringLiteral("between_field"), QMetaType::Int, -10, 10, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBetween with uint ******
    C_ATTR(betweenUint, :Local :AutoArgs)
    void betweenUint(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorBetween(QStringLiteral("between_field"), QMetaType::UInt, 10, 20, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBetween with float ******
    C_ATTR(betweenFloat, :Local :AutoArgs)
    void betweenFloat(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorBetween(QStringLiteral("between_field"), QMetaType::Float, -10.0, 10.0, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBetween with string ******
    C_ATTR(betweenString, :Local :AutoArgs)
    void betweenString(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorBetween(QStringLiteral("between_field"), QMetaType::QString, 5, 10, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorBoolean ******
    C_ATTR(boolean, :Local :AutoArgs)
    void boolean(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorBoolean(QStringLiteral("boolean_field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorConfirmed ******
    C_ATTR(confirmed, :Local :AutoArgs)
    void confirmed(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorConfirmed(QStringLiteral("pass"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDate with standard formats ******
    C_ATTR(date, :Local :AutoArgs)
    void date(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDate(QStringLiteral("field"), QString(), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDate with custom format ******
    C_ATTR(dateFormat, :Local :AutoArgs)
    void dateFormat(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDate(QStringLiteral("field"), QStringLiteral("yyyy d MM"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDateTime with standard formats ******
    C_ATTR(dateTime, :Local :AutoArgs)
    void dateTime(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDateTime(QStringLiteral("field"), QString(), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDateTime with custom format ******
    C_ATTR(dateTimeFormat, :Local :AutoArgs)
    void dateTimeFormat(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDateTime(QStringLiteral("field"), QStringLiteral("yyyy d MM mm:HH" ), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDifferent ******
    C_ATTR(different, :Local :AutoArgs)
    void different(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDifferent(QStringLiteral("field"), QStringLiteral("other"), QString(), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDigits without exact length ******
    C_ATTR(digits, :Local :AutoArgs)
    void digits(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDigits(QStringLiteral("field"), -1, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDigits with exact length ******
    C_ATTR(digitsLength, :Local :AutoArgs)
    void digitsLength(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDigits(QStringLiteral("field"), 10, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorDigitsBetween ******
    C_ATTR(digitsBetween, :Local :AutoArgs)
    void digitsBetween(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorDigitsBetween(QStringLiteral("field"), 5, 10, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorEmail ******
    C_ATTR(email, :Local :AutoArgs)
    void email(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorEmail(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorFilled ******
    C_ATTR(filled, :Local :AutoArgs)
    void filled(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorFilled(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorIn ******
    C_ATTR(in, :Local :AutoArgs)
    void in(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorIn(QStringLiteral("field"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei"), QStringLiteral("drei")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorInteger ******
    C_ATTR(integer, :Local :AutoArgs)
    void integer(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorInteger(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorIp ******
    C_ATTR(ip, :Local :AutoArgs)
    void ip(Context *c) {
        Validator v(c);
        ValidatorIp::Constraints constraints = ValidatorIp::NoConstraint;

        if (!c->request()->parameters().value(QStringLiteral("constraints")).isEmpty()) {
            QStringList cons = c->request()->parameters().value(QStringLiteral("constraints")).split(QStringLiteral(","));
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

        v.addValidator(new ValidatorIp(QStringLiteral("field"), constraints, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }


    // ***** Endpoint for ValidatorJson ******
    C_ATTR(json, :Local :AutoArgs)
    void json(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorJson(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }


    // ***** Endpoint for ValidatorMax ******
    C_ATTR(max, :Local :AutoArgs)
    void max(Context *c) {
        Validator v(c);
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->parameters().value(QStringLiteral("type")).isEmpty()) {
            QString t = c->request()->parameters().value(QStringLiteral("type"));
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
        ValidatorMax *vm = new ValidatorMax(QStringLiteral("field"), type, 10.0, QString(), QStringLiteral("invalid"));
        vm->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(vm);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorMin ******
    C_ATTR(min, :Local :AutoArgs)
    void min(Context *c) {
        Validator v(c);
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->parameters().value(QStringLiteral("type")).isEmpty()) {
            QString t = c->request()->parameters().value(QStringLiteral("type"));
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
        ValidatorMin *vm = new ValidatorMin(QStringLiteral("field"), type, 10.0, QString(), QStringLiteral("invalid"));
        vm->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(vm);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorNotIn ******
    C_ATTR(notIn, :Local :AutoArgs)
    void notIn(Context *c) {
        Validator v(c);
        const QStringList values({QStringLiteral("eins"),QStringLiteral("zwei"),QStringLiteral("drei"),QStringLiteral("vier")});
        ValidatorNotIn *va = new ValidatorNotIn(QStringLiteral("field"), values, QString(), QStringLiteral("invalid"));
        va->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(va);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }


    // ***** Endpoint for ValidatorNumeric ******
    C_ATTR(numeric, :Local :AutoArgs)
    void numeric(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorNumeric(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }


    // ***** Endpoint for ValidatorPresent ******
    C_ATTR(present, :Local :AutoArgs)
    void present(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorPresent(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }


    // ***** Endpoint for ValidatorNumeric ******
    C_ATTR(regex, :Local :AutoArgs)
    void regex(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRegularExpression(QStringLiteral("field"), QRegularExpression(QStringLiteral("^(\\d\\d)/(\\d\\d)/(\\d\\d\\d\\d)$")), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequired ******
    C_ATTR(required, :Local :AutoArgs)
    void required(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequired(QStringLiteral("field"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredIf ******
    C_ATTR(requiredIf, :Local :AutoArgs)
    void requiredIf(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredIf(QStringLiteral("field"), QStringLiteral("field2"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredUnless ******
    C_ATTR(requiredUnless, :Local :AutoArgs)
    void requiredUnless(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredUnless(QStringLiteral("field"), QStringLiteral("field2"), QStringList({QStringLiteral("eins"), QStringLiteral("zwei")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredWith ******
    C_ATTR(requiredWith, :Local :AutoArgs)
    void requiredWith(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredWith(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredWithAll ******
    C_ATTR(requiredWithAll, :Local :AutoArgs)
    void requiredWithAll(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredWithAll(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3"), QStringLiteral("field4")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredWithout ******
    C_ATTR(requiredWithout, :Local :AutoArgs)
    void requiredWithout(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredWithout(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorRequiredWithoutAll ******
    C_ATTR(requiredWithoutAll, :Local :AutoArgs)
    void requiredWithoutAll(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorRequiredWithoutAll(QStringLiteral("field"), QStringList({QStringLiteral("field2"), QStringLiteral("field3")}), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorSame ******
    C_ATTR(same, :Local :AutoArgs)
    void same(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorSame(QStringLiteral("field"), QStringLiteral("other"), QString(), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorSize ******
    C_ATTR(size, :Local :AutoArgs)
    void size(Context *c) {
        Validator v(c);
        QMetaType::Type type = QMetaType::UnknownType;

        if (!c->request()->parameters().value(QStringLiteral("type")).isEmpty()) {
            QString t = c->request()->parameters().value(QStringLiteral("type"));
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
        ValidatorSize *vm = new ValidatorSize(QStringLiteral("field"), type, 10.0, QString(), QStringLiteral("invalid"));
        vm->setCustomValidationDataError(QStringLiteral("validationdataerror"));
        v.addValidator(vm);
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorTime
    C_ATTR(time, :Local :AutoArgs)
    void time(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorTime(QStringLiteral("field"), QString(), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorTime with custom format ******
    C_ATTR(timeFormat, :Local :AutoArgs)
    void timeFormat(Context *c) {
        Validator v(c);
        v.addValidator(new ValidatorTime(QStringLiteral("field"), QStringLiteral("m:hh"), QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
        }
    }

    // ***** Endpoint for ValidatorUrl
    C_ATTR(url, :Local :AutoArgs)
    void url(Context *c) {
        ValidatorUrl::Constraints constraints = ValidatorUrl::NoConstraint;
        QStringList schemes;
        QString scheme = c->request()->parameters().value(QStringLiteral("schemes"));
        if (!scheme.isEmpty()) {
            schemes = scheme.split(QStringLiteral(","));
        }

        if (!c->request()->parameters().value(QStringLiteral("constraints")).isEmpty()) {
            QStringList cons = c->request()->parameters().value(QStringLiteral("constraints")).split(QStringLiteral(","));
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

        Validator v(c);
        v.addValidator(new ValidatorUrl(QStringLiteral("field"), constraints, schemes, QString(), QStringLiteral("invalid")));
        if (v.validate()) {
            c->response()->setBody(QByteArrayLiteral("valid"));
        } else {
            c->response()->setBody(v.errorStrings().first().toString());
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

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(QStringLiteral("POST"),
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

    Headers headers;
    headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
    QUrlQuery query;

    const QList<Qt::DateFormat> dateFormats({Qt::ISODate, Qt::RFC2822Date, Qt::TextDate});

    const QList<QString> acceptedValues({QStringLiteral("yes"), QStringLiteral("on"), QStringLiteral("1"), QStringLiteral("true")});


    // **** Start testing ValidatorAccepted *****

    int count = 0;
    for (const QString &val : acceptedValues) {
        QTest::newRow(QString(QStringLiteral("accepted-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/accepted?accepted_field=") + val << headers << QByteArray() << QByteArrayLiteral("valid");
        count++;
    }

    QTest::newRow("accepted-invalid") << QStringLiteral("/validator/test/accepted?accepted_field=asdf") << headers << QByteArray()
                                      << QByteArrayLiteral("invalid");

    QTest::newRow("accepted-empty") << QStringLiteral("/validator/test/accepted?accepted_field=") << headers << QByteArray()
                                    << QByteArrayLiteral("invalid");

    QTest::newRow("accepted-missing") << QStringLiteral("/validator/test/accepted") << headers << QByteArray()
                                      << QByteArrayLiteral("invalid");




    // **** Start testing ValidatorAfter *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("after-date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDate(1999, 9, 9).toString(df));
        QTest::newRow(QString(QStringLiteral("after-date-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("after-date-parsingerror") << QStringLiteral("/validator/test/afterDate?after_field=lökjasdfjh") << headers << QByteArray()
                                             << QByteArrayLiteral("parsingerror");

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QTime(13, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("after-time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QTime(11, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("after-time-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("after-time-parsingerror") << QStringLiteral("/validator/test/afterTime?after_field=kjnagiuh") << headers << QByteArray()
                                             << QByteArrayLiteral("parsingerror");


    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDateTime::currentDateTime().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("after-datetime-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterDateTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("after_field"), QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df));
        QTest::newRow(QString(QStringLiteral("after-datetime-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/afterDateTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("after-datetime-parsingerror") << QStringLiteral("/validator/test/afterDateTime?after_field=aio,aü") << headers << QByteArray()
                                                 << QByteArrayLiteral("parsingerror");


    QTest::newRow("after-invalidvalidationdata00") << QStringLiteral("/validator/test/afterInvalidValidationData?after_field=") + QDate::currentDate().addDays(2).toString(Qt::ISODate)
                                                   << headers << QByteArray() << QByteArrayLiteral("validationdataerror");

    QTest::newRow("after-invalidvalidationdata01") << QStringLiteral("/validator/test/afterInvalidValidationData2?after_field=") + QDate::currentDate().addDays(2).toString(Qt::ISODate)
                                                   << headers << QByteArray() << QByteArrayLiteral("validationdataerror");

    QTest::newRow("after-format-valid") << QStringLiteral("/validator/test/afterFormat?after_field=") + QDateTime::currentDateTime().addDays(2).toString(QStringLiteral("yyyy d MM HH:mm"))
                                        << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("after-format-invalid") << QStringLiteral("/validator/test/afterFormat?after_field=") + QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(QStringLiteral("yyyy d MM HH:mm"))
                                          << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("after-format-parsingerror") << QStringLiteral("/validator/test/afterFormat?after_field=23590uj09") << headers << QByteArray() << QByteArrayLiteral("parsingerror");




    // **** Start testing ValidatorAlpha *****

    QTest::newRow("alpha-valid") << QStringLiteral("/validator/test/alpha?alpha_field=adsfä") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alpha-invalid") << QStringLiteral("/validator/test/alpha?alpha_field=ad_sf 2ä!") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("alpha-empty") << QStringLiteral("/validator/test/alpha?alpha_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alpha-missing") << QStringLiteral("/validator/test/alpha") << headers << QByteArray() << QByteArrayLiteral("valid");





    // **** Start testing ValidatorAlphaDash *****

    QTest::newRow("alphadash-valid") << QStringLiteral("/validator/test/alphaDash?alphadash_field=ads2-fä_3") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alphadash-invalid") << QStringLiteral("/validator/test/alphaDash?alphadash_field=ad sf_2ä!") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("alphadash-empty") << QStringLiteral("/validator/test/alphaDash?alphadash_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alphadash-missing") << QStringLiteral("/validator/test/alphaDash") << headers << QByteArray() << QByteArrayLiteral("valid");



    // **** Start testing ValidatorAlphaNum *****

    QTest::newRow("alphanum-valid") << QStringLiteral("/validator/test/alphaNum?alphanum_field=ads2fä3") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alphanum-invalid") << QStringLiteral("/validator/test/alphaNum?alphanum_field=ad sf_2ä!") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("alphanum-empty") << QStringLiteral("/validator/test/alphaNum?alphanum_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("alphanum-missing") << QStringLiteral("/validator/test/alphaNum") << headers << QByteArray() << QByteArrayLiteral("valid");




    // **** Start testing ValidatorBefore *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDate(1999, 9, 9).toString(df));
        QTest::newRow(QString(QStringLiteral("before-date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDate::currentDate().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("before-date-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeDate?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("before-date-parsingerror") << QStringLiteral("/validator/test/beforeDate?before_field=lökjasdfjh") << headers << QByteArray()
                                             << QByteArrayLiteral("parsingerror");

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QTime(11, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("before-time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QTime(13, 0).toString(df));
        QTest::newRow(QString(QStringLiteral("before-time-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("before-time-parsingerror") << QStringLiteral("/validator/test/beforeTime?before_field=kjnagiuh") << headers << QByteArray()
                                             << QByteArrayLiteral("parsingerror");


    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(df));
        QTest::newRow(QString(QStringLiteral("before-datetime-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeDateTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("valid");


        query.clear();
        query.addQueryItem(QStringLiteral("before_field"), QDateTime::currentDateTime().addDays(2).toString(df));
        QTest::newRow(QString(QStringLiteral("before-datetime-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/beforeDateTime?") + query.toString(QUrl::FullyEncoded) << headers << QByteArray() << QByteArrayLiteral("invalid");

        count++;
    }

    QTest::newRow("before-datetime-parsingerror") << QStringLiteral("/validator/test/beforeDateTime?before_field=aio,aü") << headers << QByteArray()
                                                 << QByteArrayLiteral("parsingerror");


    QTest::newRow("before-invalidvalidationdata00") << QStringLiteral("/validator/test/beforeInvalidValidationData?before_field=") + QDate(1999, 9, 9).toString(Qt::ISODate)
                                                   << headers << QByteArray() << QByteArrayLiteral("validationdataerror");

    QTest::newRow("before-invalidvalidationdata01") << QStringLiteral("/validator/test/beforeInvalidValidationData2?before_field=") + QDate(1999, 9, 9).toString(Qt::ISODate)
                                                   << headers << QByteArray() << QByteArrayLiteral("validationdataerror");

    QTest::newRow("before-format-valid") << QStringLiteral("/validator/test/beforeFormat?before_field=") + QDateTime(QDate(1999, 9, 9), QTime(19, 19)).toString(QStringLiteral("yyyy d MM HH:mm"))
                                        << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("before-format-invalid") << QStringLiteral("/validator/test/beforeFormat?before_field=") + QDateTime::currentDateTime().addDays(2).toString(QStringLiteral("yyyy d MM HH:mm"))
                                          << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("before-format-parsingerror") << QStringLiteral("/validator/test/beforeFormat?before_field=23590uj09") << headers << QByteArray() << QByteArrayLiteral("parsingerror");






    // **** Start testing ValidatorBetween *****

    QTest::newRow("between-int-valid") << QStringLiteral("/validator/test/betweenInt?between_field=0") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-int-invalid-lower") << QStringLiteral("/validator/test/betweenInt?between_field=-15") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-int-invalid-greater") << QStringLiteral("/validator/test/betweenInt?between_field=15") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-int-empty") << QStringLiteral("/validator/test/betweenInt?between_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-uint-valid") << QStringLiteral("/validator/test/betweenUint?between_field=15") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-uint-invalid-lower") << QStringLiteral("/validator/test/betweenUint?between_field=5") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-uint-invalid-greater") << QStringLiteral("/validator/test/betweenUint?between_field=25") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-uint-empty") << QStringLiteral("/validator/test/betweenUint?between_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-float-valid") << QStringLiteral("/validator/test/betweenFloat?between_field=0.0") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-float-invalid-lower") << QStringLiteral("/validator/test/betweenFloat?between_field=-15.2") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-float-invalid-greater") << QStringLiteral("/validator/test/betweenFloat?between_field=15.2") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-float-empty") << QStringLiteral("/validator/test/betweenFloat?between_field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-string-valid") << QStringLiteral("/validator/test/betweenString?between_field=abcdefg") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("between-string-invalid-lower") << QStringLiteral("/validator/test/betweenString?between_field=abc") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-string-invalid-greater") << QStringLiteral("/validator/test/betweenString?between_field=abcdefghijklmn") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("between-string-empty") << QStringLiteral("/validator/test/betweenString?between_field=") << headers << QByteArray() << QByteArrayLiteral("valid");



    // **** Start testing ValidatorBoolean *****

    QList<QString> booleanValidList({QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("on"), QStringLiteral("off")});

    for (const QString &bv : booleanValidList) {
        QTest::newRow(QString(QStringLiteral("boolean-valid-%1").arg(bv)).toUtf8().constData())
                << QStringLiteral("/validator/test/boolean?boolean_field=") + bv << headers << QByteArray() << QByteArrayLiteral("valid");
    }

    QList<QString> booleanInvalidList({QStringLiteral("2"), QStringLiteral("-45"), QStringLiteral("wahr"), QStringLiteral("unwahr"), QStringLiteral("ja")});

    for (const QString &bv : booleanInvalidList) {
        QTest::newRow(QString(QStringLiteral("boolean-invalid-%1").arg(bv)).toUtf8().constData())
                << QStringLiteral("/validator/test/boolean?boolean_field=") + bv << headers << QByteArray() << QByteArrayLiteral("invalid");
    }

    QTest::newRow("boolean-empty") << QStringLiteral("/validator/test/boolean?boolean_field=") << headers << QByteArray() << QByteArrayLiteral("valid");




    // **** Start testing ValidatorBoolean *****

    QTest::newRow("confirmed-valid") << QStringLiteral("/validator/test/confirmed?pass=abcdefg&pass_confirmation=abcdefg") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("confirmed-invalid") << QStringLiteral("/validator/test/confirmed?pass=abcdefg&pass_confirmation=hijklmn") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("confirmed-empty") << QStringLiteral("/validator/test/confirmed?pass&pass_confirmation=abcdefg") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("confirmed-missing-confirmation") << QStringLiteral("/validator/test/confirmed?pass=abcdefg") << headers << QByteArray() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorDate *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(QStringLiteral("date-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/date?field=") + QDate::currentDate().toString(df) << headers << QByteArray() << QByteArrayLiteral("valid");
        count++;
    }

    QTest::newRow("date-invalid") << QStringLiteral("/validator/test/date?field=123456789") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("date-empty") << QStringLiteral("/validator/test/date?field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("date-format-valid") << QStringLiteral("/validator/test/dateFormat?field=") + QDate::currentDate().toString(QStringLiteral("yyyy d MM")) << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("date-format-invalid") << QStringLiteral("/validator/test/dateFormat?field=") + QDate::currentDate().toString(QStringLiteral("MM yyyy d")) << headers << QByteArray() << QByteArrayLiteral("invalid");




    // **** Start testing ValidatorDateTime *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(QStringLiteral("datetime-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/dateTime?field=") + QDateTime::currentDateTime().toString(df) << headers << QByteArray() << QByteArrayLiteral("valid");
        count++;
    }

    QTest::newRow("datetime-invalid") << QStringLiteral("/validator/test/dateTime?field=123456789") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("datetime-empty") << QStringLiteral("/validator/test/dateTime?field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("datetime-format-valid") << QStringLiteral("/validator/test/dateTimeFormat?field=") + QDateTime::currentDateTime().toString(QStringLiteral("yyyy d MM mm:HH")) << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("datetime-format-invalid") << QStringLiteral("/validator/test/dateTimeFormat?field=") + QDateTime::currentDateTime().toString(QStringLiteral("MM mm yyyy HH d")) << headers << QByteArray() << QByteArrayLiteral("invalid");





    // **** Start testing ValidatorDifferent *****

    QTest::newRow("different-valid") << QStringLiteral("/validator/test/different?field=abcdefg&other=hijklmno") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("different-invalid") << QStringLiteral("/validator/test/different?field=abcdefg&other=abcdefg") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("different-empty") << QStringLiteral("/validator/test/different?field=&other=hijklmno") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("different-other-missing") << QStringLiteral("/validator/test/different?field=abcdefg") << headers << QByteArray() << QByteArrayLiteral("valid");




    // **** Start testing ValidatorDigits *****

    QTest::newRow("digits-valid") << QStringLiteral("/validator/test/digits?field=0123456") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("digits-invalid") << QStringLiteral("/validator/test/digits?field=01234asdf56") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("digits-empty") << QStringLiteral("/validator/test/digits?field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("digits-length-valid") << QStringLiteral("/validator/test/digitsLength?field=0123456789") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("digits-length-invalid") << QStringLiteral("/validator/test/digitsLength?field=012345") << headers << QByteArray() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorDigitsBetween *****

    QTest::newRow("digitsbetween-valid") << QStringLiteral("/validator/test/digitsBetween?field=0123456") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("digitsbetween-invalid") << QStringLiteral("/validator/test/digitsBetween?field=01234ad56") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("digitsbetween-empty") << QStringLiteral("/validator/test/digitsBetween?field=") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("digitsbetween-invalid-lower") << QStringLiteral("/validator/test/digitsBetween?field=0123") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("digitsbetween-invalid-greater") << QStringLiteral("/validator/test/digitsBetween?field=0123456789123") << headers << QByteArray() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorEmail *****

    const QList<QString> validEmailList({
                                            QStringLiteral("first.last@iana.org"),
                                            QStringLiteral("1234567890123456789012345678901234567890123456789012345678901234@iana.org"),
                                            QStringLiteral("\"first\\\"last\"@iana.org"),
                                            QStringLiteral("\"first@last\"@iana.org"),
                                            QStringLiteral("\"first\\\\last\"@iana.org"),
                                            QStringLiteral("x@x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x23456789.x2"),
                                            QStringLiteral("1234567890123456789012345678901234567890123456789012345678@12345678901234567890123456789012345678901234567890123456789.12345678901234567890123456789012345678901234567890123456789.123456789012345678901234567890123456789012345678901234567890123.iana.org"),
                                            QStringLiteral("first.last@[12.34.56.78]"),
                                            QStringLiteral("first.last@12.34.56.78"),
                                            QStringLiteral("first.last@[IPv6:::12.34.56.78]"),
                                            QStringLiteral("first.last@[IPv6:::b3:b4]"),
                                            QStringLiteral("first.last@[IPv6:::]"),
                                            QStringLiteral("\"first\\last\"@iana.org"),
                                            QStringLiteral("user%2Bmailbox@iana.org"),
                                            QStringLiteral("customer/department@iana.org"),
                                            QStringLiteral("customer/department=shipping@iana.org"),
                                            QStringLiteral("\"Doug \\\"Ace\\\" L.\"@iana.org"),
                                            QStringLiteral("%2B1~1%2B@iana.org"),
                                            QStringLiteral("{_test_}@iana.org"),
                                            QStringLiteral("\"[[ test ]]\"@iana.org"),
                                            QStringLiteral("\"test&#13;&#10; blah\"@iana.org"),
                                            QStringLiteral("(foo)cal(bar)@(baz)iamcal.com(quux)"),
                                            QStringLiteral("cal(woo(yay)hoopla)@iamcal.com"),
                                            QStringLiteral("cal(foo\\@bar)@iamcal.com"),
                                            QStringLiteral("cal(foo\\)bar)@iamcal.com"),
                                            QStringLiteral("first(Welcome to&#13;&#10; the (\"wonderful\" (!)) world&#13;&#10; of email)@iana.org"),
                                            QStringLiteral("pete(his account)@silly.test(his host)"),
                                            QStringLiteral("c@(Chris's host.)public.example"),
                                            QStringLiteral("_______@example.com")
                                        });
    count = 0;
    for (const QString &email : validEmailList) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), email);
        QTest::newRow(QString(QStringLiteral("email-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/email") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");
        count++;
    }

    const QList<QString> invalidEmailList({
                                             QStringLiteral("first@...........com"),
                                             QStringLiteral("first.last@sub.do,com"),
                                             QStringLiteral("first\\@last@iana.org"),
                                             QStringLiteral("first.last"),
                                             QStringLiteral(".first.last@iana.org"),
                                             QStringLiteral("first.last.@iana.org"),
                                             QStringLiteral("\"first\"last\"@iana.org"),
                                             QStringLiteral("\"\"\"@iana.org"),
                                             QStringLiteral("first\\\\@last@iana.org"),
                                             QStringLiteral("Doug\\ \\\"Ace\\\"\\ Lovell@iana.org"),
                                             QStringLiteral("()[]\\;:,><@iana.org"),
                                             QStringLiteral("test@."),
                                             QStringLiteral("test@[123.123.123.123"),
                                             QStringLiteral("test@123.123.123.123]")
                                         });
    count = 0;
    for (const QString &email : invalidEmailList) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), email);
        QTest::newRow(QString(QStringLiteral("email-invalid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/email") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("email-empty") << QStringLiteral("/validator/test/email") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");



    // **** Start testing ValidatorFilled *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("toll"));
    QTest::newRow("filled-valid") << QStringLiteral("/validator/test/filled") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    QTest::newRow("filled-missing") << QStringLiteral("/validator/test/filled") << headers << QByteArray() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("filled-invalid") << QStringLiteral("/validator/test/filled") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorIn *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("zwei"));
    QTest::newRow("in-valid") << QStringLiteral("/validator/test/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("vier"));
    QTest::newRow("in-invalid") << QStringLiteral("/validator/test/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("in-empty") << QStringLiteral("/validator/test/in") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    QTest::newRow("in-missing") << QStringLiteral("/validator/test/in") << headers << QByteArray() << QByteArrayLiteral("valid");



    // **** Start testing ValidatorInteger *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2345"));
    QTest::newRow("integer-valid01") << QStringLiteral("/validator/test/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-2345"));
    QTest::newRow("integer-valid02") << QStringLiteral("/validator/test/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-23a45 f"));
    QTest::newRow("integer-invalid01") << QStringLiteral("/validator/test/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("a-23f45"));
    QTest::newRow("integer-invalid02") << QStringLiteral("/validator/test/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("integer-empty") << QStringLiteral("/validator/test/integer") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    QTest::newRow("in-missing") << QStringLiteral("/validator/test/integer") << headers << QByteArray() << QByteArrayLiteral("valid");




    // **** Start testing ValidatorIp *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    QTest::newRow("ip-v4-valid") << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

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
                << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
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
                << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");
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
                << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv4Only"));
    QTest::newRow("ip-ipv4only-valid") << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv4Only"));
    QTest::newRow("ip-ipv4only-invalid") << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv6Only"));
    QTest::newRow("ip-ipv6only-valid") << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("IPv6Only"));
    QTest::newRow("ip-ipv6only-invalid") << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoPrivateRange"));
    QTest::newRow("ip-noprivate-valid00")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoPrivateRange"));
    QTest::newRow("ip-noprivate-valid01")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

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
                << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoReservedRange"));
    QTest::newRow("ip-noreserved-valid00")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoReservedRange"));
    QTest::newRow("ip-noreserved-valid01")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

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
                << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("192.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-valid00")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("2a02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-valid01")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("229.0.43.8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-invalid00")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("ff02:810d:22c0:1c8c:5900:83dc:83b6:9ed8"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoMultiCast"));
    QTest::newRow("ip-nomulticast-invalid01")  << QStringLiteral("/validator/test/ip") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");




    // **** Start testing ValidatorJson *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":2e%2B6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\",\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\",\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"));
    QTest::newRow("json-valid") << QStringLiteral("/validator/test/json") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("{\"Herausgeber\":\"Xema\",\"Nummer\":\"1234-5678-9012-3456\",\"Deckung\":2e 6,\"Waehrung\":\"EURO\",\"Inhaber\":{\"Name\":\"Mustermann\",\"Vorname\":\"Max\",\"maennlich\":true,\"Hobbys\":[\"Reiten\",\"Golfen\",\"Lesen\"],\"Alter\":42,\"Kinder\":[],\"Partner\":null}}"));
    QTest::newRow("json-invalid") << QStringLiteral("/validator/test/json") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorMax *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("max-sint-empty") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("max-sint-valid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("max-sint-invalid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15.912037"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghij"));
    QTest::newRow("max-uint-valid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("max-uint-invalid") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("max-validationdataerror") << QStringLiteral("/validator/test/max") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("validationdataerror");





    // **** Start testing ValidatorMin *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("min-sint-empty") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("min-sint-valid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("min-sint-invalid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15"));
    QTest::newRow("min-uint-valid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("min-uint-invalid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("15.912037"));
    QTest::newRow("min-float-valid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("min-float-invalid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijklmnop"));
    QTest::newRow("min-string-valid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdef"));
    QTest::newRow("min-string-invalid") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("min-validationdataerror") << QStringLiteral("/validator/test/min") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("validationdataerror");




    // **** Start testing ValidatorMin *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("fünf"));
    QTest::newRow("notin-valid") << QStringLiteral("/validator/test/notIn") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("vier"));
    QTest::newRow("notin-invalid") << QStringLiteral("/validator/test/notIn") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");


    // **** Start testing ValidatorMin *****

    const QList<QString> validNumerics({QStringLiteral("23"), QStringLiteral("-3465"), QStringLiteral("23.45"), QStringLiteral("-3456.32453245"), QStringLiteral("23.345345e15"), QStringLiteral("-1.23e4")});

    count = 0;
    for (const QString &num : validNumerics) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), num);
        QTest::newRow(qUtf8Printable(QStringLiteral("numeric-valid0%1").arg(count)))
                << QStringLiteral("/validator/test/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");
        count++;
    }

    const QList<QString> invalidNumerics({QStringLiteral("2s3"), QStringLiteral("-a3465"), QStringLiteral("23:45"), QStringLiteral("-3456:32453245"), QStringLiteral("23.345345c15"), QStringLiteral("-1.23D4")});

    count = 0;
    for (const QString &num : invalidNumerics) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), num);
        QTest::newRow(qUtf8Printable(QStringLiteral("numeric-invalid0%1").arg(count)))
                << QStringLiteral("/validator/test/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("numeric-empty") << QStringLiteral("/validator/test/numeric") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");


    // **** Start testing ValidatorPresent *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("present-valid") << QStringLiteral("/validator/test/present") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfasdf"));
    QTest::newRow("present-invalid") << QStringLiteral("/validator/test/present") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorRegex *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("08/12/1985"));
    QTest::newRow("regex-valid") << QStringLiteral("/validator/test/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("8/2/85"));
    QTest::newRow("regex-invalid") << QStringLiteral("/validator/test/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("regex-empty") << QStringLiteral("/validator/test/regex") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");


    // **** Start testing ValidatorRequired *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("08/12/1985"));
    QTest::newRow("required-valid") << QStringLiteral("/validator/test/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("required-empty") << QStringLiteral("/validator/test/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("08/12/1985"));
    QTest::newRow("required-missing") << QStringLiteral("/validator/test/required") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");




    // **** Start testing ValidatorRequiredIf *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-valid00") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("adfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid01") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid02") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredif-valid03") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("eins"));
    QTest::newRow("requiredif-valid04") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-invalid00") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredif-invalid01") << QStringLiteral("/validator/test/requiredIf") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorRequiredUnless *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredunless-valid00") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-valid01") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("eins"));
    QTest::newRow("requiredunless-valid02") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("zwei"));
    QTest::newRow("requiredunless-valid03") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-invalid00") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("vier"));
    QTest::newRow("requiredunless-invalid01") << QStringLiteral("/validator/test/requiredUnless") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");




    // **** Start testing ValidatorRequiredWith *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid00") << QStringLiteral("/validator/test/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid01") << QStringLiteral("/validator/test/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-valid02") << QStringLiteral("/validator/test/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-invalid00") << QStringLiteral("/validator/test/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwith-invalid01") << QStringLiteral("/validator/test/requiredWith") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorRequiredWithAll *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid00") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid01") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid02") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-valid03") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-invalid00") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field3"), QStringLiteral("asdfdasf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfdasf"));
    QTest::newRow("requiredwithall-invalid01") << QStringLiteral("/validator/test/requiredWithAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorRequiredWithout *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithout-valid00") << QStringLiteral("/validator/test/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithout-valid01") << QStringLiteral("/validator/test/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("requiredwithout-invalid00") << QStringLiteral("/validator/test/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("asdfasdf"));
    QTest::newRow("requiredwithout-invalid01") << QStringLiteral("/validator/test/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("asdfasdf"));
    QTest::newRow("requiredwithout-invalid02") << QStringLiteral("/validator/test/requiredWithout") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");


    // **** Start testing ValidatorRequiredWithoutAll *****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field2"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid00") << QStringLiteral("/validator/test/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid01") << QStringLiteral("/validator/test/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-valid02") << QStringLiteral("/validator/test/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    QTest::newRow("requiredwithoutall-invalid00") << QStringLiteral("/validator/test/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field4"), QStringLiteral("wlklasdf"));
    QTest::newRow("requiredwithoutall-invalid01") << QStringLiteral("/validator/test/requiredWithoutAll") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");


    // **** Start testing ValidatorSame *****
    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlklasdf"));
    QTest::newRow("same-valid") << QStringLiteral("/validator/test/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("wlklasdf"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlkla"));
    QTest::newRow("same-invalid") << QStringLiteral("/validator/test/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    query.addQueryItem(QStringLiteral("other"), QStringLiteral("wlkla"));
    QTest::newRow("same-empty") << QStringLiteral("/validator/test/same") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");


    // **** Start testing ValidatorSame *****

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("size-sint-empty") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10"));
    QTest::newRow("size-sint-valid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("sint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5"));
    QTest::newRow("size-sint-invalid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10"));
    QTest::newRow("size-uint-valid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("uint"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("5"));
    QTest::newRow("size-uint-invalid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("10.0"));
    QTest::newRow("size-float-valid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("float"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("-5.234652435"));
    QTest::newRow("size-flost-invalid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghij"));
    QTest::newRow("size-string-valid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("string"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdef"));
    QTest::newRow("size-string-invalid") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("strsdf"));
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("abcdefghijlmnop"));
    QTest::newRow("size-validationdataerror") << QStringLiteral("/validator/test/size") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("validationdataerror");



    // **** Start testing ValidatorTime *****

    count = 0;
    for (Qt::DateFormat df : dateFormats) {
        QTest::newRow(QString(QStringLiteral("time-valid0%1").arg(count)).toUtf8().constData())
                << QStringLiteral("/validator/test/time?field=") + QTime::currentTime().toString(df) << headers << QByteArray() << QByteArrayLiteral("valid");
        count++;
    }

    QTest::newRow("time-invalid") << QStringLiteral("/validator/test/time?field=123456789") << headers << QByteArray() << QByteArrayLiteral("invalid");

    QTest::newRow("time-empty") << QStringLiteral("/validator/test/time?field=%20") << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("time-format-valid") << QStringLiteral("/validator/test/timeFormat?field=") + QTime::currentTime().toString(QStringLiteral("m:hh")) << headers << QByteArray() << QByteArrayLiteral("valid");

    QTest::newRow("time-format-invalid") << QStringLiteral("/validator/test/timeFormat?field=") + QTime::currentTime().toString(QStringLiteral("m:AP")) << headers << QByteArray() << QByteArrayLiteral("invalid");



    // **** Start testing ValidatorUrl*****

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    QTest::newRow("url-valid00") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user"));
    QTest::newRow("url-valid01") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("user"));
    QTest::newRow("url-valid02") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("file:///home/user/test.txt"));
    QTest::newRow("url-valid03") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("%20"));
    QTest::newRow("url-empty") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoRelative"));
    QTest::newRow("url-norelative-valid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoRelative"));
    QTest::newRow("url-norelative-invalid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-valid00") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("/home/user/test.txt"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-valid01") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("file:///home/user/test.txt"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("NoLocalFile"));
    QTest::newRow("url-nolocalfile-invalid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("schemes"), QStringLiteral("HTTP,https"));
    QTest::newRow("url-scheme-valid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("ftp://www.example.org"));
    query.addQueryItem(QStringLiteral("schemes"), QStringLiteral("HTTP,https"));
    QTest::newRow("url-scheme-invalid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");


    query.clear();
    query.addQueryItem(QStringLiteral("field"), QStringLiteral("http://www.example.org"));
    query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("WebsiteOnly"));
    QTest::newRow("url-websiteonly-valid") << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("valid");

    const QStringList invalidWebsiteUrls({QStringLiteral("ftp://www.example.org"), QStringLiteral("file:///home/user/test.txt"), QStringLiteral("/home/user")});
    count = 0;
    for (const QString &invalidWebsite : invalidWebsiteUrls) {
        query.clear();
        query.addQueryItem(QStringLiteral("field"), invalidWebsite);
        query.addQueryItem(QStringLiteral("constraints"), QStringLiteral("WebsiteOnly"));
        QTest::newRow(qUtf8Printable(QStringLiteral("url-websiteonly-invalid0%1").arg(count)))
                << QStringLiteral("/validator/test/url") << headers << query.toString(QUrl::FullyEncoded).toLatin1() << QByteArrayLiteral("invalid");
        count++;
    }


}

QTEST_MAIN(TestValidator)

#include "testvalidator.moc"

#endif //VALIDATORTEST_H
