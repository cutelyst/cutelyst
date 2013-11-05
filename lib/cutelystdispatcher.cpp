#include "cutelystdispatcher.h"

#include "cutelystcontroller.h"
#include "cutelystaction.h"

#include <QMetaMethod>
#include <QStringBuilder>
#include <QDebug>

CutelystDispatcher::CutelystDispatcher(QObject *parent) :
    QObject(parent)
{
}

void CutelystDispatcher::setupActions()
{
    // Find all the User classes
    int metaType = QMetaType::User;
    while (QMetaType::isRegistered(metaType)) {
        const QMetaObject *meta = QMetaType::metaObjectForType(metaType);
        if (qstrcmp(meta->superClass()->className(), "CutelystController") == 0) {
            // App controller
            CutelystController *controller = qobject_cast<CutelystController*>(meta->newInstance());
            qDebug() << "Found a controller:" << controller << meta->className();

            int i = 0;
            while (i < meta->methodCount()) {
                QMetaMethod method = meta->method(i);
                if (method.methodType() == QMetaMethod::Slot || method.methodType() == QMetaMethod::Method) {
                    if (method.name() != "destroyed" &&
                            method.name() != "deleteLater" &&
                            method.name() != "_q_reregisterTimers") {
                        qDebug() << Q_FUNC_INFO << method.name() << method.attributes() << method.methodType() << method.methodSignature();
                        qDebug() << Q_FUNC_INFO << method.parameterTypes() << method.tag() << method.access();
                        CutelystAction *action = new CutelystAction(method, controller);
                        qDebug() << "====================";
                        qDebug() << "====================" << controller->classNamespace();
                        qDebug() << "====================" << action->name();

                        QString privateName = controller->classNamespace() % QLatin1Char('/') % action->name();
                        if (!m_actions.contains(privateName)) {
                            m_actions.insert(privateName, action);
                        } else {
                            delete action;
                        }
                        qDebug() << "====================";
                    }
                }
                ++i;
            }
        }

        if (meta->classInfoCount()) {
            QMetaClassInfo classInfo = meta->classInfo(0);
            qDebug() << Q_FUNC_INFO << classInfo.name() << classInfo.value();
        }
        ++metaType;
    }

    printActions();
}

void CutelystDispatcher::printActions()
{
    qDebug() << "Loaded private actions:";
    QString privateTitle("Private");
    QString classTitle("Class");
    QString methodTitle("Method");
    int privateLength = privateTitle.length();
    int classLength = classTitle.length();
    int actionLength = methodTitle.length();
    QHash<QString, CutelystAction*>::ConstIterator it = m_actions.constBegin();
    while (it != m_actions.constEnd()) {
        CutelystAction *action = it.value();
        privateLength = qMax(privateLength, it.key().length());
        classLength = qMax(classLength, action->className().length());
        actionLength = qMax(actionLength, action->name().length());
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";
    qDebug() << "|" << privateTitle.leftJustified(privateLength).toUtf8().data()
             << "|" << classTitle.leftJustified(classLength).toUtf8().data()
             << "|" << methodTitle.leftJustified(actionLength).toUtf8().data()
             << "|";
    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";

    it = m_actions.constBegin();
    while (it != m_actions.constEnd()) {
        CutelystAction *action = it.value();
        qDebug() << "|" << it.key().leftJustified(privateLength).toUtf8().data()
                 << "|" << action->className().leftJustified(classLength).toUtf8().data()
                 << "|" << action->name().leftJustified(actionLength).toUtf8().data()
                 << "|";
        ++it;
    }

    qDebug() << "." << QString().fill(QLatin1Char('-'), privateLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), classLength).toUtf8().data()
             << "+" << QString().fill(QLatin1Char('-'), actionLength).toUtf8().data()
             << ".";

}
