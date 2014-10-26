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

#include "action_p.h"
#include "controller.h"
#include "context.h"
#include "common.h"

#include <QMetaClassInfo>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QDebug>

using namespace Cutelyst;

Action::Action() :
    d_ptr(new ActionPrivate)
{
}

Action::~Action()
{
    delete d_ptr;
}

QList<QPair<QByteArray, QByteArray> > Action::attributeList() const
{
    Q_D(const Action);
    return d->attributesList;
}

void Action::setupAction(const QMetaMethod &method, Controller *controller)
{
    Q_D(Action);

    d->name = controller->ns() + '/' + method.name();
    d->ns = controller->ns();
    d->method = method;
    d->methodName = method.name();
    d->controller = controller;

    QString actionNamespace;
    QList<QPair<QByteArray, QByteArray> > attributes;

    // Parse the Method attributes declared with Q_CLASSINFO
    // They start with the method_name then
    // optionally followed by the number of arguments it takes
    // and finally the attribute name.
    QRegularExpression regex(d->methodName % QLatin1String("_(\\w+)"));
    for (int i = 0; i < controller->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = controller->metaObject()->classInfo(i);
        QByteArray name = classInfo.name();
        if (name == "Namespace") {
            actionNamespace = classInfo.value();
            continue;
        }

        QRegularExpressionMatch match = regex.match(name);
        if (match.hasMatch()) {
            QString type = match.captured(1);
            QString value = classInfo.value();
            if (type == QLatin1String("Path")) {
                if (value.isEmpty()) {
                    value = controller->ns() % QLatin1String("/") % d->name;
                } else if (value.startsWith(QLatin1String("/"))) {
                    value = controller->ns() % QLatin1String("/") % d->name % QLatin1String("/") % value;
                } else {
                    value = controller->ns() % QLatin1String("/") % value;
                }
            }
            attributes.append(qMakePair(type.toLocal8Bit(), value.toLocal8Bit()));

        }
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
            const QByteArray &type = parameterTypes.at(i);
            // Make sure the user defines special
            // parameters types AFTER the ones to be captured
            ignoreParameters = true;
            QByteArray name = d->name;
            if (type == "Global") {
                if (!d->name.startsWith('/')) {
                    name.prepend('/');
                }
                attributes.append(qMakePair(QByteArrayLiteral("Path"),
                                            name));
            } else if (type == "Local") {
                attributes.append(qMakePair(QByteArrayLiteral("Path"),
                                            name));
            } else if (type == "Path") {
                attributes.append(qMakePair(QByteArrayLiteral("Path"),
                                            controller->ns()));
            } else if (type == "Args" && !d->attributes.contains("Args")) {
                d->numberOfArgs = parameterCount;
                attributes.append(qMakePair(QByteArrayLiteral("Args"),
                                            QByteArray::number(d->numberOfArgs)));
            } else if (type == "CaptureArgs" && !d->attributes.contains("CaptureArgs")) {
                d->numberOfCaptures = parameterCount;
                attributes.append(qMakePair(QByteArrayLiteral("CaptureArgs"),
                                            QByteArray::number(d->numberOfCaptures)));
            }
        }
    }

    // Print out deprecated declarations
    for (int i = 0; i < attributes.size(); ++i) {
        const QPair<QByteArray, QByteArray> &pair = attributes.at(i);
        qCWarning(CUTELYST_CORE) << "Action attributes declaration DEPRECATED"
                                 << controller->objectName()
                                 << pair.first << pair.second;
    }

    // If the method is private add a Private attribute
    if (method.access() == QMetaMethod::Private) {
        attributes.append(qMakePair(QByteArrayLiteral("Private"), QByteArray()));
    }

    // Build up the list of attributes for the class info
    for (int i = 0; i < controller->metaObject()->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = controller->metaObject()->classInfo(i);
        if (d->methodName == classInfo.name()) {
            attributes.append(d->parseAttributes(classInfo.value()));
        }
    }

    // Add the attributes to the hash in the reverse order so
    // that values() return them in the right order
    for (int i = attributes.size() - 1; i >= 0; --i) {
        const QPair<QByteArray, QByteArray> &pair = attributes.at(i);
        d->attributes.insertMulti(pair.first, pair.second);
    }
    d->attributesList = attributes;
}

