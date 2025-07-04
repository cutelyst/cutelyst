/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "action.h"
#include "application.h"
#include "common.h"
#include "context_p.h"
#include "controller_p.h"
#include "dispatcher.h"

#include <ranges>

#include <QMetaClassInfo>
#include <QRegularExpression>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

/**
 * \ingroup core
 * \class Cutelyst::Controller controller.h Cutelyst/Controller
 * \brief %Cutelyst %Controller base class.
 *
 * Controllers are where the actions in the %Cutelyst framework reside. Each action is
 * represented by a function/method with an attribute to identify what kind of action it is.
 * See the Cutelyst::Dispatcher for more information about how %Cutelyst dispatches to actions.
 *
 * <h3 id="namespace">%Controller namespace</h3>
 * If not explicitely set by C_NAMESPACE() macro, the controller namespace will be generated
 * from the C++ class name and optional C++ namespace name the class is part of. Camel-case
 * class and namespace names like \c FooBar will be converted into a controller namespace
 * \c foo/bar, while \c Foo would be only the \c foo controller namespace. A class \c FooBar
 * inside C++ namespace \c FuBaz would have the controller namespace \c /fu/baz/foo/bar.
 *
 * The controller namespace will be prepended to the public path of your controller methods
 * unless you specify a global public path.
 *
 * A root namespace can be created by setting an empty controller namespace with C_NAMESPACE("").
 *
 * <h3>Private and public paths</h3>
 * Public paths are the routes that you can call with your user agent like the web browser.
 * Private paths are internally used and can for example be used to get an URI via
 * Context::uriForAction(). The public path has not to be the same like the private path, but
 * it can be the same.
 *
 * While the public path is created with the <b>:Path</b> attribute prepended by the
 * <a href="#namespace">controller namespace</a> or from multiple <br>:Chained</b> actions,
 * the private path will be the function/method name prepended by the controller namespace.
 *
 * <h3 id="attributes">Method attributes</h3>
 * Use the C_ATTR() macro to give hints about methods build like C_ATTR(methodName, options),
 * where options are some of the following:
 *
 * \par :Path
 * \parblock
 * An ending public path relative to the <a href="#namespace">controller namespace</a>. The
 * public path has not to be the same as the method name but it can. For example:<br>
 * <b>:Path("")</b> - /namespace (used for the index)<br>
 * <b>:Path("foo")</b> - /namespace/foo<br>
 * <b>:Path("/bar")</b> - /bar
 * \endparblock
 *
 * \par :Global
 * Alias to <b>:Path("/methodname")</b> which sets the method relative to your root.
 *
 * \par :Local
 * Alias to <b>:Path("methodname")</b>.
 *
 * \par :Chained
 * Sets the name of this part of the chain. If it is specified without arguments, it takes the
 * name of the action as default. To starte a chain use <b>:Chained("/")</b>.
 *
 * \par :PathPart
 * The part of the chained path.
 *
 * \par :Args
 * \parblock
 * Indicates the number of expected arguments on a public path. If omitted, an unlimited number of
 * args will be accepted. Can also be set to \c 0 to expect no arguments.
 *
 * Arguments are the request url parts behind your public route path. Like if your public path is
 * \c /foo/bar and you call \a /foo/bar/fu/baz and there is no other path fitting that and if your
 * \c /foo/bar path will expect two arguments, the method behind \c foo/bar would be called with the
 * two arguments \c 'fu' and \c 'baz'.
 * \endparblock
 *
 * \par :AutoArgs
 * Will calculate the number of expected arguments from the number of method arguments where the
 * pointer to the Context ist not counted. Can not be mixed with :AutoCaptureArgs.
 *
 * \par :CaptureArgs
 * Indicates the number of expected arguments on a publich chain path part.
 *
 * \par :AutoCaptureArgs
 * Will calculate the number of expected capture arguments for chain parts from the number of
 * method arguments where the pointer to the Context is not counted. Can not be mixed with
 * :AutoArgs.
 *
 * \par :Private
 * Marks a method as private. Can also be achieved by declaring the method in the private section
 * of your class declaration. This attribute can be useful if you want to mark a public method as
 * private for the dispatcher. It can still be used as public method by your C++ code but will be
 * visible for the dispatcher as private.
 *
 * <h3>Method arguments</h3>
 * Methods are only exposed to the dispatcher when they have a C_ATTR() macro in front and have
 * a pointer to a Context object as first parameter. If the method should take request arguments
 * it should either have QStringList to take the arguments from Request::arguments() or the expected
 * amount of QString parameters.
 *
 * <h3>Special methods</h3>
 * There are also three special methods that can be implemented that will be automatically
 * dispatched, they are Begin(), Auto() and End().
 *
 * \par Begin(Context*)
 * \parblock
 * Called on the closest namespace match. If the %Controller implements \b Begin(), it’s that
 * action that will be called. Otherwise it will try to match looking at the namespace. So there
 * will be only one \b Begin() action dispatched.
 *
 * Begin will be dispached as first action before the \b Auto() actions and the actual action
 * will be dispatched.
 * \endparblock
 *
 * \par Auto(Context*)
 * \parblock
 * Called in namespace order after the \b Begin() action (if any) and before the actual action.
 * If you have a \c Foo and a \c FooBar controller with 'foo' and 'foo/bar' namespaces and both
 * implement \b Auto() and you call an action on the 'foo/bar' namespace, you get Foo->Auto() and
 * FooBar->Auto() called.
 * \endparblock
 *
 * \par End(Context*)
 * \parblock
 * Called on the closest namespace match. If the %Controller implements \b End(), it’s that action
 * that will be called. Otherwise it will try to match looking at the namespace. So there will be
 * only one \b End() action dispatched.
 * \endparblock
 *
 * <h3>Examples</h3>
 * @code{.h}
 * #include <Cutelyst/Controller>
 *
 * using namespace Cutelyst;
 *
 * class Root : public Controller
 * {
 *      // set this as the root namespace
 *      C_NAMESPACE("")
 *      Q_OBJECT
 * public:
 *      explicit Root(QObject *parent = nulltr);
 *      ~Root() override;
 *
 *      // Will create a nameless path on the empty root path
 *      // without any arguments expected what will give a public
 *      // path of '/'. The private path will be '/index'.
 *      C_ATTR(index, :Path :Args(0))
 *      void index(Context *c);
 *
 *      // Will create a nameless path on the empty root path
 *      // taking an unlimited count of arguments that will give
 *      // a public path '/...', where the dots indicate the unlimited
 *      // count of arguments.
 *      C_ATTR(defaultPage, :Path)
 *      void defaultPage(Context *c);
 * };
 *
 * // will have the namespace 'foo/bar'
 * class FooBar : public Controller
 * {
 *      Q_OBJECT
 * public:
 *      explicit FooBar(QObject *parent = nullptr);
 *      ~FooBar() override;
 *
 *      // Will create a nameless path on the namespace 'foo/bar'
 *      // without any arguments expected what will give a public
 *      // path of '/foo/bar'. The private path will be '/foo/bar/index'.
 *      C_ATTR(index, :Path :AutoArgs)
 *      void index(Context *c);
 *
 *      // Will create the path 'baaaz' on the namespace 'foo/bar'
 *      // with one expected argument what will give a public path
 *      // of '/foo/bar/baaaz/∗', where the asterisk indicates the expected
 *      // single argument that will be given to the arg parameter of the
 *      // method. The private path will be '/foo/bar/baz'.
 *      C_ATTR(baz, :Path("baaaz") :Args(1))
 *      void baz(Context *c, const QString &arg);
 * };
 *
 * namespace Api {
 *
 * // will have the namespace 'api/books'
 * class Books : public Controller
 * {
 *      Q_OBJECT
 * punlic:
 *      explicit Books(QObject *parent = nullptr);
 *      ~Books() override;
 *
 *      // Will create a nameless path on the namespace 'api/books'
 *      // without any arguments expected waht will give a public
 *      // path of '/api/books'. The private path will be '/api/books/index'.
 *      C_ATTR(index, :Path :Args(0))
 *      void index(Context *c);
 *
 *      // Will create a chain start path capturing zero arguments.
 *      // Can be used to setup stuff for other chained actions.
 *      // There will be no public path for this. The private path will be
 *      // '/api/books/base'.
 *      C_ATTR(base, :Chained("/") :PathPart("api/books") :CaptureArgs(0))
 *      void base(Context *c);
 *
 *      // Will create a chained action based on 'base' taking three arguments.
 *      // The public path will be '/api/books/create/∗/∗/∗' while the private
 *      // path will be '/api/books/create'. Will also end the chain with taking
 *      // three arguments.
 *      C_ATTR(create, :Chained("base") :PathPart("create") :Args(3))
 *      void create(Context *c, const QString &title,
 *                              const QString &rating,
 *                              const QString &author);
 *
 *      // Will create a chain start path capturing one argument..
 *      // Can be used to setup stuff for other chained actions.
 *      // There will be no public path for this. The private path will be
 *      // '/api/books/author'.
 *      C_ATTR(author, :Chained("/") :PathPart("api/author") :CaptureArgs(1))
 *      void author(Context *c, const QString &authorId);
 *
 *      // Will create a chained action based on 'author' taking zerog arguments
 *      // and ending the chain. Will have a public path of '/api/author/∗/remove',
 *      // the private path will be '/api/books/remove'.
 *      C_ATTR(remove, :Chained("author") :PathPart("remove") :Args(0))
 *      void remove(Context *c);
 * };
 *
 * }
 * @endcode
 */

