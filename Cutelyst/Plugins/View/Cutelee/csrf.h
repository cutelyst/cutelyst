/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSRF_H
#define CSRF_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <cutelee/filter.h>
#    include <cutelee/node.h>
#    include <cutelee/safestring.h>
#    include <cutelee/util.h>

class CSRFTag final : public Cutelee::AbstractNodeFactory
{
    Q_OBJECT
public:
    Cutelee::Node *getNode(const QString &tagContent, Cutelee::Parser *p) const override;
};

class CSRF final : public Cutelee::Node
{
    Q_OBJECT
public:
    explicit CSRF(Cutelee::Parser *parser = nullptr);

    void render(Cutelee::OutputStream *stream, Cutelee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // CSRF_H
