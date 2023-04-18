/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef URIFOR_H
#define URIFOR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <grantlee/filter.h>
#    include <grantlee/node.h>
#    include <grantlee/safestring.h>
#    include <grantlee/util.h>

class UriForTag final : public Grantlee::AbstractNodeFactory
{
    Grantlee::Node *getNode(const QString &tagContent, Grantlee::Parser *p) const override;
};

class UriFor final : public Grantlee::Node
{
    Q_OBJECT
public:
    explicit UriFor(const QString &path, const QStringList &args, Grantlee::Parser *parser = nullptr);

    void render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Grantlee::FilterExpression m_path;
    std::vector<Grantlee::FilterExpression> m_argsExpressions;
    std::vector<Grantlee::FilterExpression> m_queryExpressions;
};

#endif

#endif // URIFOR_H