Controller::Controller(QObject *parent)
    : QObject(parent)
    , d_ptr(new ControllerPrivate(this))
{
}

Controller::~Controller()
{
    Q_D(Controller);
    qDeleteAll(d->actionList);
    delete d_ptr;
}

QString Controller::ns() const noexcept
{
    Q_D(const Controller);
    return d->pathPrefix;
}

Action *Controller::actionFor(QStringView name) const
{
    Q_D(const Controller);
    auto it = d->actions.constFind(name);
    if (it != d->actions.constEnd()) {
        return it->action;
    }
    return d->dispatcher->getAction(name.toString(), d->pathPrefix);
}

ActionList Controller::actions() const noexcept
{
    Q_D(const Controller);
    return d->actionList;
}

bool Controller::operator==(const char *className)
{
    return !qstrcmp(metaObject()->className(), className);
}

bool Controller::preFork(Application *app)
{
    Q_UNUSED(app)
    return true;
}

bool Controller::postFork(Application *app)
{
    Q_UNUSED(app)
    return true;
}

ControllerPrivate::ControllerPrivate(Controller *parent)
    : q_ptr(parent)
{
}

void ControllerPrivate::init(Application *app, Dispatcher *_dispatcher)
{
    Q_Q(Controller);

    q->setObjectName(QString::fromLatin1(q->metaObject()->className()));

    dispatcher  = _dispatcher;
    application = app;

    // Application must always be our parent
    q->setParent(app);

    const QMetaObject *meta = q->metaObject();
    const QString className = QString::fromLatin1(meta->className());
    q->setObjectName(className);

    bool namespaceFound = false;
    for (int i = meta->classInfoCount() - 1; i >= 0; --i) {
        if (qstrcmp(meta->classInfo(i).name(), "Namespace") == 0) {
            pathPrefix = QString::fromLatin1(meta->classInfo(i).value());
            while (pathPrefix.startsWith(u'/')) {
                pathPrefix.remove(0, 1);
            }
            namespaceFound = true;
            break;
        }
    }

    if (!namespaceFound) {
        QString controlerNS;
        bool lastWasUpper = true;

        for (int i = 0; i < className.length(); ++i) {
            const QChar c = className.at(i);
            if (c.isLower() || c.isDigit()) {
                controlerNS.append(c);
                lastWasUpper = false;
            } else if (c == u'_') {
                controlerNS.append(c);
                lastWasUpper = true;
            } else {
                if (!lastWasUpper) {
                    controlerNS.append(u'/');
                }
                if (c != u':') {
                    controlerNS.append(c.toLower());
                }
                lastWasUpper = true;
            }
        }
        pathPrefix = controlerNS;
    }

    registerActionMethods(meta, q, app);
}

