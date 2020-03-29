/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef URIFOR_H
#define URIFOR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <cutelee/filter.h>
#include <cutelee/safestring.h>
#include <cutelee/util.h>
#include <cutelee/node.h>

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
