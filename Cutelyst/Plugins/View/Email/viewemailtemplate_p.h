/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWEMAILTEMPLATE_P_H
#define VIEWEMAILTEMPLATE_P_H

#include "viewemailtemplate.h"
#include "viewemail_p.h"

using namespace SimpleMail;
namespace Cutelyst {

class ViewEmailTemplatePrivate : public ViewEmailPrivate
{
public:
    virtual ~ViewEmailTemplatePrivate() override = default;

    QString templatePrefix;
    QString defaultView;
};

}

#endif // VIEWEMAILTEMPLATE_P_H