void ControllerPrivate::setupFinished()
{
    Q_Q(Controller);

    const ActionList beginList = dispatcher->getActions(QStringLiteral("Begin"), pathPrefix);
    if (!beginList.isEmpty()) {
        beginAutoList.append(beginList.last());
    }

    beginAutoList.append(dispatcher->getActions(QStringLiteral("Auto"), pathPrefix));

    const ActionList endList = dispatcher->getActions(QStringLiteral("End"), pathPrefix);
    if (!endList.isEmpty()) {
        end = endList.last();
    }

    for (Action *action : std::as_const(actionList)) {
        action->dispatcherReady(dispatcher, q);
    }

    q->preFork(qobject_cast<Application *>(q->parent()));
}

bool Controller::_DISPATCH(Context *c)
{
    Q_D(Controller);

    bool ret = true;

    const int &actionRefCount = c->d_ptr->actionRefCount;

    // Dispatch to Begin and Auto
    const auto beginAutoList = d->beginAutoList;
    for (Action *action : beginAutoList) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.enqueue(action);
        } else if (!action->dispatch(c)) {
            ret = false;
            break;
        }
    }

    // Dispatch to Action
    if (ret) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.enqueue(c->action());
        } else {
            ret = c->action()->dispatch(c);
        }
    }

    // Dispatch to End
    if (d->end) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.enqueue(d->end);
        } else if (!d->end->dispatch(c)) {
            ret = false;
        }
    }

    if (actionRefCount) {
        c->d_ptr->engineRequest->status |= EngineRequest::Async;
    }

    return ret;
}

