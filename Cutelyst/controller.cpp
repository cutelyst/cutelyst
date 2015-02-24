/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "controller_p.h"

#include "application.h"
#include "dispatcher.h"
#include "action.h"
#include "common.h"

#include <QMetaClassInfo>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QDebug>

using namespace Cutelyst;

Controller::Controller(QObject *parent) :
    QObject(parent),
    d_ptr(new ControllerPrivate(this))
{
}

Controller::~Controller()
{
    Q_D(Controller);
    qDeleteAll(d->actions);
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
    Action *ret = d->actions.value(name);
    if (ret) {
        return ret;
    }
    return d->dispatcher->getAction(name, d->pathPrefix);
}

ActionList Controller::actions() const
{
    Q_D(const Controller);
    return d->actions.values();
}

bool Controller::operator==(const char *className)
{
    return !qstrcmp(metaObject()->className(), className);
}

void Controller::Begin(Context *ctx)
{
    Q_UNUSED(ctx)
}

bool Controller::Auto(Context *ctx)
{
    Q_UNUSED(ctx)
    return true;
}

void Controller::End(Context *ctx)
{
    Q_UNUSED(ctx)
}

bool Controller::preFork(Application *app)
{
    Q_UNUSED(app)
}

bool Controller::postFork(Application *app)
{
    Q_UNUSED(app)
}

