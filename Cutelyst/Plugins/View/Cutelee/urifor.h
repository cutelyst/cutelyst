/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef URIFOR_H
#define URIFOR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#    include <cutelee/filter.h>
#    include <cutelee/node.h>
#    include <cutelee/safestring.h>
#    include <cutelee/util.h>

class UriForTag final : public Cutelee::AbstractNodeFactory
{
    Cutelee::Node *getNode(const QString &tagContent, Cutelee::Parser *p) const override;
};

class UriFor final : public Cutelee::Node
{
    Q_OBJECT
public:
    explicit UriFor(const QString &path, const QStringList &args, Cutelee::Parser *parser = nullptr);

    void render(Cutelee::OutputStream *stream, Cutelee::Context *gc) const override;

private:
    mutable QString m_cutelystContext = QStringLiteral("c");
    Cutelee::FilterExpression m_path;
    std::vector<Cutelee::FilterExpression> m_argsExpressions;
    std::vector<Cutelee::FilterExpression> m_queryExpressions;
};

#endif

#endif // URIFOR_H