Action *ControllerPrivate::actionClass(const QVariantHash &args)
{
    const auto attributes     = args.value(QStringLiteral("attributes")).value<ParamsMultiMap>();
    const QString actionClass = attributes.value(QStringLiteral("ActionClass"));

    QObject *object = instantiateClass(actionClass, "Cutelyst::Action");
    if (object) {
        if (auto action = qobject_cast<Cutelyst::Action *>(object); action) {
            return action;
        }
        qCWarning(CUTELYST_CONTROLLER) << "ActionClass" << actionClass << "is not an ActionClass"
                                       << object->metaObject()->superClass()->className();
        delete object;
    }

    return new Action;
}

Action *ControllerPrivate::createAction(const QVariantHash &args,
                                        const QMetaMethod &method,
                                        Controller *controller,
                                        Application *app)
{
    Action *action = actionClass(args);
    if (!action) {
        return nullptr;
    }

    QStack<Component *> roles = gatherActionRoles(args);
    for (int i = 0; i < roles.size(); ++i) {
        Component *code = roles.at(i);
        code->init(app, args);
        code->setParent(action);
    }
    action->applyRoles(roles);
    action->setMethod(method);
    action->setController(controller);
    action->setName(args.value(QStringLiteral("name")).toString());
    action->setReverse(args.value(QStringLiteral("reverse")).toString());
    action->setupAction(args, app);

    return action;
}

void ControllerPrivate::registerActionMethods(const QMetaObject *meta,
                                              Controller *controller,
                                              Application *app)
{
    // Setup actions
    for (int i = 0; i < meta->methodCount(); ++i) {
        const QMetaMethod method = meta->method(i);
        const QByteArray name    = method.name();

        // We register actions that are either a Q_SLOT
        // or a Q_INVOKABLE function which has the first
        // parameter type equal to Context*
        if (method.isValid() &&
            (method.methodType() == QMetaMethod::Method ||
             method.methodType() == QMetaMethod::Slot) &&
            (method.parameterCount() &&
             method.parameterType(0) == qMetaTypeId<Cutelyst::Context *>())) {

            // Build up the list of attributes for the class info
            QByteArray attributeArray;
            for (int i2 = meta->classInfoCount() - 1; i2 >= 0; --i2) {
                QMetaClassInfo classInfo = meta->classInfo(i2);
                if (name == classInfo.name()) {
                    attributeArray.append(classInfo.value());
                }
            }
            ParamsMultiMap attrs = parseAttributes(method, attributeArray, name);

            QString reverse;
            if (controller->ns().isEmpty()) {
                reverse = QString::fromLatin1(name);
            } else {
                reverse = controller->ns() + QLatin1Char('/') + QString::fromLatin1(name);
            }

            Action *action =
                createAction({{QStringLiteral("name"), QVariant::fromValue(name)},
                              {QStringLiteral("reverse"), QVariant::fromValue(reverse)},
                              {QStringLiteral("namespace"), QVariant::fromValue(controller->ns())},
                              {QStringLiteral("attributes"), QVariant::fromValue(attrs)}},
                             method,
                             controller,
                             app);

            actions.insert(action->reverse(), {action->reverse(), action});
            actionList.append(action);
        }
    }
}

