/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef C_UTILS_LANGSELECT_H
#define C_UTILS_LANGSELECT_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>
#include <QVector>

class QLocale;

namespace Cutelyst {

class Context;
class LangSelectPrivate;

class CUTELYST_PLUGIN_UTILS_LANGSELECT_EXPORT LangSelect : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LangSelect)
public:
    enum Source : quint8 {
        URLQuery,
        Session,
        Cookie,
        SubDomain,
        Path,
        AcceptHeader = 254,
        Fallback = 255
    };

    /**
     * Constructs a new %LangSelect object with the given @a parent.
     */
    LangSelect(Application *parent);

    /**
     * Deconstructs the %LangSelect object.
     */
    virtual ~LangSelect();

    void setSupportedLocales(const QVector<QLocale> &locales);

    void setSupportedLocales(const QStringList &locales);

    void addSupportedLocale(const QLocale &locale);

    void addSupportedLocale(const QString &locale);

    void setLocalesFromDir(const QString &path, const QString &name, const QString prefix = QString(), const QString &suffix = QString());

    void setLocalesFromDirs(const QString &path, const QString &name);

    QVector<QLocale> supportedLocales() const;

    void setQueryKey(const QString &key);

    void setSessionKey(const QString &key);

    void setCookieName(const QString &name);

    void setSubDomainIndex(qint8 idx);

    void setPathIndex(qint8 idx);

    void setSourceOrder(const QVector<Source> &order);

    void setFallbackLocale(const QLocale &fallback);

    static QVector<QLocale> getSupportedLocales();

protected:
    /**
     * Sets the plugin up.
     */
    virtual bool setup(Application *app) override;

private:
    LangSelectPrivate *const d_ptr;
};

}

#endif // C_UTILS_LANGSELECT_H
