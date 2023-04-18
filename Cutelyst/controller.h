/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_CONTROLLER_H
#define CUTELYST_CONTROLLER_H

#include <Cutelyst/action.h>
#include <Cutelyst/context.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/request.h>
#include <Cutelyst/response.h>

#include <QObject>

#define STR(X) #X
#define C_PATH(X, Y) Q_CLASSINFO(STR(X##_Path), STR(Y))
#define C_NAMESPACE(value) Q_CLASSINFO("Namespace", value)
#define C_ATTR(X, Y) Q_CLASSINFO(STR(X), STR(Y)) Q_INVOKABLE

#define CActionFor(str) \
    ([this]() -> Cutelyst::Action * { \
        static thread_local Cutelyst::Action *action = \
            Cutelyst::Controller::actionFor(str); \
        return action; \
    }()) /**/

namespace Cutelyst {

class ControllerPrivate;
/*! \class Controller controller.h Cutelyst/Controller
 * @brief %Cutelyst %Controller base class
 *
 * Controllers are where the actions in the Cutelyst framework reside.
 * Each action is represented by a function with an attribute to identify
 * what kind of action it is. See the Cutelyst::Dispatcher for more info
 * about how Cutelyst dispatches to actions.
 *
 * Use C_ATTR to give hints about methods
 * build like methodName_option
 * Where option is one of the following:
 *
 * \b :Path - An ending path relative to the class info Namespace
 * for example:
 * \n :Path("") - /namespace/controlername (used for the index)
 * \n :Path("foo") - /namespace/controlername/foo
 * \n :Path("/bar") - /namespace/bar
 *
 * \b :Chained - Sets the name of this part of the chain. If it
 * is specified without arguments, it takes the name of
 * the action as default.
 *
 * \b :PathPart - The part of the chained path
 *
 * \b :Args - In the case of more than 9 parameters, to build
 * the path set the needed number here, where an empty string
 * means unlimited arguments.
 *
 * \b :CaptureArgs - In the case of more than 9 parameters, to
 * be captured the path set the needed number here, where -1
 * means unlimited arguments.
 *
 * \b :Global - Alias to Path="/methodname" which sets the
 * method relative to your root.
 *
 * \b :Local - Alias to Path="methodname".
 *
 * \b :Args - When used with "Path" it indicates the number of
 * arguments in the path.
 * \n The number is computed by counting the arguments the method expects.
 * \n However if no Args value is set, assumed to 'slurp' all
 *    remaining path parts under this namespace.
 *
 * There are also three special methods that can be implemented
 * that will be automatically dispatched, they are Begin(),
 * Auto() and End().
 *
 * Begin(Context*) and End(Context*) are both called on the closest
 * namespace match. If the Controller implements Begin it's that action
 * that will be called otherwise it will try to match looking at the
 * namespace.
 *
 * Auto(Context*) is called in namespace order, so if
 * you have a Foo and a FooBar controller with 'foo' and 'foo/bar'
 * namespaces respectively and both implement Auto(), you get
 * Foo->Auto() and FooBar->Auto() called.
 */
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
     */
    QString ns() const;

    /**
     * Returns the Cutelyst::Action object (if any) for a given method name in
     * this class namespace.
     *
     * You can also use the macro CActionFor to keep the resolved action around.
     */
    Action *actionFor(const QString &name) const;

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
    ActionList actions() const;

    /**
     * Return TRUE if className is equal to this Controller's name
     */
    bool operator==(const char *className);

protected:
    /**
     * This method is called right after Controller has been setup
     * and before application forks and \sa postFork() is called.
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
     * Transversing each namespace's Begin(), nearest Auto(), the Action method of
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

#endif // CUTELYST_CONTROLLER_H
