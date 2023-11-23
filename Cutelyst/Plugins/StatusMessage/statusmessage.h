/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_PLUGIN_STATUSMESSAGE
#define CUTELYST_PLUGIN_STATUSMESSAGE

#include <Cutelyst/context.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

namespace Cutelyst {

class StatusMessagePrivate;
/**
 * @ingroup plugins
 * @headerfile "" <Cutelyst/Plugins/StatusMessage>
 * @brief Manage status messages over multiple requests stored in the Session.
 *
 * The %StatusMessage plugin can be used to generate status messages that are saved
 * to the user’s session that will be available over multiple requests. It distinguishes between
 * normal status messages set via status() or statusQuery() and error messages set via error() or
 * errorQuery(). The static load() method loads the status messages into the Context::stash() at
 * appropriate places like the Auto() method of the root controller. The status messages will be
 * identified by a message id put into the URL query of the request URL using the \c "mid" query
 * key by default.
 *
 * <h3>Usage example</h3>
 * Load the plugin in your applications’s init method:
 * @code{.cpp}
 * #include <Cutelyst/Plugins/StatusMessage/StatusMessage>
 *
 * bool MyCutelystApp::init()
 * {
 *      // other initialization stuff
 *      // ...
 *
 *      new StatusMessage(this);
 *
 *      // maybe more initialization stuff
 *      // ...
 * }
 * @endcode
 *
 * Then use for example your root controller’s Auto method to load the messages into the
 * \link Context::stash() stash\endlink:
 * @code{.cpp}
 * #include <Cutelyst/Plugins/StatusMessage/StatusMessage>
 *
 * bool Root::Auto(Context *c)
 * {
 *      // other stuff
 *      // ...
 *
 *      StatusMessage::load(c);
 * }
 * @endcode
 *
 * In another controller we can now generate a status and/or error message:
 * @code{.cpp}
 * #include <Cutelyst/Plugins/StatusMessage/StatusMessage>
 *
 * void MyOtherController::dostuff(Context *c)
 * {
 *      // do some stuff like creating something
 *
 *      if (ok) {
 *          // this will redirect to a new URL that has a query set with mid=messagetoken
 *          // that will be used to load the message in the Auto method of the root controller
 *          // into the stash
 *          c->response()->redirect("/stuffdone",
 *                                  {},
 *                                  StatusMessage::statusQuery(c, "Successfully done your stuff."));
 *          return;
 *      }
 *
 *      // handle errors
 *      // ...
 * }
 * @endcode
 *
 * @logcat{plugins.statusmessage}
 */
class CUTELYST_PLUGIN_STATUSMESSAGE_EXPORT StatusMessage : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StatusMessage)
public:
    /**
     * Constructs a new %StatusMessage object with the given Application \a parent.
     */
    StatusMessage(Application *parent);

    /**
     * Destroys the %StatusMessage object.
     */
    virtual ~StatusMessage() override;

    /**
     * Returns the key prefix inside Session where messages will be stored.
     * Defaults to \c "status_mg".
     */
    [[nodiscard]] QString sessionPrefix() const noexcept;

    /**
     * Sets the key prefix inside Session where messages will be stored. Defaults to
     * \c "status_msg".
     */
    void setSessionPrefix(const QString &sessionPrefix);

    /**
     * Returns the name of the URL query parameter that holds the token on the page where you want
     * to retrieve/display the status message. Defaults to \c "mid".
     */
    [[nodiscard]] QString tokenParam() const noexcept;

    /**
     * Sets the name of the URL query parameter that holds the token on the page where you want to
     * retrieve/display the status message. Defaults to \c "mid".
     */
    void setTokenParam(const QString &tokenParam);

    /**
     * Returns the name of the stash key where "success" status messages are loaded when load() is
     * called. Defaults to \c "status_msg".
     */
    [[nodiscard]] QString statusMsgStashKey() const noexcept;

    /**
     * Sets the name of the stash key where "success" status messages are loaded when load() is
     * called. Defaults to \c "status_msg".
     */
    void setStatusMsgStashKey(const QString &statusMsgStashKey);

    /**
     * Returns the name of the stash key where error messages are loaded when load() is called.
     * Defaults to \c "error_msg".
     */
    [[nodiscard]] QString errorMgStashKey() const noexcept;

    /**
     * Sets the name of the stash key where error messages are loaded when load() is called.
     * Defaults to \c "error_msg".
     */
    void setErrorMgStashKey(const QString &errorMgStashKey);

    /**
     * Load both messages that match the token param (mid=###) into the
     * \link Context::stash() stash\endlink for display by the view.
     */
    static void load(Context *c);

    /**
     * Saves an error message \a msg and returns the generated message id (mid).
     */
    [[nodiscard]] static QString error(Context *c, const QString &msg);

    /**
     * Saves an error message \a msg returning query parameters with the generated
     * message id (mid) and it's token combined with \a query.
     */
    [[nodiscard]] static ParamsMultiMap
        errorQuery(Context *c, const QString &msg, ParamsMultiMap query = {});

    /**
     * Saves a status message \a msg and returns the generated message id (mid).
     */
    [[nodiscard]] static QString status(Context *c, const QString &msg);

    /**
     * Saves an status message \a msg returning query parameters with the generated
     * message id (mid) and it's token combined with \a query.
     */
    [[nodiscard]] static ParamsMultiMap
        statusQuery(Context *c, const QString &msg, ParamsMultiMap query = {});

protected:
    /**
     * Reimplemented from Plugin::setup().
     */
    virtual bool setup(Application *app) override;

    StatusMessagePrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_postFork(Application *))
};

} // namespace Cutelyst

#endif // CUTELYST_PLUGIN_STATUSMESSAGE
