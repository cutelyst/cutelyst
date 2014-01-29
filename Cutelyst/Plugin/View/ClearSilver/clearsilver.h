#ifndef CLEARSILVER_H
#define CLEARSILVER_H

#include <QObject>

#include "../ViewInterface.h"

namespace Cutelyst {

class ClearSilverPrivate;
class ClearSilver : public QObject, ViewInterface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClearSilver)
    Q_PLUGIN_METADATA(IID "org.cutelyst.ClearSilver" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ViewInterface)
public:
    explicit ClearSilver(QObject *parent = 0);
    ~ClearSilver();

    Q_PROPERTY(QString includePath READ includePath WRITE setIncludePath)
    QString includePath() const;
    void setIncludePath(const QString &path);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension)
    QString templateExtension() const;
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper)
    QString wrapper() const;
    void setWrapper(const QString &name);

    bool render(Context *ctx);

protected:
    ClearSilverPrivate *d_ptr;
};

}

#endif // CLEARSILVER_H
