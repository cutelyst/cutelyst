/*
 * Copyright (C) 2016-2019 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_PLUGIN_STATUSMESSAGE
#define CUTELYST_PLUGIN_STATUSMESSAGE

#include <Cutelyst/plugin.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/context.h>

namespace Cutelyst {

class StatusMessagePrivate;
class CUTELYST_PLUGIN_STATUSMESSAGE_EXPORT StatusMessage : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StatusMessage)
public:
    /**
     * Constructs a new status message object with the given Application parent.
     */
    StatusMessage(Application *parent);
    virtual ~StatusMessage() override;

    /**
     * Returns the key prefix inside Session where messages will be stored.
     */
    QString sessionPrefix() const;

    /**
     * Sets the key prefix inside Session where messages will be stored. Defaults to "status_msg".
     */
    void setSessionPrefix(const QString &sessionPrefix);

    /**
     * Returns the name of the URL param that holds the token on the page where you want to retrieve/display the status message.
     */
    QString tokenParam() const;

    /**
     * Sets the name of the URL param that holds the token on the page where you want to retrieve/display the status message. Defaults to "mid".
     */
    void setTokenParam(const QString &tokenParam);

    /**
     * Returns the name of the stash key where "success" status messages are loaded when load() is called. Defaults to status_msg.
     */
    QString statusMsgStashKey() const;

    /**
     * Sets the name of the stash key where "success" status messages are loaded when load() is called. Defaults to status_msg.
     */
    void setStatusMsgStashKey(const QString &statusMsgStashKey);

    /**
     * Returns the name of the stash key where error messages are loaded when load() is called.
     */
    QString errorMgStashKey() const;

    /**
     * Sets the name of the stash key where error messages are loaded when load() is called. Defaults to error_msg.
     */
    void setErrorMgStashKey(const QString &errorMgStashKey);

    /**
     * Load both messages that match the token param (mid=###) into the stash for display by the view.
     */
    static void load(Context *c);

    /**
     * Saves an error message returning the generated message id (mid)
     */
    static QString error(Context *c, const QString &msg);

    /**
     * Saves an error message returning query parameters with the generated message id (mid) and it's token
     */
    static ParamsMultiMap errorQuery(Context *c, const QString &msg, ParamsMultiMap query = ParamsMultiMap());

    /**
     * Saves a status message returning the generated message id (mid)
     */
    static QString status(Context *c, const QString &msg);

    /**
     * Saves an status message returning query parameters with the generated message id (mid) and it's token
     */
    static ParamsMultiMap statusQuery(Context *c, const QString &msg, ParamsMultiMap query = ParamsMultiMap());

protected:
    /**
     * Reimplemented from Plugin::setup().
     */
    virtual bool setup(Application *app) override;

    StatusMessagePrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_postFork(Application*))
};

}

#endif // CUTELYST_PLUGIN_STATUSMESSAGE
