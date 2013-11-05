#ifndef CUTELYSTDISPATCHER_H
#define CUTELYSTDISPATCHER_H

#include <QObject>
#include <QHash>

class CutelystAction;
class CutelystDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit CutelystDispatcher(QObject *parent = 0);
    void setupActions();

private:
    void printActions();
    QHash<QString, CutelystAction*> m_actions;
};

#endif // CUTELYSTDISPATCHER_H