bool Controller::_BEGIN(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    if (d->begin) {
        d->begin->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}

bool Controller::_AUTO(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    Q_FOREACH (Action *autoAction, d->autoList) {
        if (!autoAction->dispatch(ctx)) {
            return false;
        }
    }
    return true;
}

bool Controller::_ACTION(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    if (ctx->action()) {
        return ctx->action()->dispatch(ctx);
    }
    return !ctx->error();
}

bool Controller::_END(Context *ctx)
{
//    qDebug() << Q_FUNC_INFO;
    Q_D(Controller);
    if (d->end) {
        d->end->dispatch(ctx);
        return !ctx->error();
    }
    return true;
}


ControllerPrivate::ControllerPrivate(Controller *parent) :
    q_ptr(parent)
{
}

void ControllerPrivate::init(Application *app, Dispatcher *_dispatcher)
{
    Q_Q(Controller);

    dispatcher = _dispatcher;

    // Application must always be our parent
    q->setParent(app);

    const QMetaObject *meta = q->metaObject();
    const QString &className = QString::fromLatin1(meta->className());
    q->setObjectName(className);

    QByteArray controlerNS;
    for (int i = 0; i < meta->classInfoCount(); ++i) {
        if (meta->classInfo(i).name() == QLatin1String("Namespace")) {
            controlerNS = meta->classInfo(i).value();
            break;
        }
    }

    if (controlerNS.isNull()) {
        bool lastWasUpper = true;

        for (int i = 0; i < className.length(); ++i) {
            if (className.at(i).toLower() == className.at(i)) {
                controlerNS.append(className.at(i));
                lastWasUpper = false;
            } else {
                if (lastWasUpper) {
                    controlerNS.append(className.at(i).toLower());
                } else {
                    controlerNS.append(QLatin1Char('/') % className.at(i).toLower());
                }
                lastWasUpper = true;
            }
        }
    }
    pathPrefix = controlerNS;

    registerActionMethods(meta, q, app);
}

void ControllerPrivate::setupFinished()
{
    Q_Q(Controller);

    const ActionList &beginList = dispatcher->getActions(QStringLiteral("Begin"), pathPrefix);
    if (!beginList.isEmpty()) {
        begin = beginList.last();
        actionSteps.append(begin);
    }

    autoList = dispatcher->getActions(QStringLiteral("Auto"), pathPrefix);
    actionSteps.append(autoList);

    const ActionList &endList = dispatcher->getActions(QStringLiteral("End"), pathPrefix);
    if (!endList.isEmpty()) {
        end = endList.last();
    }

    Q_FOREACH (Action *action, actions.values()) {
        action->dispatcherReady(dispatcher, q);
    }

    q->preFork(qobject_cast<Application *>(q->parent()));
}

void Controller::_DISPATCH(Context *ctx)
{
    Q_D(Controller);

    bool failedState = false;

    // Dispatch to _BEGIN and _AUTO
    Q_FOREACH (Action *action, d->actionSteps) {
        if (!action->dispatch(ctx)) {
            failedState = true;
            break;
        }
    }

    // Dispatch to _ACTION
    if (!failedState) {
        ctx->action()->dispatch(ctx);
    }

    // Dispatch to _END
    if (d->end) {
        d->end->dispatch(ctx);
    }
}

Action *ControllerPrivate::actionClass(const QVariantHash &args)
{
    QMap<QString, QString> attributes;
    attributes = args.value("attributes").value<QMap<QString, QString> >();
    QString actionClass = attributes.value("ActionClass");

    QObject *object = instantiateClass(actionClass.toLatin1(), "Cutelyst::Action");
    if (object) {
        Action *action = qobject_cast<Action*>(object);
        if (action) {
            return qobject_cast<Action*>(object);
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
        return 0;
    }

    QString name = args.value("name").toString();
    QRegularExpression regex(QStringLiteral("^_(DISPATCH|BEGIN|AUTO|ACTION|END)$"));
    QRegularExpressionMatch match = regex.match(name);
    if (!match.hasMatch()) {
        QStack<Code *> roles = gatherActionRoles(args);
        for (int i = 0; i < roles.size(); ++i) {
            Code *code = roles.at(i);
            code->init(app, args);
            code->setParent(action);
        }
        action->applyRoles(roles);
    }

    action->setMethod(method);
    action->setController(controller);
    action->setupAction(args, app);

    return action;
}

void ControllerPrivate::registerActionMethods(const QMetaObject *meta, Controller *controller, Application *app)
{
    // Setup actions
    for (int i = 0; i < meta->methodCount(); ++i) {
        const QMetaMethod &method = meta->method(i);
        const QByteArray &name = method.name();

        // We register actions that are either a Q_SLOT
        // or a Q_INVOKABLE function which has the first
        // parameter type equal to Context*
        if (method.isValid() &&
                (method.methodType() == QMetaMethod::Method || method.methodType() == QMetaMethod::Slot) &&
                (method.parameterCount() && method.parameterType(0) == qMetaTypeId<Cutelyst::Context *>())) {

            // Build up the list of attributes for the class info
            QByteArray attributeArray;
            for (int i = 0; i < meta->classInfoCount(); ++i) {
                QMetaClassInfo classInfo = meta->classInfo(i);
                if (name == classInfo.name()) {
                    attributeArray.append(classInfo.value());
                }
            }
            QMap<QString, QString> attrs = parseAttributes(method, attributeArray, name);

            QString reverse;
            if (controller->ns().isEmpty()) {
                reverse = name;
            } else {
                reverse = controller->ns() % QLatin1Char('/') % name;
            }

            Action *action = createAction({
                                              {"name"      , QVariant::fromValue(name)},
                                              {"reverse"   , QVariant::fromValue(reverse)},
                                              {"namespace" , QVariant::fromValue(controller->ns())},
                                              {"attributes", QVariant::fromValue(attrs)}
                                          },
                                          method,
                                          controller,
                                          app);

            actions.insertMulti(action->reverse(), action);
        }
    }
}

QMap<QString, QString> ControllerPrivate::parseAttributes(const QMetaMethod &method, const QByteArray &str, const QByteArray &name)
{
    QList<QPair<QString, QString> > attributes;
    // This is probably not the best parser ever
    // but it handles cases like:
    // :Args:Local('fo"')o'):ActionClass('foo')
    // into
    // (QPair("Args",""), QPair("Local","'fo"')o'"), QPair("ActionClass","'foo'"))

    QString key;
    QString value;
    int size = str.size();
    int pos = 0;
    while (pos < size) {
        // find the start of a key
        if (str.at(pos) == ':') {
            int keyStart = ++pos;
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
                                value = str.mid(valueStart, valueEnd - valueStart);
                                break;
                            } else if (pos >= size) {
                                // found the end of the string
                                // save the remainig as the value
                                value = str.mid(valueStart, valueEnd - valueStart);
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
                    value = QByteArray();
                    break;
                }
                ++keyLength;
                ++pos;
            }

            // stopre the key
            key = str.mid(keyStart, keyLength);

            // remove quotes
            if (!value.isEmpty()) {
                if ((value.startsWith(QChar('\'')) && value.endsWith(QChar('\''))) ||
                        (value.startsWith(QChar('"')) && value.endsWith(QChar('"')))) {
                    value.remove(0, 1);
                    value.remove(value.size() - 1, 1);
                }
            }

            // store the key/value pair found
            attributes.append(qMakePair(key, value));
            continue;
        }
        ++pos;
    }

    // if the method has the CaptureArgs as an argument
    // set it on the attributes
    int parameterCount = 0;
    bool ignoreParameters = false;
    QList<QByteArray> parameterTypes = method.parameterTypes();
    for (int i = 1; i < method.parameterCount(); ++i) {
        int typeId = method.parameterType(i);
        if (typeId == QMetaType::QString && !ignoreParameters) {
            ++parameterCount;
        } else {
            // store the key/value pair found
            attributes.append(qMakePair(QString::fromLatin1(parameterTypes.at(i)),
                                        QByteArray()));

            // Print out deprecated declarations
            qCWarning(CUTELYST_CONTROLLER) << "Action attributes declaration DEPRECATED"
                                           << name
                                           << parameterTypes.at(i);
        }
    }

    QMap<QString, QString> ret;
    // Add the attributes to the hash in the reverse order so
    // that values() return them in the right order
    for (int i = attributes.size() - 1; i >= 0; --i) {
        const QPair<QString, QString> &pair = attributes.at(i);
        QString key = pair.first;
        QString value = pair.second;
        if (key == "Global") {
            key = QByteArrayLiteral("Path");
            value = name;
            if (!value.startsWith('/')) {
                value.prepend('/');
            }
            value = parsePathAttr(value);
        } else if (key == "Local") {
            key = QByteArrayLiteral("Path");
            value = parsePathAttr(name);
        } else if (key == "Path") {
            value = parsePathAttr(value);
        } else if (key == "Args") {
            QString args = value;
            if (args.isEmpty()) {
                value = QByteArray::number(parameterCount);
            } else {
                value = args.remove(QRegularExpression("\\D")).toLocal8Bit();
            }
        } else if (key == "CaptureArgs") {
            QString captureArgs = value;
            value = captureArgs.remove(QRegularExpression("\\D")).toLocal8Bit();
        } else if (key == QLatin1String("Chained")) {
            value = parseChainedAttr(value);
        }

        ret.insertMulti(key, value);
    }

    if (parameterCount && !ret.contains("Args")) {
        ret.insert("Args", QByteArray::number(parameterCount));
    }

    // If the method is private add a Private attribute
    if (!ret.contains("Private") && method.access() == QMetaMethod::Private) {
        ret.insert("Private", QByteArray());
    }

    return ret;
}

