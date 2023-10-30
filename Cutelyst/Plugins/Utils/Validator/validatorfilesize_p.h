/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORFILESIZE_P_H
#define CUTELYSTVALIDATORFILESIZE_P_H

#include "validatorfilesize.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorFileSizePrivate : public ValidatorRulePrivate
{
public:
    ValidatorFileSizePrivate(const QString &f,
                             ValidatorFileSize::Option o,
                             QVariant mi,
                             QVariant ma,
                             const ValidatorMessages &m,
                             const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , min(std::move(mi))
        , max(std::move(ma))
        , option(o)
    {
    }

    static constexpr char16_t ascii_B{66};
    static constexpr char16_t ascii_E{69};
    static constexpr char16_t ascii_G{71};
    static constexpr char16_t ascii_I{73};
    static constexpr char16_t ascii_K{75};
    static constexpr char16_t ascii_M{77};
    static constexpr char16_t ascii_P{80};
    static constexpr char16_t ascii_T{84};
    static constexpr char16_t ascii_Y{89};

    QVariant min;
    QVariant max;
    ValidatorFileSize::Option option = ValidatorFileSize::NoOption;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORFILESIZE_P_H
