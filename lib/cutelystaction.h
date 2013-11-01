#ifndef CUTELYSTACTION_H
#define CUTELYSTACTION_H

#include <QObject>
#include <QStringList>
#include <QMetaMethod>

class CutelystContext;
class CutelystController;
class CutelystAction : public QObject
{
    Q_OBJECT
public:
    explicit CutelystAction(const QMetaMethod &method, CutelystController *parent = 0);

    bool isValid() const;

    /**
     * @return The sub attributes that are set for this action,
     * like Local, Path, Private and so on. This determines
     * how the action is dispatched to.
     */
    QHash<QString, QString> attributes() const;

    /**
     * @return Returns the name of the component where this action is defined
     */
    QString className() const;

    /**
     * @return Returns the controller where this action is defined
     */
    CutelystController* controller() const;

    /**
     * @brief dispatch Dispatch this action against a context
     */
    bool dispatch(CutelystContext *c);

    /**
     * @brief Check Args attribute, and makes sure number of
     * args matches the setting. Always returns true if Args is omitted.
     */
    bool match(CutelystContext *c) const;

    /**
     * @brief Can be implemented by action class
     * and action role authors. If the method exists,
     * then it will be called with the request context
     * and an array reference of the captures for this action.
     *
     * @return Returning true from this method causes the chain
     * match to continue, returning makes the chain not match
     * (and alternate, less preferred chains will be attempted).
     */
    bool matchCaptures(CutelystContext *c) const;

    /**
     * @brief name
     * @return Returns the sub name of this action.
     */
    QString name() const;

    /**
     * @brief numberOfArgs
     * @return Returns the number of args this action expects.
     * This is 0 if the action doesn't take any arguments and
     * undef if it will take any number of arguments.
     */
    quint8 numberOfArgs() const;

    /**
     * @brief numberOfCaptures
     * @return Returns the number of captures this action
     * expects for Chained actions.
     */
    quint8 numberOfCaptures() const;

    /**
     * @brief meta
     * @return Returns the meta information about this action.
     */
    QMetaMethod meta() const;

private:
    bool invokeMethod(CutelystContext *c,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());

    QMetaMethod m_method;
    QHash<QString, QString> m_attributes;
    CutelystController *m_controller;
};

#endif // CUTELYSTACTION_H
