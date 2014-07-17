#ifndef GRANTLEE_VIEW_H
#define GRANTLEE_VIEW_H

#include <QObject>

#include "../ViewInterface.h"

namespace Cutelyst {

class GrantleeViewPrivate;
class GrantleeView : public ViewInterface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GrantleeView)
    Q_PLUGIN_METADATA(IID "org.cutelyst.Grantlee" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ViewInterface)
public:
    Q_INVOKABLE explicit GrantleeView(QObject *parent = 0);
    ~GrantleeView();

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
    GrantleeViewPrivate *d_ptr;
};

}

#endif // GRANTLEE_VIEW_H