ParamsMultiMap ControllerPrivate::parseAttributes(const QMetaMethod &method,
                                                  const QByteArray &str,
                                                  const QByteArray &name)
{
    ParamsMultiMap ret;
    std::vector<std::pair<QString, QString>> attributes;
    // This is probably not the best parser ever
    // but it handles cases like:
    // :Args:Local('fo"')o'):ActionClass('foo')
    // into
    // (("Args",""), ("Local","'fo"')o'"), ("ActionClass","'foo'"))

    int size = str.size();
    int pos  = 0;
    while (pos < size) {
        // find the start of a key
        if (str.at(pos) == ':') {
            QString key;
            QString value;
            int keyStart  = ++pos;
            int keyLength = 0;
            while (pos < size) {
                if (str.at(pos) == '(') {
                    // attribute has value
                    int valueStart = ++pos;
                    while (pos < size) {
                        if (str.at(pos) == ')') {
                            // found the possible end of the value
                            int valueEnd = pos;
                            ++pos;
                            if (pos < size && str.at(pos) == ':') {
                                // found the start of a key so this is
                                // really the end of a value
                                value =
                                    QString::fromLatin1(str.mid(valueStart, valueEnd - valueStart));
                                break;
                            } else if (pos >= size) {
                                // found the end of the string
                                // save the remaining as the value
                                value =
                                    QString::fromLatin1(str.mid(valueStart, valueEnd - valueStart));
                                break;
                            }
                            // string was not '):' or ')$'
                            continue;
                        }
                        ++pos;
                    }
                    break;
                } else if (str.at(pos) == ':') {
                    // Attribute has no value
                    break;
                }
                ++keyLength;
                ++pos;
            }

            // store the key
            key = QString::fromLatin1(str.mid(keyStart, keyLength));

            // remove quotes
            if (!value.isEmpty()) {
                if ((value.startsWith(u'\'') && value.endsWith(u'\'')) ||
                    (value.startsWith(u'"') && value.endsWith(u'"'))) {
                    value.remove(0, 1);
                    value.remove(value.size() - 1, 1);
                }
            }

            // store the key/value pair found
            attributes.emplace_back(key, value);
            continue;
        }
        ++pos;
    }

    const static auto digitRE = QRegularExpression(u"\\D"_s);

    // Add the attributes to the map in the reverse order
    std::ranges::for_each(attributes | std::views::reverse, [&](const auto &pair) {
        QString key   = pair.first;
        QString value = pair.second;
        if (key.compare(u"Global") == 0) {
            key   = QStringLiteral("Path");
            value = parsePathAttr(QLatin1Char('/') + QString::fromLatin1(name));
        } else if (key.compare(u"Local") == 0) {
            key   = QStringLiteral("Path");
            value = parsePathAttr(QString::fromLatin1(name));
        } else if (key.compare(u"Path") == 0) {
            value = parsePathAttr(value);
        } else if (key.compare(u"Args") == 0) {
            QString args = value;
            if (!args.isEmpty()) {
                value = args.remove(digitRE);
            }
        } else if (key.compare(u"CaptureArgs") == 0) {
            QString captureArgs = value;
            value               = captureArgs.remove(digitRE);
        } else if (key.compare(u"Chained") == 0) {
            value = parseChainedAttr(value);
        }

        ret.insert(key, value);
    });

    // Handle special AutoArgs and AutoCaptureArgs case
    if (!ret.contains(QStringLiteral("Args")) && !ret.contains(QStringLiteral("CaptureArgs")) &&
        (ret.contains(QStringLiteral("AutoArgs")) ||
         ret.contains(QStringLiteral("AutoCaptureArgs")))) {
        if (ret.contains(QStringLiteral("AutoArgs")) &&
            ret.contains(QStringLiteral("AutoCaptureArgs"))) {
            qFatal("Action '%s' has both AutoArgs and AutoCaptureArgs, which is not allowed",
                   name.constData());
        } else {
            QString parameterName;
            if (ret.contains(QStringLiteral("AutoArgs"))) {
                ret.remove(QStringLiteral("AutoArgs"));
                parameterName = QStringLiteral("Args");
            } else {
                ret.remove(QStringLiteral("AutoCaptureArgs"));
                parameterName = QStringLiteral("CaptureArgs");
            }

            // If the signature is not QStringList we count them
            if (!(method.parameterCount() == 2 &&
                  method.parameterType(1) == QMetaType::QStringList)) {
                int parameterCount = 0;
                for (int i2 = 1; i2 < method.parameterCount(); ++i2) {
                    int typeId = method.parameterType(i2);
                    if (typeId == QMetaType::QString) {
                        ++parameterCount;
                    }
                }
                ret.replace(parameterName, QString::number(parameterCount));
            }
        }
    }

    // If the method is private add a Private attribute
    if (!ret.contains(u"Private"_s) && method.access() == QMetaMethod::Private) {
        ret.insert(u"Private"_s, {});
    }

    return ret;
}

