/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "action_p.h"
#include "common.h"
#include "context.h"
#include "controller.h"

using namespace Cutelyst;

Action::Action(QObject *parent)
    : Component(new ActionPrivate, parent)
{
}

Action::Action(ActionPrivate *ptr, QObject *parent)
    : Component(ptr, parent)
{
}

Component::Modifiers Action::modifiers() const
{
    return Component::OnlyExecute;
}

void Action::setMethod(const QMetaMethod &method)
{
    Q_D(Action);
    d->method = method;
    if (method.returnType() == QMetaType::Bool) {
        d->evaluateBool = true;
    }

    if (method.parameterCount() == 2 && method.parameterType(1) == QMetaType::QStringList) {
        d->listSignature = true;
    }
}

void Action::setController(Controller *controller)
{
    Q_D(Action);
    d->controller = controller;
}

void Action::setupAction(const QVariantHash &args, Application *app)
{
    Q_D(Action);

    init(app, args);

    d->ns = args.value(QLatin1String("namespace")).toString();

    const auto attributes = args.value(QLatin1String("attributes")).value<ParamsMultiMap>();
    d->attributes         = attributes;

    const QString argsAttr = attributes.value(QLatin1String("Args"));
    if (!argsAttr.isEmpty()) {
        d->numberOfArgs = qint8(argsAttr.toInt());
    }

    const QString capturesAttr = attributes.value(QLatin1String("CaptureArgs"));
    if (!capturesAttr.isEmpty()) {
        d->numberOfCaptures = qint8(capturesAttr.toInt());
    }
}

ParamsMultiMap Action::attributes() const noexcept
{
    Q_D(const Action);
    return d->attributes;
}

QString Action::attribute(const QString &name, const QString &defaultValue) const
{
    Q_D(const Action);
    return d->attributes.value(name, defaultValue);
}

void Action::setAttributes(const ParamsMultiMap &attributes)
{
    Q_D(Action);
    d->attributes = attributes;
}

QString Action::className() const
{
    Q_D(const Action);
    return QString::fromLatin1(d->controller->metaObject()->className());
}

Controller *Action::controller() const
{
    Q_D(const Action);
    return d->controller;
}

bool Action::match(int numberOfArgs) const noexcept
{
    Q_D(const Action);
    // If the number of args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfArgs == -1 || d->numberOfArgs == numberOfArgs;
}

bool Action::matchCaptures(int numberOfCaptures) const noexcept
{
    Q_D(const Action);
    // If the number of capture args is -1 (not defined)
    // it will slurp all args so we don't care
    // about how many args was passed, otherwise
    // count them
    return d->numberOfCaptures == -1 || d->numberOfCaptures == numberOfCaptures;
}

QString Action::ns() const noexcept
{
    Q_D(const Action);
    return d->ns;
}

qint8 Action::numberOfArgs() const noexcept
{
    Q_D(const Action);
    return d->numberOfArgs;
}

qint8 Action::numberOfCaptures() const noexcept
{
    Q_D(const Action);
    return d->numberOfCaptures;
}

bool Action::doExecute(Context *c)
{
    Q_D(const Action);
    if (c->detached()) {
        return false;
    }

    bool ret;

#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))

    if (d->evaluateBool) {
        bool methodRet;

        if (d->listSignature) {
            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_RETURN_ARG(bool, methodRet),
                                   Q_ARG(Cutelyst::Context *, c),
                                   Q_ARG(QStringList, c->request()->args()));
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args.append(d->emptyArgs);

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_RETURN_ARG(bool, methodRet),
                                   Q_ARG(Cutelyst::Context *, c),
                                   Q_ARG(QString, args.at(0)),
                                   Q_ARG(QString, args.at(1)),
                                   Q_ARG(QString, args.at(2)),
                                   Q_ARG(QString, args.at(3)),
                                   Q_ARG(QString, args.at(4)),
                                   Q_ARG(QString, args.at(5)),
                                   Q_ARG(QString, args.at(6)),
                                   Q_ARG(QString, args.at(7)),
                                   Q_ARG(QString, args.at(8)));
        }

        if (ret) {
            c->setState(methodRet);
            return methodRet;
        }

        // The method failed to be called which means we should detach
        c->detach();
        c->setState(false);

        return false;
    } else {
        if (d->listSignature) {
            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_ARG(Cutelyst::Context *, c),
                                   Q_ARG(QStringList, c->request()->args()));
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args.append(d->emptyArgs);

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   Q_ARG(Cutelyst::Context *, c),
                                   Q_ARG(QString, args.at(0)),
                                   Q_ARG(QString, args.at(1)),
                                   Q_ARG(QString, args.at(2)),
                                   Q_ARG(QString, args.at(3)),
                                   Q_ARG(QString, args.at(4)),
                                   Q_ARG(QString, args.at(5)),
                                   Q_ARG(QString, args.at(6)),
                                   Q_ARG(QString, args.at(7)),
                                   Q_ARG(QString, args.at(8)));
        }
        c->setState(ret);
        return ret;
    }

#else

    /*
     * Qt 6.5 introduced a new variadic version of QMetaMethod::invoke() that
     * does not work with our current implementation above. The following code
     * is a fast fix to use the new version but there might be better / more elegant
     * approaches.
     *
     * See: https://codereview.qt-project.org/c/qt/qtbase/+/422745
     *
     * TODO: check for more flexible implementation
     */

    if (d->evaluateBool) {
        bool methodRet;

        if (d->listSignature) {

            QStringList args = c->request()->args();

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   qReturnArg(methodRet),
                                   c,
                                   args);
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args.append(d->emptyArgs);

            switch (d->method.parameterCount()) {
            case 0:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet));
                break;
            case 1:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c);
                break;
            case 2:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0));
                break;
            case 3:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1));
                break;
            case 4:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2));
                break;
            case 5:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3));
                break;
            case 6:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4));
                break;
            case 7:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5));
                break;
            case 8:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6));
                break;
            case 9:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6), args.value(7));
                break;
            default:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, qReturnArg(methodRet), c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6), args.value(7), args.value(8));
                break;
            }
        }

        if (ret) {
            c->setState(methodRet);
            return methodRet;
        }

        // The method failed to be called which means we should detach
        c->detach();
        c->setState(false);

        return false;
    } else {
        if (d->listSignature) {

            QStringList args = c->request()->args();

            ret = d->method.invoke(d->controller,
                                   Qt::DirectConnection,
                                   c,
                                   args);
        } else {
            QStringList args = c->request()->args();
            // Fill the missing arguments
            args.append(d->emptyArgs);

            switch (d->method.parameterCount()) {
            case 0:
                ret = d->method.invoke(d->controller, Qt::DirectConnection);
                break;
            case 1:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c);
                break;
            case 2:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0));
                break;
            case 3:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1));
                break;
            case 4:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2));
                break;
            case 5:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3));
                break;
            case 6:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4));
                break;
            case 7:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5));
                break;
            case 8:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6));
                break;
            case 9:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6), args.value(7));
                break;
            default:
                ret = d->method.invoke(d->controller, Qt::DirectConnection, c, args.value(0), args.value(1), args.value(2), args.value(3), args.value(4), args.value(5), args.value(6), args.value(7), args.value(8));
                break;
            }
        }
        c->setState(ret);
        return ret;
    }

#endif
}

#include "moc_action.cpp"
