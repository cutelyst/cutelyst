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
#include "csrf.h"

#include <grantlee/exception.h>
#include <grantlee/parser.h>

#include <Cutelyst/Context>
#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/Request>
#include <Cutelyst/Response>

#ifdef PLUGIN_CSRFPROTECTION_ENABLED
#include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>
#endif

Grantlee::Node *CSRFTag::getNode(const QString &tagContent, Grantlee::Parser *p) const
{
    Q_UNUSED(tagContent);
    return new CSRF(p);
}

CSRF::CSRF(Grantlee::Parser *parser) : Grantlee::Node(parser)
{
}

void CSRF::render(Grantlee::OutputStream *stream, Grantlee::Context *gc) const
{
#ifdef PLUGIN_CSRFPROTECTION_ENABLED
    // In case cutelyst context is not set as "c"
    auto c = gc->lookup(m_cutelystContext).value<Cutelyst::Context *>();
    if (!c) {
        const QVariantHash hash = gc->stackHash(0);
        auto it = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().userType() == qMetaTypeId<Cutelyst::Context *>()) {
                c = it.value().value<Cutelyst::Context *>();
                if (c) {
                    m_cutelystContext = it.key();
                    break;
                }
            }
            ++it;
        }

        if (!c) {
            return;
        }
    }

    *stream << Cutelyst::CSRFProtection::getTokenFormField(c);
#else
    Q_UNUSED(stream)
    Q_UNUSED(gc)
    qWarning("%s", "The CSRF protection plugin has not been built.");
#endif
}
