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

#include <QMetaClassInfo>
#include <QRegularExpression>

using namespace Cutelyst;

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

QString Controller::ns() const
{
    Q_D(const Controller);
    return d->pathPrefix;
}

Action *Controller::actionFor(const QString &name) const
{
    Q_D(const Controller);
    auto it = d->actions.constFind(name);
    if (it != d->actions.constEnd()) {
        return it->action;
    }
    return d->dispatcher->getAction(name, d->pathPrefix);
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

ActionList Controller::actions() const
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

    const auto actions = actionList;
    for (Action *action : actions) {
        action->dispatcherReady(dispatcher, q);
    }

    q->preFork(qobject_cast<Application *>(q->parent()));
}

bool Controller::_DISPATCH(Context *c)
{
    Q_D(Controller);

    bool ret = true;

    int &actionRefCount = c->d_ptr->actionRefCount;

    // Dispatch to Begin and Auto
    const auto beginAutoList = d->beginAutoList;
    for (Action *action : beginAutoList) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.append(action);
        } else if (!action->dispatch(c)) {
            ret = false;
            break;
        }
    }

    // Dispatch to Action
    if (ret) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.append(c->action());
        } else {
            ret = c->action()->dispatch(c);
        }
    }

    // Dispatch to End
    if (d->end) {
        if (actionRefCount) {
            c->d_ptr->pendingAsync.append(d->end);
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
        Action *action = qobject_cast<Action *>(object);
        if (action) {
            return action;
        }
        qCWarning(CUTELYST_CONTROLLER) << "ActionClass"
                                       << actionClass
                                       << "is not an ActionClass";
        delete object;
    }

    return new Action;
}

Action *ControllerPrivate::createAction(const QVariantHash &args, const QMetaMethod &method, Controller *controller, Application *app)
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

void ControllerPrivate::registerActionMethods(const QMetaObject *meta, Controller *controller, Application *app)
{
    // Setup actions
    for (int i = 0; i < meta->methodCount(); ++i) {
        const QMetaMethod method = meta->method(i);
        const QByteArray name    = method.name();

        // We register actions that are either a Q_SLOT
        // or a Q_INVOKABLE function which has the first
        // parameter type equal to Context*
        if (method.isValid() &&
            (method.methodType() == QMetaMethod::Method || method.methodType() == QMetaMethod::Slot) &&
            (method.parameterCount() && method.parameterType(0) == qMetaTypeId<Cutelyst::Context *>())) {

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

            Action *action = createAction({{QStringLiteral("name"), QVariant::fromValue(name)},
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

ParamsMultiMap ControllerPrivate::parseAttributes(const QMetaMethod &method, const QByteArray &str, const QByteArray &name)
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
        QString key;
        QString value;

        // find the start of a key
        if (str.at(pos) == ':') {
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
                            if (++pos < size && str.at(pos) == ':') {
                                // found the start of a key so this is
                                // really the end of a value
                                value = QString::fromLatin1(str.mid(valueStart, valueEnd - valueStart));
                                break;
                            } else if (pos >= size) {
                                // found the end of the string
                                // save the remainig as the value
                                value = QString::fromLatin1(str.mid(valueStart, valueEnd - valueStart));
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

            // stopre the key
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
            attributes.emplace_back(std::make_pair(key, value));
            continue;
        }
        ++pos;
    }

    // Add the attributes to the map in the reverse order so
    // that values() return them in the right order
    for (const auto &pair : attributes) {
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
                value = args.remove(QRegularExpression(QStringLiteral("\\D")));
            }
        } else if (key.compare(u"CaptureArgs") == 0) {
            QString captureArgs = value;
            value               = captureArgs.remove(QRegularExpression(QStringLiteral("\\D")));
        } else if (key.compare(u"Chained") == 0) {
            value = parseChainedAttr(value);
        }

        ret.insert(key, value);
    }

    // Handle special AutoArgs and AutoCaptureArgs case
    if (!ret.contains(QStringLiteral("Args")) && !ret.contains(QStringLiteral("CaptureArgs")) &&
        (ret.contains(QStringLiteral("AutoArgs")) || ret.contains(QStringLiteral("AutoCaptureArgs")))) {
        if (ret.contains(QStringLiteral("AutoArgs")) && ret.contains(QStringLiteral("AutoCaptureArgs"))) {
            qFatal("Action '%s' has both AutoArgs and AutoCaptureArgs, which is not allowed", name.constData());
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
            if (!(method.parameterCount() == 2 && method.parameterType(1) == QMetaType::QStringList)) {
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
    if (!ret.contains(QStringLiteral("Private")) && method.access() == QMetaMethod::Private) {
        ret.replace(QStringLiteral("Private"), QString());
    }

    return ret;
}

QStack<Component *> ControllerPrivate::gatherActionRoles(const QVariantHash &args)
{
    QStack<Component *> roles;
    const auto attributes = args.value(QStringLiteral("attributes")).value<ParamsMultiMap>();
    auto doesIt           = attributes.constFind(QStringLiteral("Does"));
    while (doesIt != attributes.constEnd() && doesIt.key().compare(u"Does") == 0) {
        QObject *object = instantiateClass(doesIt.value(), QByteArrayLiteral("Cutelyst::Component"));
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
        instanceName.remove(QRegularExpression(QStringLiteral("\\W")));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QMetaType id = QMetaType::fromName(instanceName.toLatin1().data());
        if (!id.isValid()) {
            if (!instanceName.endsWith(QLatin1Char('*'))) {
                instanceName.append(QLatin1Char('*'));
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
                        << "Class name"
                        << instanceName
                        << "is not a derived class of"
                        << super;
                }

                QObject *object = metaObj->newInstance();
                if (!object) {
                    qCWarning(CUTELYST_CONTROLLER)
                        << "Could create a new instance of"
                        << instanceName
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
                       "Could not create component '%s', you can register it with qRegisterMetaType<%s>(); or set a proper CUTELYST_PLUGINS_DIR",
                       qPrintable(instanceName),
                       qPrintable(instanceName));
        }
    }
#else
        int id = QMetaType::type(instanceName.toLatin1().data());
        if (!id) {
            if (!instanceName.endsWith(QLatin1Char('*'))) {
                instanceName.append(QLatin1Char('*'));
            }

            id = QMetaType::type(instanceName.toLatin1().data());
            if (!id && !instanceName.startsWith(u"Cutelyst::")) {
                instanceName = QLatin1String("Cutelyst::") + instanceName;
                id           = QMetaType::type(instanceName.toLatin1().data());
            }
        }

        if (id) {
            const QMetaObject *metaObj = QMetaType::metaObjectForType(id);
            if (metaObj) {
                if (!superIsClassName(metaObj->superClass(), super)) {
                    qCWarning(CUTELYST_CONTROLLER)
                        << "Class name"
                        << instanceName
                        << "is not a derived class of"
                        << super;
                }

                QObject *object = metaObj->newInstance();
                if (!object) {
                    qCWarning(CUTELYST_CONTROLLER)
                        << "Could create a new instance of"
                        << instanceName
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

        if (!id) {
            qCCritical(CUTELYST_CONTROLLER,
                       "Could not create component '%s', you can register it with qRegisterMetaType<%s>(); or set a proper CUTELYST_PLUGINS_DIR",
                       qPrintable(instanceName),
                       qPrintable(instanceName));
        }
    }
#endif
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