void Action::dispatcherReady(const Dispatcher *dispatch)
{
    Q_UNUSED(dispatch)
    // Default implementations does nothing
}

QMap<QByteArray, QByteArray> Action::attributes() const
{
    Q_D(const Action);
    return d->attributes;
}

QByteArray Action::className() const
{
    Q_D(const Action);
    return d->controller->metaObject()->className();
}

Controller *Action::controller() const
{
    Q_D(const Action);
    return d->controller;
}

bool Action::dispatch(Context *ctx) const
{
    Q_D(const Action);

    if (ctx->detached()) {
        return false;
    }

    QStringList args = ctx->args();
    // Fill the missing arguments
    args += d->emptyArgs;

    if (d->method.returnType() == QMetaType::Bool) {
        bool methodRet;
        bool ret;
        ret = d->method.invoke(d->controller,
                               Qt::DirectConnection,
                               Q_RETURN_ARG(bool, methodRet),
                               Q_ARG(Cutelyst::Context*, ctx),
                               Q_ARG(QString, args.at(0)),
                               Q_ARG(QString, args.at(1)),
                               Q_ARG(QString, args.at(2)),
                               Q_ARG(QString, args.at(3)),
                               Q_ARG(QString, args.at(4)),
                               Q_ARG(QString, args.at(5)),
                               Q_ARG(QString, args.at(6)),
                               Q_ARG(QString, args.at(7)),
                               Q_ARG(QString, args.at(8)));

        if (ret) {
            ctx->setState(methodRet);
            return methodRet;
        }

        // The method failed to be called which means we should detach
        ctx->detach();
        ctx->setState(false);

        return false;
    } else {
        bool ret = d->method.invoke(d->controller,
                                    Qt::DirectConnection,
                                    Q_ARG(Cutelyst::Context*, ctx),
                                    Q_ARG(QString, args.at(0)),
                                    Q_ARG(QString, args.at(1)),
                                    Q_ARG(QString, args.at(2)),
                                    Q_ARG(QString, args.at(3)),
                                    Q_ARG(QString, args.at(4)),
                                    Q_ARG(QString, args.at(5)),
                                    Q_ARG(QString, args.at(6)),
                                    Q_ARG(QString, args.at(7)),
                                    Q_ARG(QString, args.at(8)));
        ctx->setState(ret);
        return ret;
    }
}

bool Action::match(int numberOfArgs) const
{
    Q_D(const Action);
    // If the number of args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfArgs == -1 || d->numberOfArgs == numberOfArgs;
}

bool Action::matchCaptures(int numberOfCaptures) const
{
    Q_D(const Action);
    // If the number of capture args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfCaptures == -1 || d->numberOfCaptures == numberOfCaptures;
}

QByteArray Action::name() const
{
    Q_D(const Action);
    return d->methodName;
}

QByteArray Action::privateName() const
{
    Q_D(const Action);
    return d->name;
}

QByteArray Action::ns() const
{
    Q_D(const Action);
    return d->ns;
}

quint8 Action::numberOfArgs() const
{
    Q_D(const Action);
    return d->numberOfArgs;
}

quint8 Action::numberOfCaptures() const
{
    Q_D(const Action);
    return d->numberOfCaptures;
}

bool Action::isValid() const
{
    Q_D(const Action);
    return d->valid;
}


QList<QPair<QByteArray, QByteArray> > ActionPrivate::parseAttributes(const QByteArray &str)
{
    QList<QPair<QByteArray, QByteArray> > ret;

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
            while (pos < size) {
                if (str.at(pos) == '(') {
                    // attribute has value
                    key = str.mid(keyStart, pos - keyStart);
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
                    key = str.mid(keyStart, pos - keyStart);
                    value = QByteArray();
                    break;
                }
                ++pos;
            }

            // store the key/value pair found
            ret.append(qMakePair(key, value));
            continue;
        }
        ++pos;
    }

    return ret;
}
