/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOR_P_H
#define CUTELYSTVALIDATOR_P_H

#include "validator.h"
#include "validatorrule.h"

#include <QHash>
#include <QQueue>

namespace Cutelyst {

class ValidatorPrivate
{
    Q_DISABLE_COPY(ValidatorPrivate)
public:
    explicit ValidatorPrivate(const char *trContext)
        : translationContext(trContext)
    {
    }

    ValidatorPrivate(std::initializer_list<ValidatorRule *> vals, const char *trContext)
        : translationContext(trContext)
        , validators(vals)
    {
        if (!validators.empty()) {
            for (ValidatorRule *r : validators) {
                r->setTranslationContext(trContext);
            }
        }
    }

    ~ValidatorPrivate()
    {
        qDeleteAll(validators);
        validators.clear();
    }

    const char *translationContext{nullptr};
    std::vector<ValidatorRule *> validators;
};

class AsyncValidator : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncValidator)
public:
    explicit AsyncValidator(Context *c)
        : QObject()
        , m_context{c}
    {
        connect(this, &AsyncValidator::finished, this, &QObject::deleteLater);
    }

    ~AsyncValidator() override = default;

    void start(const std::vector<ValidatorRule *> &validators,
               Validator::ValidatorFlags flags,
               const ParamsMultiMap &params);

private Q_SLOTS:
    void doValidation();

Q_SIGNALS:
    void finished(const Cutelyst::ValidatorResult &result);

private:
    QQueue<ValidatorRule *> m_validators;
    ValidatorResult m_result;
    QPointer<Context> m_context{nullptr};
    ParamsMultiMap m_params;
    bool m_cancelValidation{false};
    bool m_stopOnFirstError{false};
    bool m_noTrimming{false};
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOR_P_H
