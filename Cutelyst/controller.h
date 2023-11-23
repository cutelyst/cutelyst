/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/action.h>
#include <Cutelyst/context.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/request.h>
#include <Cutelyst/response.h>

#include <QObject>

#define STR(X) #X
#define C_PATH(X, Y) Q_CLASSINFO(STR(X##_Path), STR(Y))

/**
 * \related Cutelyst::Controller
 * Explicitely sets the <a href="#namespace">controller namespace</a> to \a value.
 * Use this in the private part of your controller declaration.
 */
#define C_NAMESPACE(value) Q_CLASSINFO("Namespace", value)

/**
 * \related Cutelyst::Controller
 * Sets method attributes \a Y to method \a X and marks the method
 * as \link QObject::Q_INVOKABLE Q_INVOKABLE\endlink.
 * @code{.h}
 * ...
 *
 * C_ATTR(index, :Path :Args(0))
 * void index(Context *c);
 *
 * C_ATTR(pageNotFound, :Path)
 * void pageNotFound(Context *c;
 *
 * ...
 * @endcode
 */
#define C_ATTR(X, Y) Q_CLASSINFO(STR(X), STR(Y)) Q_INVOKABLE

/**
 * \related Cutelyst::Controller
 */
#define CActionFor(str) \
    ([this]() -> Cutelyst::Action * { \
        static thread_local Cutelyst::Action *action = Cutelyst::Controller::actionFor(str); \
        return action; \
    }()) /**/

namespace Cutelyst {

class ControllerPrivate;
class CUTELYST_LIBRARY Controller : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a Controller object with the given \p parent.
     */
    explicit Controller(QObject *parent = nullptr);
    virtual ~Controller();

    /**
     * This specifies the internal namespace the controller should
     * be bound to.
     * By default the controller is bound to the URI version of the
     * controller name. For instance a controller named
     * 'MyFooBar' will be bound to 'my/foo/bar'.
     * The default Root controller is an example of setting
     * namespace to '' (the null string).
     *
     * \sa <a href="#namespace">Controller namespace</a>
     */
    [[nodiscard]] QString ns() const noexcept;

    /**
     * Returns the Cutelyst::Action object (if any) for a given method name in
     * this class namespace.
     *
     * You can also use the macro CActionFor to keep the resolved action around.
     */
    Action *actionFor(QStringView name) const;

    /**
     * Returns the Cutelyst::ActionList containing all actions which belongs to
     * this controller.
     */
    [[nodiscard]] ActionList actions() const noexcept;

    /**
     * Return TRUE if className is equal to this Controller's name
     */
    bool operator==(const char *className);

protected:
    /**
     * This method is called right after Controller has been setup
     * and before application forks and postFork() is called.
     *
     * Reimplement this method if you need to configure
     * internal variable and you need to know for
     * example which configuration options are enabled.
     */
    virtual bool preFork(Application *app);

    /**
     * This method is called after the application
     * has registered all controllers.
     *
     * Reimplement this method if you need to configure
     * internal variable and you need to know for
     * example which configuration options are enabled.
     */
    virtual bool postFork(Application *app);

    /**
     * This is called by the dispatch engine to do the contextual action dispatching.
     * Transversing nearest Begin(), each namespace’s Auto(), the Action method of
     * this controller and nearest End().
     */
    bool _DISPATCH(Context *c);

    ControllerPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Controller)
    friend class Application;
    friend class Dispatcher;
};

} // namespace Cutelyst
