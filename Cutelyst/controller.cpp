/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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
    d_ptr(new ControllerPrivate)
{
}

Controller::~Controller()
{
    delete d_ptr;
}

QByteArray Controller::ns() const
{
    Q_D(const Controller);
    return d->pathPrefix;
}

Action *Controller::actionFor(const QByteArray &name) const
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

}

bool Controller::Auto(Context *ctx)
{
    return true;
}

void Controller::End(Context *ctx)
{

}

void Controller::init()
{
    Q_D(Controller);

    const QMetaObject *meta = metaObject();
    const QString &className = QString::fromLatin1(meta->className());
    setObjectName(className);

    QByteArray controlerNS;
    for (int i = 0; i < meta->classInfoCount(); ++i) {
        if (metaObject()->classInfo(i).name() == QLatin1String("Namespace")) {
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
    d->pathPrefix = controlerNS;

    d->registerActionMethods(meta, this);
}

void Controller::setupActions(Dispatcher *dispatcher)
{
    Q_D(Controller);

    d->dispatcher = dispatcher;

    ActionList beginList;
    beginList = dispatcher->getActions(QByteArrayLiteral("Begin"), d->pathPrefix);
    if (!beginList.isEmpty()) {
        d->begin = beginList.last();
        d->actionSteps.append(d->begin);
    }

    d->autoList = dispatcher->getActions(QByteArrayLiteral("Auto"), d->pathPrefix);
    d->actionSteps.append(d->autoList);

    ActionList endList;
    endList = dispatcher->getActions(QByteArrayLiteral("End"), d->pathPrefix);
    if (!endList.isEmpty()) {
        d->end = endList.last();
    }
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


Action *ControllerPrivate::actionClass(const QVariantHash &args)
{
    QMap<QByteArray, QByteArray> attributes = args.value("attributes").value<QMap<QByteArray, QByteArray> >();
    QString actionClass = attributes.value("ActionClass");

    if (!actionClass.isEmpty()) {
        actionClass.remove(QRegularExpression("\\W"));
        if (!actionClass.endsWith(QChar('*'))) {
            actionClass.append("*");
        }

        int id = QMetaType::type(actionClass.toLocal8Bit().data());
        if (!id) {
            QString actionClassCutelyst = QLatin1String("Cutelyst::") % actionClass;
            id = QMetaType::type(actionClassCutelyst.toLocal8Bit().data());
        }

        if (id) {
            const QMetaObject *metaObj = QMetaType::metaObjectForType(id);
            if (metaObj) {
                QObject *object = metaObj->newInstance();
                if (object && superIsAction(metaObj->superClass())) {
                    return qobject_cast<Action*>(object);
                } else {
                    qCWarning(CUTELYST_CORE) << "ActionClass"
                                             << actionClass
                                             << "is not an ActionClass";
                    delete object;
                }
            }
        }

        if (!id) {
            qCWarning(CUTELYST_CORE) << "ActionClass"
                                     << actionClass
                                     << "is not registerd, you can register it with qRegisterMetaType<"
                                     << actionClass.toLocal8Bit().data()
                                     << "*>();";
        }
    }

    return new Action;
}

Action *ControllerPrivate::createAction(const QVariantHash &args)
{
    Action *action = actionClass(args);

    QByteArray name = args.value("name").toByteArray();
    QRegularExpression regex("^_(DISPATCH|BEGIN|AUTO|ACTION|END)$");
    QRegularExpressionMatch match = regex.match(name);
    if (!match.hasMatch()) {
        QMap<QByteArray, QByteArray> attributes = args.value("attributes").value<QMap<QByteArray, QByteArray> >();
        QList<QByteArray> roles = attributes.values("Does");
        if (!roles.isEmpty()) {

        }
    }

    return action;
}

void ControllerPrivate::registerActionMethods(const QMetaObject *meta, Controller *controller)
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
            QMap<QByteArray, QByteArray> attrs = parseAttributes(method, attributeArray, name);

            QByteArray reverse = controller->ns() + '/' + name;

            Action *action = createAction({
                                              {"name"      , QVariant::fromValue(name)},
                                              {"reverse"   , QVariant::fromValue(reverse)},
                                              {"namespace" , QVariant::fromValue(controller->ns())},
                                              {"attributes", QVariant::fromValue(attrs)}
                                          });
            action->setupAction(method, {
                                    {"name"      , QVariant::fromValue(name)},
                                    {"reverse"   , QVariant::fromValue(reverse)},
                                    {"namespace" , QVariant::fromValue(controller->ns())},
                                    {"attributes", QVariant::fromValue(attrs)}
                                },
                                controller);

            actions.insertMulti(action->reverse(), action);
        }
    }
}

QMap<QByteArray, QByteArray> ControllerPrivate::parseAttributes(const QMetaMethod &method, const QByteArray &str, const QByteArray &name)
{
    QList<QPair<QByteArray, QByteArray> > attributes;
    // This is probably not the best parser ever
    // but it handles cases like:
    // :Args:Local('fo"')o'):ActionClass('foo')
    // into
    // (QPair("Args",""), QPair("Local","'fo"')o'"), QPair("ActionClass","'foo'"))

    QByteArray key;
    QByteArray value;
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
            attributes.append(qMakePair(parameterTypes.at(i),
                                        QByteArray()));

            // Print out deprecated declarations
            qCWarning(CUTELYST_CORE) << "Action attributes declaration DEPRECATED"
                                     << name
                                     << parameterTypes.at(i);
        }
    }

    QMap<QByteArray, QByteArray> ret;
    // Add the attributes to the hash in the reverse order so
    // that values() return them in the right order
    for (int i = attributes.size() - 1; i >= 0; --i) {
        const QPair<QByteArray, QByteArray> &pair = attributes.at(i);
        QByteArray key = pair.first;
        QByteArray value = pair.second;
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

QByteArray ControllerPrivate::parsePathAttr(const QByteArray &_value)
{
    QByteArray value = _value;
    if (value.isNull()) {
        value = "";
    }

    if (value.startsWith('/')) {
        return value;
    } else if (value.length()) {
        return pathPrefix + '/' + value;
    }
    return pathPrefix;
}

bool ControllerPrivate::superIsAction(const QMetaObject *super)
{
    if (super) {
        if (qstrcmp(super->className(), "Cutelyst::Action") == 0) {
            return true;
        }
        return superIsAction(super->superClass());
    }
    return false;
}
