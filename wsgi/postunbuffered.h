#ifndef POSTUNBUFFERED_H
#define POSTUNBUFFERED_H

#include <QIODevice>

class PostUnbuffered : public QIODevice
{
    Q_OBJECT
public:
    explicit PostUnbuffered(QObject *parent = 0);

    qint64 m_contentLength = 0;
};

#endif // POSTUNBUFFERED_H