QStack<Code *> ControllerPrivate::gatherActionRoles(const QVariantHash &args)
{
    QStack<Code *> roles;
    QMap<QByteArray, QByteArray> attributes;
    attributes = args.value("attributes").value<QMap<QByteArray, QByteArray> >();
    Q_FOREACH (const QByteArray &role, attributes.values("Does")) {
        QObject *object = instantiateClass(role, "Cutelyst::Code");
        if (object) {
            roles.push(qobject_cast<Code *>(object));
        }
    }
    return roles;
}

QString ControllerPrivate::parsePathAttr(const QString &_value)
{
    QString value = _value;
    if (value.isNull()) {
        value = "";
    }

    if (value.startsWith(QChar('/'))) {
        return value;
    } else if (value.length()) {
        return pathPrefix + '/' + value;
    }
    return pathPrefix;
}

QString ControllerPrivate::parseChainedAttr(const QString &attr)
{
    if (attr.isEmpty()) {
        return QStringLiteral("/");
    }

    if (attr == QLatin1String(".")) {
        return QLatin1Char('/') % pathPrefix;
    } else if (!attr.startsWith(QChar('/'))) {
        if (!pathPrefix.isEmpty()) {
            return QLatin1Char('/') % pathPrefix % QLatin1Char('/') % attr;
        } else {
            // special case namespace '' (root)
            return QLatin1Char('/') % attr;
        }
    }

    return attr;
}

QObject *ControllerPrivate::instantiateClass(const QByteArray &name, const QByteArray &super)
{
    QString instanceName = name;
    if (!instanceName.isEmpty()) {
        instanceName.remove(QRegularExpression("\\W"));

        int id = QMetaType::type(instanceName.toLocal8Bit().data());
        if (!id) {
            if (!instanceName.endsWith(QChar('*'))) {
                instanceName.append("*");
            }

            id = QMetaType::type(instanceName.toLocal8Bit().data());
            if (!id && !instanceName.startsWith(QLatin1String("Cutelyst::"))) {
                instanceName = QLatin1String("Cutelyst::") % instanceName;
                id = QMetaType::type(instanceName.toLocal8Bit().data());
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
        }

        if (!id) {
            qCCritical(CUTELYST_CONTROLLER)
                    << "Class name"
                    << instanceName
                    << "is not registerd, you can register it with qRegisterMetaType<"
                    << instanceName.toLocal8Bit().data()
                    << ">();";
            exit(1);
        }
    }
    return 0;
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
