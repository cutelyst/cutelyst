/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSRF_H
#define CSRF_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <grantlee/filter.h>
#    include <grantlee/node.h>
#    include <grantlee/safestring.h>
#    include <grantlee/util.h>

class CSRFTag final : public Grantlee::AbstractNodeFactory
{
    Q_OBJECT
public:
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};

class CSRF final : public Grantlee::Node
{
    Q_OBJECT
public:
    explicit CSRF(Grantlee::Parser *parser = nullptr);

    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // CSRF_H
