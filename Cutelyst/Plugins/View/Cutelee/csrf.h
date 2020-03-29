/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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
#ifndef CSRF_H
#define CSRF_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <cutelee/filter.h>
#include <cutelee/safestring.h>
#include <cutelee/util.h>
#include <cutelee/node.h>

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