QStack<Component *> ControllerPrivate::gatherActionRoles(const QVariantHash &args)
{
    QStack<Component *> roles;
    const auto attributes = args.value(QStringLiteral("attributes")).value<ParamsMultiMap>();
    auto doesIt           = attributes.constFind(QStringLiteral("Does"));
    while (doesIt != attributes.constEnd() && doesIt.key().compare(u"Does") == 0) {
        QObject *object =
            instantiateClass(doesIt.value(), QByteArrayLiteral("Cutelyst::Component"));
        if (object) {
            roles.push(qobject_cast<Component *>(object));
        }
        ++doesIt;
    }
    return roles;
}

QString ControllerPrivate::parsePathAttr(const QString &value)
{
    QString ret = pathPrefix;
    if (value.startsWith(u'/')) {
        ret = value;
    } else if (!value.isEmpty()) {
        ret = pathPrefix + u'/' + value;
    }
    return ret;
}

QString ControllerPrivate::parseChainedAttr(const QString &attr)
{
    QString ret = QStringLiteral("/");
    if (attr.isEmpty()) {
        return ret;
    }

    if (attr.compare(u".") == 0) {
        ret.append(pathPrefix);
    } else if (!attr.startsWith(u'/')) {
        if (!pathPrefix.isEmpty()) {
            ret.append(pathPrefix + u'/' + attr);
        } else {
            // special case namespace '' (root)
            ret.append(attr);
        }
    } else {
        ret = attr;
    }

    return ret;
}

QObject *ControllerPrivate::instantiateClass(const QString &name, const QByteArray &super)
{
    QString instanceName = name;
    if (!instanceName.isEmpty()) {
        const static QRegularExpression nonWordsRE(u"\\W"_s);
        instanceName.remove(nonWordsRE);

        QMetaType id = QMetaType::fromName(instanceName.toLatin1().data());
        if (!id.isValid()) {
            if (!instanceName.endsWith(u'*')) {
                instanceName.append(u'*');
            }

            id = QMetaType::fromName(instanceName.toLatin1().data());
            if (!id.isValid() && !instanceName.startsWith(u"Cutelyst::")) {
                instanceName = QLatin1String("Cutelyst::") + instanceName;
                id           = QMetaType::fromName(instanceName.toLatin1().data());
            }
        }

        if (id.isValid()) {
            const QMetaObject *metaObj = id.metaObject();
            if (metaObj) {
                if (!superIsClassName(metaObj->superClass(), super)) {
                    qCWarning(CUTELYST_CONTROLLER)
                        << "Class name" << instanceName << "is not a derived class of" << super;
                }

                QObject *object = metaObj->newInstance();
                if (!object) {
                    qCWarning(CUTELYST_CONTROLLER)
                        << "Could create a new instance of" << instanceName
                        << "make sure it's default constructor is "
                           "marked with the Q_INVOKABLE macro";
                }

                return object;
            }
        } else {
            Component *component = application->createComponentPlugin(name);
            if (component) {
                return component;
            }

            component = application->createComponentPlugin(instanceName);
            if (component) {
                return component;
            }
        }

        if (!id.isValid()) {
            qCCritical(CUTELYST_CONTROLLER,
                       "Could not create component '%s', you can register it with "
                       "qRegisterMetaType<%s>(); or set a proper CUTELYST_PLUGINS_DIR",
                       qPrintable(instanceName),
                       qPrintable(instanceName));
        }
    }

    return nullptr;
}

bool ControllerPrivate::superIsClassName(const QMetaObject *super, const QByteArray &className)
{
    if (super) {
        if (super->className() == className) {
            return true;
        }
        return superIsClassName(super->superClass(), className);
    }
    return false;
}

#include "moc_controller.cpp"
